// mono.cc
// 7/27/2015 jichi
// https://github.com/mono/mono/blob/master/mono/metadata/object.h
#include "engine/model/kirikiriz.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"

#include "hijack/hijackmanager.h"
//#include "util/textutil.h"
//#include "hijack/hijackmanager.h"
#include "cpputil/cppcstring.h"
#include "memdbg/memsearch.h"
#include"ithsys/ithsys.h"
#include "mono/monoobject.h"
//#include "mono/monotype.h"
#include "winhook/hookcode.h"
#include "unistr/unichar.h"
//#include "winhook/hookfun.h"
//#include "ntdll/ntdll.h"
#include <qt_windows.h>
#include <QtCore/QSet>
#include <cstdint>
#include <unordered_set>

#define DEBUG "model/mono"
#include "sakurakit/skdebug.h"

//#pragma intrinsic(_ReturnAddress)

namespace { // unnamed

    class TextHook
    {
        typedef TextHook Self;

        QByteArray data_; // persistent storage, which makes this function not thread-safe
        int role_;

        bool hookBefore(winhook::hook_stack* s)
        {
            auto text = (LPCSTR)s->stack[1]; // text in arg1
            if (!text || !*text)
                return true;
            auto split = s->stack[0]; // retaddr
            auto sig = Engine::hashThreadSignature(role_, split);
            data_ = EngineController::instance()->dispatchTextA(text, role_, sig);
            s->stack[1] = (ulong)data_.constData(); // reset arg1
            return true;
        }

    public:
        explicit TextHook(int role) : role_(role) {}

        bool attach(const uint8_t* pattern, size_t patternSize, ulong startAddress, ulong stopAddress)
        {
            ulong addr = MemDbg::findBytes(pattern, patternSize, startAddress, stopAddress);
            return addr
                && (addr = MemDbg::findEnclosingAlignedFunction(addr))
                && winhook::hook_before(addr,
                    std::bind(&Self::hookBefore, this, std::placeholders::_1));
        }
    };
}
namespace { // unnamed
    bool SearchResourceString(LPCWSTR str)
    {
        DWORD hModule = (DWORD)GetModuleHandleW(nullptr);
        IMAGE_DOS_HEADER *DosHdr;
        IMAGE_NT_HEADERS *NtHdr;
        DosHdr = (IMAGE_DOS_HEADER *)hModule;
        DWORD rsrc, size;
        //__asm int 3
        if (IMAGE_DOS_SIGNATURE == DosHdr->e_magic) {
            NtHdr = (IMAGE_NT_HEADERS *)(hModule + DosHdr->e_lfanew);
            if (IMAGE_NT_SIGNATURE == NtHdr->Signature) {
            rsrc = NtHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
            if (rsrc) {
                rsrc += hModule;
                if (IthGetMemoryRange((LPVOID)rsrc, &rsrc ,&size) &&
                    SearchPattern(rsrc, size - 4, str, wcslen(str) << 1))
                return true;
            }
            }
        }
        return false;
    }
    bool CheckFile(LPCWSTR name)
    {
        WIN32_FIND_DATAW unused;
        HANDLE file = FindFirstFileW(name, &unused);
        if (file != INVALID_HANDLE_VALUE)
        {
            FindClose(file);
            return true;
        }
        wchar_t path[MAX_PATH * 2];
        wchar_t* end = path + GetModuleFileNameW(nullptr, path, MAX_PATH);
        while (*(--end) != L'\\');
        wcscpy_s(end + 1, MAX_PATH, name);
        file = FindFirstFileW(path, &unused);
        if (file != INVALID_HANDLE_VALUE)
        {
            FindClose(file);
            return true;
        }
        return false;
    }
}

bool kirikirizEngine::match() {
    if ( ::CheckFile(L"*.xp3") ||  ::SearchResourceString(L"TVP(KIRIKIRI)")) {
        if (::SearchResourceString(L"TVP(KIRIKIRI) Z ")) { // TVP(KIRIKIRI) Z CORE
        // jichi 11/24/2014: Disabled that might crash VBH
        //if (Util::CheckFile(L"plugin\\KAGParser.dll"))
        //  InsertKAGParserHook();
        //else if (Util::CheckFile(L"plugin\\KAGParserEx.dll"))
        //  InsertKAGParserExHook();
        
            return true;
        } 
  }
    return false;
}
bool kirikirizEngine::attach()
{
    return false;
  //  ulong startAddress, stopAddress;
  //  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress)) {
  //      DOUT("cannot getProcessMemoryRange");
  //      return false;
  //  }

  //  const BYTE bytes[] = {
  //  0x38,0x4b, 0x21,     // 0122812f   384b 21          cmp byte ptr ds:[ebx+0x21],cl
  //  0x0f,0x95,0xc1,      // 01228132   0f95c1           setne cl
  //  0x33,0xc0,           // 01228135   33c0             xor eax,eax
  //  0x38,0x43, 0x20,     // 01228137   3843 20          cmp byte ptr ds:[ebx+0x20],al
  //  0x0f,0x95,0xc0,      // 0122813a   0f95c0           setne al
  //  0x33,0xc8,           // 0122813d   33c8             xor ecx,eax
  //  0x33,0x4b, 0x10,     // 0122813f   334b 10          xor ecx,dword ptr ds:[ebx+0x10]
  //  0x0f,0xb7,0x43, 0x14 // 01228142   0fb743 14        movzx eax,word ptr ds:[ebx+0x14]
  //}; 

  //ULONG addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  ////GROWL_DWORD(addr);
  //if (!addr) {
  //  DOUT("vnreng:KiriKiriZ2: pattern not found");
  //  return false;
  //}

  //static auto h = new TextHook(Engine::ScenarioRole); // never deleted
  //return h->attach(bytes, sizeof(bytes), startAddress, stopAddress) || h->attach(bytes, sizeof(bytes), startAddress, stopAddress);
  //HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);
  //return true;
}  