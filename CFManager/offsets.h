#pragma once
#include "Overlay.h"
#include <cstdint>
#include <d3d9.h>
#include <DirectXMath.h>
#include <fstream>
#include <sstream>
#include <iomanip>  
#include <cstdio>
#include <array>
#include <unordered_map>
#include "Classes.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <shlobj.h>        
 
 
static bool showOffsetFinder = false;
const std::string CONFIG_FILE = "offsets.json";
using namespace DirectX;
namespace fs = std::filesystem;
inline std::string update_status = "Idle";
inline uintptr_t CFBASE = 0;
inline uintptr_t CFSHELL = 0;
inline uintptr_t LT_SHELL = 0;
constexpr int MAX_PLAYERS = 16;
inline D3DVIEWPORT9 g_view_port;
using json = nlohmann::json;


 
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


    inline bool ESPDRAWBOX = false;
    inline bool AIMBOTBOX = false;
}

 
extern bool Dcheckbox;
extern bool enableAimbot;
extern float AimFov;
extern float AimSpeed;
extern float MaxAimDistance;
extern int firstHotkey;
extern bool Headcheckbox;
extern bool Healthcheckbox;
extern bool showInfoText;
extern bool Namecheckbox;
extern bool Distancecheckbox;
extern bool weaponcheckbox;
extern bool Filterteams;
extern bool showFPS;
extern bool crosshair_notify;
extern bool Bonecheckbox;
extern bool showEspLines;
extern int esptype;
extern int fov;
extern float boxtk;
extern float hptk;
extern float hdtk;
extern float bonetk;
extern bool draw_radar;
 
extern bool Flogs[6];
 

inline json SerializeCheatConfig() {
    json j;
    j["Dcheckbox"] = Dcheckbox;
    j["enableAimbot"] = enableAimbot;
    j["AimFov"] = AimFov;
    j["AimSpeed"] = AimSpeed;
    j["MaxAimDistance"] = MaxAimDistance;
    j["firstHotkey"] = firstHotkey;
    j["Headcheckbox"] = Headcheckbox;
    j["Healthcheckbox"] = Healthcheckbox;
    j["showInfoText"] = showInfoText;
    j["Namecheckbox"] = Namecheckbox;
    j["Distancecheckbox"] = Distancecheckbox;
    j["weaponcheckbox"] = weaponcheckbox;
    j["Filterteams"] = Filterteams;
    j["showFPS"] = showFPS;
    j["draw_radar"] = draw_radar;
    j["Bonecheckbox"] = Bonecheckbox;
    j["showEspLines"] = showEspLines;
    j["esptype"] = esptype;
    j["fov"] = fov;
    j["boxtk"] = boxtk;
    j["hptk"] = hptk;
    j["hdtk"] = hdtk;
    j["bonetk"] = bonetk;
    j["crosshair_notify"] = crosshair_notify;
     
    

    {
        std::vector<bool> flogsVec(std::begin(Flogs), std::end(Flogs));
        j["Flogs"] = flogsVec;
    }

    return j;
}

inline void DeserializeCheatConfig(const json& j) {
    Dcheckbox = j.value("Dcheckbox", Dcheckbox);
    enableAimbot = j.value("enableAimbot", enableAimbot);
    AimFov = j.value("AimFov", AimFov);
    AimSpeed = j.value("AimSpeed", AimSpeed);
    MaxAimDistance = j.value("MaxAimDistance", MaxAimDistance);
    firstHotkey = j.value("firstHotkey", firstHotkey);
    Headcheckbox = j.value("Headcheckbox", Headcheckbox);
    Healthcheckbox = j.value("Healthcheckbox", Healthcheckbox);
    showInfoText = j.value("showInfoText", showInfoText);
    draw_radar = j.value("draw_radar", draw_radar);
    Namecheckbox = j.value("Namecheckbox", Namecheckbox);
    Distancecheckbox = j.value("Distancecheckbox", Distancecheckbox);
    weaponcheckbox = j.value("weaponcheckbox", weaponcheckbox);
    Filterteams = j.value("Filterteams", Filterteams);
    showFPS = j.value("showFPS", showFPS);
    Bonecheckbox = j.value("Bonecheckbox", Bonecheckbox);
    showEspLines = j.value("showEspLines", showEspLines);
    esptype = j.value("esptype", esptype);
    fov = j.value("fov", fov);
    boxtk = j.value("boxtk", boxtk);
    hptk = j.value("hptk", hptk);
    hdtk = j.value("hdtk", hdtk);
    hdtk = std::clamp(hdtk, 0.0f, 10.0f);
    bonetk = j.value("bonetk", bonetk);
    crosshair_notify = j.value("crosshair_notify", crosshair_notify);

    if (j.contains("Flogs") && j["Flogs"].is_array())
    {
        auto arr = j["Flogs"];
        for (size_t i = 0; i < arr.size() && i < 6; i++)
        {
            Flogs[i] = arr[i].get<bool>();
        }
    }
}


inline std::string getCheatConfigDir() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, path))) {
        fs::path docPath(path);
        docPath /= "Makimura";
        docPath /= "configs";
        if (!fs::exists(docPath)) {
            fs::create_directories(docPath);
        }
        return docPath.string();
    }

    fs::path fallback("configs");
    if (!fs::exists(fallback))
        fs::create_directory(fallback);
    return fallback.string();
}

inline std::string getCheatConfigPath(const std::string& configName) {
    fs::path p = getCheatConfigDir();
    p /= configName + ".json";
    return p.string();
}


inline void SaveCheatConfig(const std::string& configName) {
    json j = SerializeCheatConfig();
    std::ofstream file(getCheatConfigPath(configName));
    if (!file.is_open())
        return;
    file << std::setw(4) << j;
    file.close();
}


inline bool LoadCheatConfig(const std::string& configName) {
    std::ifstream file(getCheatConfigPath(configName));
    if (!file.is_open())
        return false;
    json j;
    try {
        file >> j;
    }
    catch (...) {
        file.close();
        return false;
    }
    file.close();
    DeserializeCheatConfig(j);
    return true;
}


inline std::vector<std::string> GetCheatConfigList() {
    std::vector<std::string> configList;
    fs::path dir(getCheatConfigDir());
    if (fs::exists(dir) && fs::is_directory(dir)) {
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.path().extension() == ".json")
                configList.push_back(entry.path().stem().string());
        }
    }
    return configList;
}
inline std::string toHex(uintptr_t value) {
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::hex << value;
    return ss.str();
}

inline std::string getConfigPath() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, path))) {
        fs::path docPath(path);
        docPath /= "Makimura";
        if (!fs::exists(docPath)) {
            fs::create_directory(docPath);
        }
        docPath /= "offsets.json";
        return docPath.string();
    }

    return "offsets.json";
}

inline bool LoadOffsets() {
    std::ifstream file(getConfigPath());
    if (!file.is_open())
        return false;

    json j;
    try {
        file >> j;
    }
    catch (...) {
        file.close();
        return false;
    }
    file.close();

    if (!j.contains("LT_SHELL") || !j.contains("MYOFFSET") ||
        !j.contains("dwCPlayerSize") || !j.contains("ILTDrawPrim") ||
        !j.contains("dwCPlayerStart"))
    {
        return false;
    }

    try {

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

    std::ofstream file(getConfigPath());
    if (!file.is_open())
        return;

    file << std::setw(4) << j;
    file.close();
}


inline void ClearConfig() {
    std::remove(getConfigPath().c_str());
}




inline bool UpdateOffsets(Memory& mem) {
    std::jthread([&mem](std::stop_token stoken) {
        if (LoadOffsets()) {

            return true;
        }
        size_t CSHELL_SIZE = mem.GetBaseSize("CShell_x64.dll");
        if (!CFSHELL || CSHELL_SIZE == 0) return false;
        size_t CFBASE_SIZE = mem.GetBaseSize("crossfire.exe");
        if (!CFBASE || CFBASE_SIZE == 0) return false;
        uintptr_t FirstsigResult =
            mem.FindSignature(offs::LT_PATTERN, CFSHELL, CFSHELL + CSHELL_SIZE);
        if (!FirstsigResult) return false;

        int32_t offset = mem.Read<int32_t>(FirstsigResult + 3);
        LT_SHELL = FirstsigResult + 7 + offset;

        uintptr_t SecondSigResult = mem.FindSignature(offs::MY_OFFSET_PATTERN,
            CFSHELL, CFSHELL + CSHELL_SIZE);
        if (!SecondSigResult) return false;
        offs::MYOFFSET = mem.Read<int32_t>(SecondSigResult + 3);





        uintptr_t ThirdSigResult = mem.FindSignature(offs::MY_PLAYERSIZE_PATTERN,
            CFSHELL, CFSHELL + CSHELL_SIZE);
        if (!ThirdSigResult) return false;


        offs::dwCPlayerSize = mem.Read<int32_t>(ThirdSigResult + 3);


        uintptr_t FourthSigResult = mem.FindSignature(offs::DRAWPRIM_PATTERN, CFBASE, CFBASE + CFBASE_SIZE);
        if (!FourthSigResult)
            return false;
        int32_t drawprimOffset = mem.Read<int32_t>(FourthSigResult + 3);
        offs::ILTDrawPrim = FourthSigResult + 7 + drawprimOffset;

        SaveOffsets();
        return (LT_SHELL <= CFSHELL + CSHELL_SIZE);
        }).detach();

    return true;
}

