// rpcclient.cc
// 2/1/2013 jichi
#include "config.h"
#include "driver/rpcclient.h"
#include "driver/rpcclientprivate.h"
#include "qtsocketsvc/socketpack.h"
#include "qtsocketsvc/socketpipe.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QTimer>

#ifdef VNRAGENT_ENABLE_TCP_SOCKET
# include "qtsocketsvc/tcpsocketclient.h"
#else
# include "qtsocketsvc/localsocketclient.h"
#endif // VNRAGENT_ENABLE_TCP_SOCKET

#define DEBUG "rpcclient"
#include "sakurakit/skdebug.h"

/** Private class */

RpcClientPrivate::RpcClientPrivate(Q *q)
  : Base(q), q_(q), appQuit(false), client(new RpcSocketClient(this)), clientPipe(nullptr)
{
  clientOverlapped = SocketService::newPipeOverlapped();

#ifdef VNR_ENABLE_TCP_SOCKET
  client->setPort(VNRAGENT_SOCKET_PORT);
  client->setAddress(VNRAGENT_SOCKET_HOST);
#else
  client->setServerName(VNRAGENT_SOCKET_PIPE);
#endif // VNRAGENT_ENABLE_TCP_SOCKET

  connect(client, SIGNAL(dataReceived(QByteArray)), SLOT(onDataReceived(QByteArray)));

  connect(client, SIGNAL(disconnected()), q, SIGNAL(disconnected()));
  connect(client, SIGNAL(disconnected()), q, SIGNAL(aborted()));
  connect(client, SIGNAL(socketError()), q, SIGNAL(aborted()));

#ifdef VNRAGENT_ENABLE_RECONNECT
  reconnectTimer = new QTimer(q);
  reconnectTimer->setSingleShot(true); // until reconnect successfully
  reconnectTimer->setInterval(ReconnectInterval);
  connect(reconnectTimer, SIGNAL(timeout()), SLOT(reconnect()));

  //connect(client, SIGNAL(socketError()), SLOT(reconnect()), Qt::QueuedConnection);
  connect(client, SIGNAL(disconnected()), reconnectTimer, SLOT(start()));
  connect(client, SIGNAL(socketError()), reconnectTimer, SLOT(start()));
#endif // VNRAGENT_ENABLE_RECONNECT
}

RpcClientPrivate::~RpcClientPrivate()
{ SocketService::deletePipeOverlapped(clientOverlapped); }

void RpcClientPrivate::start()
{
  //if (client->isConnected())
  //  return true;
  //client->stop();
  client->start();
  if (client->waitForConnected())
    onConnected();
}

void RpcClientPrivate::reconnect()
{
  //clientPipe = nullptr;
#ifdef VNRAGENT_ENABLE_RECONNECT
  if (reconnectTimer->isActive())
    reconnectTimer->stop();
#endif // VNRAGENT_ENABLE_RECONNECT
  if (client->isConnected())
    return;
  //client->stop();
  client->start();
  if (client->waitForConnected()) {
    client->restart();
    if (client->waitForConnected()) {
      onConnected();
      q_->emit reconnected();
    }
  }
}

void RpcClientPrivate::onConnected()
{
  DOUT("connected");
  clientPipe = SocketService::findLocalSocketPipeHandle(client);
  pingServer();
}

void RpcClientPrivate::pingServer()
{
  auto pid = QCoreApplication::applicationPid();
  callServer("agent.ping", marshalInteger(pid));
}

void RpcClientPrivate::callServer(const QStringList &args)
{
  if (client->isConnected()) {
    QByteArray data = SocketService::packStringList(args);
    sendData(data);
  }
}

void RpcClientPrivate::directCallServer(const QStringList &args)
{
  if (client->isConnected()) {
    QByteArray data = SocketService::packStringList(args);
    directSendData(data);
  }
}

void RpcClientPrivate::sendData(const QByteArray &data)
{ client->sendData(data, WaitInterval); }

void RpcClientPrivate::directSendData(const QByteArray &data)
{
  if (clientPipe) {
    QByteArray packet = SocketService::packPacket(data);
    SocketService::writePipe(clientPipe, packet.constData(), packet.size(), clientOverlapped, &appQuit);
  }
}

void RpcClientPrivate::onDataReceived(const QByteArray &data)
{
  QStringList l = SocketService::unpackStringList(data);
  if (!l.isEmpty())
    onCall(l);
}

void RpcClientPrivate::onCall(const QStringList &args)
{
  if (args.isEmpty()) return;
  //DOUT("cmd:" << args.first());
  auto arg = args.first();
  if (arg == "ping") {}
  else if (arg == "detach") q_->emit detachRequested();
  else if (arg == "clear") q_->emit clearTranslationRequested();
  //else if (arg == "enable") q_->emit enableRequested(true);
  else if (arg == "disable") q_->emit disableRequested();
  else if (arg == "settings")
  {
    if (args.size() == 2) q_->emit settingsReceived(args.last());
  }
  //else if (arg == "window.clear") q_->emit clearWindowTranslationRequested();
  //else if (arg == "window.enable") q_->emit enableWindowTranslationRequested(true);
  //else if (arg == "window.disable") q_->emit enableWindowTranslationRequested(false);
  else if (arg == "windows.text")
  {
    if (args.size() == 2) q_->emit windowTranslationReceived(args.last());
  }
  //else if (arg == "engine.clear") q_->emit clearEngineRequested();
  //else if (arg == "engine.enable") q_->emit enableEngineRequested(true);
  //else if (arg == "engine.disable") q_->emit enableEngineRequested(false);
  else if (arg == "engine.text")
  {
    if (args.size() == 5) {
      QString text = args[1];
      qint64 hash = unmarshalLongLong(args[2]);
      int role = unmarshalInt(args[3]);
      QString lang = args[4];
      q_->emit engineTranslationReceived(text, hash, role, lang);
    }
  }
  else {} //growl::debug(QString("Unknown command: %s").arg(cmd)); 
}

/** Public class */

// - Construction -

static RpcClient *instance_;
RpcClient *RpcClient::instance() { return ::instance_; }

RpcClient::RpcClient(QObject *parent)
  : Base(parent), d_(new D(this))
{
  d_->start();
  ::instance_ = this;
}

RpcClient::~RpcClient() { ::instance_ = nullptr; }

bool RpcClient::isActive() const { return d_->client->isConnected(); }

void RpcClient::quit() { d_->appQuit = true; }

// - Requests -

void RpcClient::requestWindowTranslation(const QString &json) { d_->sendWindowTexts(json); }

void RpcClient::sendEngineName(const QString &name)
{ d_->sendEngineName(name); }

void RpcClient::sendEngineText(const QString &text, qint64 hash, long signature, int role, bool needsTranslation)
{ d_->sendEngineText(text, hash, signature, role, needsTranslation); }

void RpcClient::directSendEngineText(const QString &text, qint64 hash, long signature, int role, bool needsTranslation)
{ d_->directSendEngineText(text, hash, signature, role, needsTranslation); }

//void RpcClient::sendEngineTextLater(const QString &text, qint64 hash, int role, bool needsTranslation)
//{ d_->sendEngineTextLater(text, hash, role, needsTranslation); }

void RpcClient::growlMessage(const QString &t) { d_->growlServer(t, D::GrowlMessage); }
void RpcClient::growlWarning(const QString &t) { d_->growlServer(t, D::GrowlWarning); }
void RpcClient::growlError(const QString &t) { d_->growlServer(t, D::GrowlError); }
void RpcClient::growlNotification(const QString &t) { d_->growlServer(t, D::GrowlNotification); }

bool RpcClient::waitForDataReceived(int interval)
{ return d_->client->isConnected() && d_->client->waitForDataReceived(interval); }

// EOF
