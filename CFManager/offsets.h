#pragma once
#include "../ESP/ESP.h"
#include "../Config/globals.h"
#include <cstdint>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <d3d9.h>
#include <DirectXMath.h>
#include <fstream>
#include <sstream>
#include <iomanip>  
#include <cstdio>
#include <array>
#include <future>
#include <unordered_map>
#include "Classes.h"
#include "../Config/config.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <shlobj.h>      

 static bool showOffsetFinder = false;
const std::string CONFIG_FILE = "offsets.json";

using namespace DirectX;
namespace fs = std::filesystem;

inline std::string update_status = "Idle";
const std::string encryptionKey = "0123456789abcdef0123456789abcdef";
const std::string ivStr = "abcdef9876543210";

inline uintptr_t CFBASE = 0;
inline uintptr_t CFSHELL = 0;
inline uintptr_t LT_SHELL = 0;
constexpr int MAX_PLAYERS = 16;
inline D3DVIEWPORT9 g_view_port;

using json = nlohmann::json;
inline std::thread offsetThread;


 

 
namespace offs {

    //============================================================
    // Signature Patterns (Fixed)
    //============================================================
    /*
        LT_PATTERN:
        ---------------------------------------------------------------------
        00007FF95FDE3865                 lea     r8, aIclientshellDe ; "IClientShell.Default"
        ________:00007FF95FDE386C         lea     rdx, off_7FF962F25D00
        ---------------------------------------------------------------------
    */
    constexpr auto LT_PATTERN =
        "48 8D 15 ? ? ? ? FF 50 ? 48 8D 0D ? ? ? ? 48 83 C4 ? E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC 48 8D 0D";

    /*
        DRAWPRIM_PATTERN:
        ---------------------------------------------------------------------
        00000001400195A9                 lea     r8, aIltdrawprimDef ; "ILTDrawPrim.Default"
        ________:00000001400195B0         lea     rdx, off_140FB6110   + 0x2c70
        ---------------------------------------------------------------------
    */
    constexpr auto DRAWPRIM_PATTERN =
        "48 8D 15 ? ? ? ? FF 50 20 48 8D 0D ? ? ? ? 48 83 C4 30 5B E9 ? ? ? ? CC CC CC CC CC 48 8D 0D ? ? ? ?";

    /*
        MY_OFFSET_PATTERN:
        ---------------------------------------------------------------------
        00007FF961845E35                 movzx   eax, byte ptr [rbp+292h]
        ---------------------------------------------------------------------
    */
    constexpr auto MY_OFFSET_PATTERN =
        "0F B6 85 ? ? ? ? 48 69 C8 ? ? ? ? 0F B6 84 29 ? ? ? ? 4C 8B 47";

    /*
        MY_PLAYERSIZE_PATTERN:
        ---------------------------------------------------------------------
        00007FF961845E3C                 imul    rcx, rax, 0DC8h
        ---------------------------------------------------------------------
    */
    constexpr auto MY_PLAYERSIZE_PATTERN =
        "48 69 C8 ? ? ? ? 0F B6 84 29 ? ? ? ? 4C 8B 47";

    /*
        Additional Player Info:
        ---------------------------------------------------------------------
        Player start is the model instance right before this address.
        00007FF961845E43                 movzx   eax, byte ptr [rcx+rbp+2A0h]
        ---------------------------------------------------------------------
    */

    //============================================================
    // Memory Offsets
    //============================================================
    inline uintptr_t MYOFFSET = 0;
    inline uintptr_t dwCPlayerStart = 0x298;
    inline uintptr_t dwCPlayerSize = 0;
    inline uintptr_t ILTDrawPrim = 0;


    
}





/*
     Config File Setup Start
  */
 
inline bool LoadOffsets() {
    if (g_encryptedOffsets.empty())
        return false;
    try {
        std::string decrypted = aesDecrypt(g_encryptedOffsets, encryptionKey, ivStr);
        json j = json::parse(decrypted);
        if (!j.contains("LT_SHELL") || !j.contains("MYOFFSET") ||
            !j.contains("dwCPlayerSize") || !j.contains("ILTDrawPrim") ||
            !j.contains("dwCPlayerStart"))
        {
            return false;
        }
        LT_SHELL = std::stoull(j["LT_SHELL"].get<std::string>(), nullptr, 16);
        offs::MYOFFSET = static_cast<int32_t>(std::stoull(j["MYOFFSET"].get<std::string>(), nullptr, 16));
        offs::dwCPlayerSize = static_cast<int32_t>(std::stoull(j["dwCPlayerSize"].get<std::string>(), nullptr, 16));
        offs::ILTDrawPrim = std::stoull(j["ILTDrawPrim"].get<std::string>(), nullptr, 16);
        offs::dwCPlayerStart = std::stoull(j["dwCPlayerStart"].get<std::string>(), nullptr, 16);
    }
    catch (...) {
        return false;
    }
    return true;
}



inline void SaveOffsets() {
    json j;
    j["LT_SHELL"] = toHex(LT_SHELL);
    j["MYOFFSET"] = toHex(static_cast<uintptr_t>(offs::MYOFFSET));
    j["dwCPlayerSize"] = toHex(static_cast<uintptr_t>(offs::dwCPlayerSize));
    j["ILTDrawPrim"] = toHex(offs::ILTDrawPrim);
    j["dwCPlayerStart"] = toHex(offs::dwCPlayerStart);
    std::stringstream ss;
    ss << std::setw(4) << j;
    std::string plainOffsets = ss.str();
    try {
        g_encryptedOffsets = aesEncrypt(plainOffsets, encryptionKey, ivStr);
    }
    catch (const std::exception& e) {
         
        g_encryptedOffsets.clear();
    }
}


inline void ClearConfig() {
    g_encryptedOffsets.clear();
}




inline void UpdateOffsets(Memory& mem) {
   
    static std::atomic<bool> isUpdating = false;

    if (isUpdating.load()) {
        LOG("[Offsets] Already updating...");
        return;
    }

    isUpdating.store(true);

    offsetThread = std::thread([&mem]() {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
        update_status = "Updating Offsets...";

        bool success = false;

        do {
            if (LoadOffsets()) {
                success = true;
                break;
            }

            size_t CSHELL_SIZE = mem.GetBaseSize("CShell_x64.dll");
            size_t CFBASE_SIZE = mem.GetBaseSize("crossfire.exe");

            if (!CFSHELL || CSHELL_SIZE == 0 || !CFBASE || CFBASE_SIZE == 0) {
                update_status = "Update Failed";
                success = false;
                break;
            }

            
            uintptr_t firstSigResult = mem.FindSignature(offs::LT_PATTERN, CFSHELL, CFSHELL + CSHELL_SIZE);
            uintptr_t secondSigResult = mem.FindSignature(offs::MY_OFFSET_PATTERN, CFSHELL, CFSHELL + CSHELL_SIZE);
            uintptr_t thirdSigResult = mem.FindSignature(offs::MY_PLAYERSIZE_PATTERN, CFSHELL, CFSHELL + CSHELL_SIZE);
            uintptr_t fourthSigResult = mem.FindSignature(offs::DRAWPRIM_PATTERN, CFBASE, CFBASE + CFBASE_SIZE);

            if (!firstSigResult || !secondSigResult || !thirdSigResult || !fourthSigResult) {
                update_status = "Update Failed";
                success = false;
                break;
            }

            
            int32_t offsetLT = mem.Read<int32_t>(firstSigResult + 3);
            int32_t offsetMYOFFSET = mem.Read<int32_t>(secondSigResult + 3);
            int32_t offsetPlayerSize = mem.Read<int32_t>(thirdSigResult + 3);
            int32_t offsetDrawPrim = mem.Read<int32_t>(fourthSigResult + 3);

           
            LT_SHELL = firstSigResult + 7 + offsetLT;
            offs::MYOFFSET = offsetMYOFFSET;
            offs::dwCPlayerSize = offsetPlayerSize;
            offs::ILTDrawPrim = fourthSigResult + 7 + offsetDrawPrim;

            success = (LT_SHELL <= CFSHELL + CSHELL_SIZE);

            if (success)
                SaveOffsets();

        } while (false);

        update_status = success ? "Offsets Updated" : "Update Failed";

        isUpdating.store(false);
        });

    
    offsetThread.detach();
}
/*
      offsets  Setup End

    */

