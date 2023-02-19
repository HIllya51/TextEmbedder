

// krkrz.cc
// 6/15/2015 jichi
// About MES and Silky's arc
// See: http://www.dsl.gr.jp/~sage/mamitool/case/elf/aishimai.html
// MES: http://tieba.baidu.com/p/2068362185
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
#include"engine/model/kirikiriz.h"
#define DEBUG "krkrz"
#include "sakurakit/skdebug.h"  
//TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/drawText)
//void tTJSNI_BaseLayer::DrawText(tjs_int x, tjs_int y, const ttstr& text,
//    tjs_uint32 color, tjs_int opa, bool aa, tjs_int shadowlevel,
//    tjs_uint32 shadowcolor, tjs_int shadowwidth, tjs_int shadowofsx,
//    tjs_int shadowofsy)
/*
int __thiscall sub_A7BE60(
        int this,
        int a2,
        int a3,
        int *a4,
        int a5,
        int a6,
        char a7,
        int a8,
        int a9,
        int a10,
        int a11,
        int a12)
{
  int v13; // eax
  int v14; // eax
  int v15; // edi
  int v16; // edx
  DWORD SysColor; // eax
  int v18; // eax
  int v19; // eax
  int result; // eax
  int v21; // ecx
  int v22; // esi
  __int64 *v23; // [esp-3Ch] [ebp-80h]
  int v24; // [esp+10h] [ebp-34h]
  int v25[2]; // [esp+14h] [ebp-30h] BYREF
  int v26; // [esp+1Ch] [ebp-28h]
  int v27; // [esp+20h] [ebp-24h]
  int v28; // [esp+24h] [ebp-20h]
  int v29; // [esp+28h] [ebp-1Ch]
  int v30; // [esp+2Ch] [ebp-18h]
  char v31; // [esp+30h] [ebp-14h]
  int v32; // [esp+40h] [ebp-4h]

  if ( !*(_DWORD *)(this + 204) )
    sub_9BEB40();
  v13 = *(_DWORD *)(this + 328);
  if ( v13 )
  {
    v14 = v13 - 1;
    if ( v14 )
    {
      if ( v14 != 3 )
      {
        sub_971440(&off_CBE890);
        v32 = 0;
        sub_9BECB0();
      }
      if ( a6 < 0 )
        sub_9BEB40();
      v15 = 14;
    }
    else
    {
      v15 = 2;
    }
  }
  else
  {
    v15 = 3;
  }
  if ( *(_BYTE *)(this + 372) )
  {
    *(_BYTE *)(this + 372) = 0;
    sub_A654F0(this + 356);
  }
  v25[0] = 0;
  v25[1] = 0;
  v26 = 0;
  v30 = 0;
  v29 = 0;
  v28 = 0;
  v27 = 0;
  v31 = 0;
  v32 = 1;
  v16 = a5;
  SysColor = a5;
  if ( (a5 & 0xFF000000) != 0 )
  {
    if ( a5 < 0 )
      SysColor = GetSysColor((unsigned __int8)a5);
    v16 = (SysColor & 0xFF00) + ((unsigned __int8)SysColor << 16) + BYTE2(SysColor);
  }

  LOBYTE(v24) = *(_BYTE *)(this + 336);
  if ( *a4 )
  {
    v18 = *(_DWORD *)(*a4 + 52);
    if ( v18 )
    {
      v23 = (__int64 *)(this + 340);
      if ( v18 < 2 )
        sub_A66460(*(_DWORD **)(this + 204), v23, a2, a3, a4, v16, v15, a6, v24, a7, a8, a9, a10, a11, a12, (int)v25);
      else
        sub_A66840(v23, a2, a3, a4, v16, v15, a6, v24, a7, a8, a9, a10, a11, a12, v25);
    }
  }
  if ( v26 )
    *(_BYTE *)(this + 337) = 1;
  v19 = *(_DWORD *)(this + 224);
  if ( v19 || *(_DWORD *)(this + 228) )
    sub_A56F90(v19, *(_DWORD *)(this + 228));
  result = sub_A7E2F0(v25, 0);
  v32 = -1;
  if ( v26 )
  {
    v21 = v25[0];
    do
    {
      v22 = *(_DWORD *)(v21 + 20);
      result = sub_A561E0();
      v21 = v22;
    }
    while ( v22 != v25[0] );
  }
  return result;
}
*/




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
               
                //0x38,0x4b, 0x21,     0x0f,0x95,0xc1,      0x33,0xc0,           0x38,0x43, 0x20,     0x0f,0x95,0xc0,      0x33,0xc8,           0x33,0x4b, 0x10,     0x0f,0xb7,0x43, 0x14  //textractor,每个字都会跑n遍这个函数
             /*  0x8b,0x00,
               0x85,0xc0,
               0x74,0x0c,
               0x8b,0x78,0x04,
               0x85,0xff */  //显示文字
              /*  0x8b,0x00,
                0x85,0xc0,
                0x74,0x0f,
                0x8b,0x78,0x04,
                0x89,0x7d,0x20*/ //显示名字

               /* 0x8a,0x86,0x50,0x01,0x00,0x00,
                0x8b,0x4d,0x10,
                0x88,0x44,0x24,0x10,
                0x8b,0x01,
                0x85,0xc0,
                0x74,0x4a*/
                0x8b,0x86,0x48,0x01,0x00,0x00,
                0x8b,0x5d,0x18,
                0x83,0xe8,0x00,
                0x74,0x69,
                0x48   //sub_B64260
                /*B6D2E0
                ……
                b6d486   call sub_B64260*/
              
            };
            ULONG range = min(stopAddress - startAddress, 0x00300000);
            ULONG addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress + range);
             
            return false && addr
                && (addr = MemDbg::findEnclosingAlignedFunction(addr))
                && winhook::hook_before(addr,
                std::bind(&Self::hookBefore, this, std::placeholders::_1));

        } 
         
        bool getMemoryRange(ulong* startAddress, ulong* stopAddress)
        { 
            *startAddress = *stopAddress = (uintptr_t)GetModuleHandleW(nullptr);
            
            MEMORY_BASIC_INFORMATION info;
            do
            {
                VirtualQuery((void*)stopAddress, &info, sizeof(info));
                *stopAddress = (uintptr_t)info.BaseAddress + info.RegionSize;
            } while (info.Protect > PAGE_NOACCESS);
            *stopAddress -= info.RegionSize;
            return true;
        }
        std::unordered_set<LPCSTR> translatedTexts_;
        bool hookBefore(winhook::hook_stack* s)
        { 
           /* char x[10];
            sprintf(x, "arg3 %s", (LPCSTR)(s->stack[1]));
            DOUT(x);*/
           // MessageBoxA(0, "", "", MB_OK);
            //auto text = (LPCSTR)s->stack[1]; // text in arg1
            //if (!text || !*text)
            //    return true;
            //auto split = s->stack[0]; // retaddr
            //auto sig = Engine::hashThreadSignature(role_, split);
            //data_ = EngineController::instance()->dispatchTextA(text, role_, sig);
            //s->stack[1] = (ulong)data_.constData(); // reset arg1
            return true;
        }
         
        QByteArray data_; // persistent storage, which makes this function not thread-safe
        int role_;

    }; // namespace ScenarioHook
} // unnamed namespace
bool KiriKiriZEngine::match() {
    if ( ::CheckFile(L"*.xp3") ||  ::SearchResourceString(L"TVP(KIRIKIRI)")) {
        if (::SearchResourceString(L"TVP(KIRIKIRI) Z ")) {  
            DOUT("kirikiriz matched");
            return true;
        } 
    }
    return false;
}
bool KiriKiriZEngine::attach()
{
    static auto h = new ScenarioHook(Engine::ScenarioRole); // never deleted
    if (!h->attach())
        return false;
    HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineW);
    return true;
}
 