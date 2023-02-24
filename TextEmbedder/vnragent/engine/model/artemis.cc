 
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
#include"engine/model/artemis.h" 

#define DEBUG "model/artemis"
#include "sakurakit/skdebug.h"   

namespace { // unnamed
    class ScenarioHook {

    public:
        explicit ScenarioHook(int role)  {}

        typedef ScenarioHook Self;
        bool attach()
        {
            ulong startAddress, stopAddress;
            //getMemoryRange(&startAddress, &stopAddress);
            Engine::getProcessMemoryRange(&startAddress, &stopAddress);

            const BYTE bytes[] = {
                0x55,                   // 005FD780 | 55                       | push ebp                                |
                0x8B, 0xEC,             // 005FD781 | 8BEC                     | mov ebp,esp                             |
                0x83, 0xE4, 0xF8,       // 005FD783 | 83E4 F8                  | and esp,FFFFFFF8                        |
                0x83, 0xEC, 0x3C,       // 005FD786 | 83EC 3C                  | sub esp,3C                              |
                0xA1, XX4,              // 005FD789 | A1 6C908600              | mov eax,dword ptr ds:[86906C]           |
                0x33, 0xC4,             // 005FD78E | 33C4                     | xor eax,esp                             |
                0x89, 0x44, 0x24, 0x38, // 005FD790 | 894424 38                | mov dword ptr ss:[esp+38],eax           |
                0x53,                   // 005FD794 | 53                       | push ebx                                |
                0x56,                   // 005FD795 | 56                       | push esi                                |
                0x8B, 0xC1,             // 005FD796 | 8BC1                     | mov eax,ecx                             |
                0xC7, 0x44, 0x24, 0x14, 0x00, 0x00, 0x00, 0x00, // 005FD798 | C74424 14 00000000       | mov dword ptr ss:[esp+14],0             |
                0x8B, 0x4D, 0x0C,       // 005FD7A0 | 8B4D 0C                  | mov ecx,dword ptr ss:[ebp+C]            |
                0x33, 0xF6,             // 005FD7A3 | 33F6                     | xor esi,esi                             |
                0x57,                   // 005FD7A5 | 57                       | push edi                                |
                0x8B, 0x7D, 0x08,       // 005FD7A6 | 8B7D 08                  | mov edi,dword ptr ss:[ebp+8]            |
                0x89, 0x44, 0x24, 0x14, // 005FD7A9 | 894424 14                | mov dword ptr ss:[esp+14],eax           |
                0x89, 0x4C, 0x24, 0x28, // 005FD7AD | 894C24 28                | mov dword ptr ss:[esp+28],ecx           |
                0x80, 0x3F, 0x00,       // 005FD7B1 | 803F 00                  | cmp byte ptr ds:[edi],0                 |
                0x0F, 0x84, XX4,        // 005FD7B4 | 0F84 88040000            | je ヘンタイ・プリズンsplit 1.5FDC42              |
                0x83, 0xB8, XX4, 0x00,  // 005FD7BA | 83B8 74030000 00         | cmp dword ptr ds:[eax+374],0            |
                0x8B, 0xDF,             // 005FD7C1 | 8BDF                     | mov ebx,edi                             |
            };

            enum { addr_offset = 0 }; // distance to the beginning of the function, which is 0x55 (push ebp)
            ULONG range = min((ulong) (stopAddress - startAddress), (ulong)0x00300000 );
            ULONG addr = MemDbg::matchBytes(bytes, sizeof(bytes), startAddress, startAddress + range);
             
            if (!addr) { 
                return false;
            }
            addr += addr_offset;
            enum { push_ebp = 0x55 }; // beginning of the function
            
            if (*(BYTE*)addr != push_ebp) { 
                return false;
            }
            DOUT(addr);
            return   winhook::hook_before(addr,
                    std::bind(&Self::hookBefore, this, std::placeholders::_1));

        }
         
        bool hookBefore(winhook::hook_stack* s)
        {  
            
              auto text = (char*)s->stack[1]; // text in arg1
               
              if (!text || !*text)
                  return true;
              auto split = s->stack[0]; // retaddr
              auto sig = Engine::hashThreadSignature(Engine::ScenarioRole, split); 
              auto data_ = EngineController::instance()->dispatchTextA(text, Engine::ScenarioRole, sig,0,true,nullptr,true).constData();
                
              char* dd = (char*)malloc(strlen(data_) + 10);
              strcpy(dd, data_);
              dd[strlen(data_)] = 0;
              s->stack[1] = (ulong)dd;
            return true;
        }
         

    }; // namespace ScenarioHook
} // unnamed namespace
 
bool artemisEngine::attach()
{ 
    static auto h = new ScenarioHook(Engine::ScenarioRole); // never deleted
    if (!h->attach())
        return false;
     
    //HijackManager::instance()->attachFunction((ulong)::GetOutlineTextMetricsA); 
    //HijackManager::instance()->attachFunction((ulong)::CreateFontA); 
    //HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);

    return true;
}
