 
#include "engine/model/elf.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "winhook/hookcode.h"
#include"winhook/hookfun.h"
#include "memdbg/memsearch.h"
#include"util/dyncodec.h"
#include"ithsys/ithsys.h"
#include "winasm/winasmdef.h"
#include"hijack/hijackfuns.h"
#include<unordered_set>
#include <qt_windows.h>
#include"engine/model/willplus3.h" 
#include "sakurakit/skdebug.h"   
 /*
wchar_t* __userpurge sub_4392B0@<eax>(
    int a1@<ecx>,
    int a2@<ebp>,
    int a3@<edi>,
    HDC a4,
    _DWORD* a5,
    int a6,
    wchar_t* a7,    //***********
    int a8,
    float a9,
    int a10)
int __userpurge sub_43A150@<eax>(  
        int a1@<ecx>,
        int a2@<ebp>,
        int a3,
        HDC a4,
        _DWORD *a5,
        _DWORD *a6, 
        int a7,
        wchar_t *a8,  //*******
        void *a9,
        int a10,
        float a11,
        _OWORD *a12,
        int a13,
        int a14,
        _OWORD *a15,
        int a16,
        int a17,
        int a18,
        int a19,
        int a20,
        void *a21,
        int a22,
        int a23)
    …………
    sub_4392B0(a1, (int)v32, a1, a4, a6, a7, a8, a10, a11, a18);
    */
namespace { // unnamed
    class ScenarioHook {

    public:
        explicit ScenarioHook(int role) : role_(role) {}

        typedef ScenarioHook Self;
        bool attach()
        {
            ulong startAddress, stopAddress;
            //getMemoryRange(&startAddress, &stopAddress);
            Engine::getProcessMemoryRange(&startAddress, &stopAddress);

            const BYTE bytes[] = {
                   0xc7,0x45,0xfc,0x00,0x00,0x00,0x00,
                   0x33,0xc9,
                   0xc7,0x47,0x78,0x00,0x00,0x00,0x00
            };
            ULONG range = min(stopAddress - startAddress, 0x00300000);
            ULONG addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress + range);
            if (addr == 0)return false;
            char xx[100];
            sprintf(xx, "findBytes %d", addr);
            DOUT(xx);
            addr = MemDbg::findEnclosingAlignedFunction(addr);
            if (addr == 0)return false;
             
            sprintf(xx, "findEnclosingAlignedFunction %d",  (addr));
            DOUT(xx);
            if(addr== 0x0043A120)
                addr = 0x0043A150; //findEnclosingAlignedFunction 不知道为什么偏到更上面的0x0043A120上去了。不过发现ensemble几作的地址都一样，干脆硬编码了。
            return   winhook::hook_before(addr,
                    std::bind(&Self::hookBefore, this, std::placeholders::_1));

        }

      
        std::unordered_set<LPCSTR> translatedTexts_;
        bool hookBefore(winhook::hook_stack* s)
        {  
           // DOUT(QString::fromUtf16((LPWSTR)s->stack[6]));//"MS UI Gothic"
            //DOUT(QString::fromUtf16((LPWSTR)s->stack[7]));//"『美しい人になりたい』%K%P"
              auto text = (LPWSTR)s->stack[7]; // text in arg1
              
              if (!text || !*text)
                  return true;
              auto split = s->stack[0]; // retaddr
              auto sig = Engine::hashThreadSignature(role_, split);
               
              
              auto str = QString::fromUtf16((LPWSTR)s->stack[7] );
              int kp = 0;
              if (str.endsWith("%K%P")) {
                  kp = 1;
                  str.chop(4);
              }
               

              auto reg = QRegExp("\\{(.*):(.*)\\}");

              reg.setMinimal(true);
              QString appendix = "";
              
              int index=0;
              QStringList qs;// DOUT(str);
              while (true) {
                  index=reg.indexIn(str,index)+1;
                 // DOUT(reg.capturedTexts().join(" "));
                  auto notation = reg.cap(0); 
                  if (notation.compare("") == 0)break;
                  qs.append(notation);  
              }
              if (qs.length() > 0) {
                  appendix = "("+qs.join("  ")+")";
              } 
              
              str = str.replace(reg, "\\1");

              str = str.replace("\\n", "");
              QString data_ = EngineController::instance()->dispatchTextW(str, role_, sig);
              if (kp) {
                  data_.append("%K%P");
              }
              data_.append( appendix );
              s->stack[7] = (ulong)(data_.utf16());
            return true;
        }

        
        int role_;

    }; // namespace ScenarioHook
} // unnamed namespace
 
bool willplus3Engine::attach()
{ 
    static auto h = new ScenarioHook(Engine::ScenarioRole); // never deleted
    if (!h->attach())
        return false;

    HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineW);
    HijackManager::instance()->attachFunction((ulong)::TextOutW);  
    return true;
}
