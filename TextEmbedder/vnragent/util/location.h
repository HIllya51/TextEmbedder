#pragma once

// location.h
// 5/3/2014 jichi

#include <QtCore/QString>

// Be careful that the functions here are not thread-safe on the startup
namespace Util {

// Location of ~/Desktop
QString desktopLocation();

// Directory of Qt5Core.dll// QtCore4.dll
QString qtLocation();

// Parent directory of Qt5Core.dll// QtCore4.dll
QString qtPrefix();
 

} // namespace Util

// EOF
