 
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
#include"engine/model/aos.h" 

#define DEBUG "model/aos"
#include "sakurakit/skdebug.h"   

namespace { // unnamed
    class ScenarioHook {

    public:
        explicit ScenarioHook(int role)  {}

        typedef ScenarioHook Self;
        bool nopbytes(const void* pattern, DWORD patternSize, DWORD lowerBound, DWORD upperBound ) {
            auto addr1 = (unsigned char* )lowerBound;
            while (1) {
                addr1 = (unsigned char*)MemDbg::findBytes(pattern, patternSize, (DWORD)addr1, upperBound);
                DOUT(addr1);
                if (addr1 > (unsigned char*)0xb90000)break;
                if (addr1 == 0)break;
                DWORD oldProtect; VirtualProtect(addr1, patternSize, PAGE_READWRITE, &oldProtect);
                addr1[0] = 0x3c;
                addr1[1] = 0xff;
            }
            return true;
            
        }
        bool attach()
        {
            ulong startAddress, stopAddress;
            //getMemoryRange(&startAddress, &stopAddress);
            Engine::getProcessMemoryRange(&startAddress, &stopAddress);

            const BYTE bytes[] = {
                //void sub_B60B40()
            //{
            //    char* v0; // esi
            //    int v1; // ebx
            //    int v2; // esi
            //    int v3; // edi
            //  v8 = dword_BDED80; 
                0x83,0xC4,0x04,                     // add     esp, 4
                0x68,0x00,0x00,0x01,0x00,               // push    10000h; Size
                0x8B,0xF0,                        // mov     esi, eax
                0x6A,0x00,                      //   push    0; Val
                0x56
                //int __usercall sub_B62D10@<eax>(__int16 *a1@<eax>, unsigned __int8 *a2@<edi>)
                //sub_B62D10((__int16 *)v1, (unsigned __int8 *)v8);
            };
            ULONG range = min((ulong)(stopAddress - startAddress), (ulong)0x00300000);
            ULONG addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, startAddress + range); 
            if (!addr) return false; 
            addr = MemDbg::findEnclosingAlignedFunction(addr);
            if (!addr)return false;
            
           /* const BYTE bytes2[] = {
                    0x3c,0x9f 
            }; 
            nopbytes(bytes2, sizeof(bytes2), startAddress, startAddress + range ); 
             */
            const BYTE bytesaddress[] = {
                0x0f,0xbe,0x07,
                0x83,0xf8,0x5c
            };
            auto charaddr=MemDbg::findBytes(bytesaddress, sizeof(bytesaddress), addr, addr + 0x1000);
            if (charaddr == 0)return false;
             
            return winhook::hook_before(addr, std::bind(&Self::hookBefore, this, std::placeholders::_1, *(int*)(charaddr - 4)));
        }
          
        bool hookBefore(winhook::hook_stack* s, ULONG charaddr)
        {
            //auto text = (char*)s->stack[2]; // text in arg1
            //
            auto text = (char*)(*(int*)charaddr);

            if (!text || !*text)
                return true;
            auto split = s->stack[0]; // retaddr
             
            auto usetext = text;
            if (text[0] == '[') {
                while (*usetext != ']')usetext++;
                usetext++;
            }
            char* match = usetext;
            for ( ;*match; match++) {
                if (*match == '\\') {
                    break;
                }
            }
             
            auto sig = Engine::hashThreadSignature(Engine::ScenarioRole, split);
            auto data_ = EngineController::instance()->dispatchTextA(QByteArray::fromRawData(usetext, match-usetext), Engine::ScenarioRole, sig, 0, true, nullptr, false ,false,true) ;
            //由于编码问题，无法正常显示。故只做提取了。
            // 
            //QByteArray prefix = QByteArray::fromRawData(text, usetext - text);
            //prefix.append(data_);
            //prefix.append(match);
            //DOUT(data_.constData());
            //DOUT(prefix.constData()); 
            ////strcpy(text, prefix.constData()); 
            //(*(int*)charaddr) = (ulong)prefix.constData();
            return true;
        }
         

    }; // namespace ScenarioHook
} // unnamed namespace
 
bool aosEngine::attach()
{ 
    static auto h = new ScenarioHook(Engine::ScenarioRole); // never deleted
    if (!h->attach())
        return false;

    //HijackManager::instance()->attachFunction((ulong)::GetOutlineTextMetricsA); 
    //HijackManager::instance()->attachFunction((ulong)::CreateFontA); 
    //HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);

    return true;
}
