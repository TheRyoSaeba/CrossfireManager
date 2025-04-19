#include "../ESP/ESP.h"
#include "Config.h"
#include"globals.h"
#include <shlobj.h>             
#include <filesystem>           
#include <fstream>              
#include <sstream>             
#include <iomanip>            
#include <cfloat>              
namespace fs = std::filesystem;

namespace fs = std::filesystem;


using namespace KLASSES;

 
void SaveColors(json& j) {
    j["g_EnemyColor"] = { g_EnemyColor.R, g_EnemyColor.G, g_EnemyColor.B, g_EnemyColor.A };
    j["g_AllyColor"] = { g_AllyColor.R, g_AllyColor.G, g_AllyColor.B, g_AllyColor.A };
    j["g_ESPLineColor"] = { g_ESPLineColor.R, g_ESPLineColor.G, g_ESPLineColor.B, g_ESPLineColor.A };
    j["g_NameColor"] = { g_NameColor.R, g_NameColor.G, g_NameColor.B, g_NameColor.A };
    j["g_HeadColor"] = { g_HeadColor.R, g_HeadColor.G, g_HeadColor.B, g_HeadColor.A };
}

void LoadColors(const json& j) {
    if (j.contains("g_EnemyColor")) {
        auto arr = j["g_EnemyColor"];
        g_EnemyColor = { arr[0], arr[1], arr[2], arr[3] };
    }
    if (j.contains("g_AllyColor")) {
        auto arr = j["g_AllyColor"];
        g_AllyColor = { arr[0], arr[1], arr[2], arr[3] };
    }
    if (j.contains("g_ESPLineColor")) {
        auto arr = j["g_ESPLineColor"];
        g_ESPLineColor = { arr[0], arr[1], arr[2], arr[3] };
    }
    if (j.contains("g_NameColor")) {
        auto arr = j["g_NameColor"];
        g_NameColor = { arr[0], arr[1], arr[2], arr[3] };
    }
    if (j.contains("g_HeadColor")) {
        auto arr = j["g_HeadColor"];
        g_HeadColor = { arr[0], arr[1], arr[2], arr[3] };
    }
}

std::string getCheatConfigDir() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, path))) {
        fs::path docPath(path);
        docPath /= xorstr_("Makimura");
        docPath /= xorstr_("configs");
        if (!fs::exists(docPath)) {
            fs::create_directories(docPath);
        }
        return docPath.string();
    }

    fs::path fallback(xorstr_("configs"));
    if (!fs::exists(fallback))
        fs::create_directory(fallback);
    return fallback.string();
}

 std::string getCheatConfigPath(const std::string& configName) {
    fs::path p = getCheatConfigDir();
    p /= configName + xorstr_(".json");
    return p.string();
}


 void SaveCheatConfig(const std::string& configName) {
    json j = SerializeCheatConfig();
    std::ofstream file(getCheatConfigPath(configName));
    if (!file.is_open())
        return;
    file << std::setw(4) << j;
    file.close();
}


  bool LoadCheatConfig(const std::string& configName) {
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


 std::vector<std::string> GetCheatConfigList() {
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
std::string toHex(uintptr_t value) {
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::hex << value;
    return ss.str();
}

 
 

 json SerializeCheatConfig() {
    json j;
    j["color"] = { color[0], color[1], color[2], color[3] };
    j["memwrite"] = memwrite;
    j["draw_radar"] = draw_radar;
    j["draw_enemies_as_arrows"] = draw_enemies_as_arrows;
    j["draw_enemy_names"] = draw_enemy_names;
    j["radar_size_factor"] = radar_size_factor;
    j["radar_elev_threshold"] = radar_elev_threshold;
    j["radar_bg_alpha"] = radar_bg_alpha;
    j["radarRadius"] = radarRadius;
    j["perWeaponConfig"] = perWeaponConfig;
    j["radarMaxDistance"] = radarMaxDistance;
    j["radar_opacity"] = radar_opacity;
    j["radar_zoom"] = radar_zoom;
    j["camera_hacks"] = camera_hacks;
    j["fast_knives"] = fast_knives;
    j["showhidekey"] = showhidekey;
    j["Dcheckbox"] = Dcheckbox;
    j["enableAimbot"] = enableAimbot;
    j["AimFov"] = AimFov;
    j["smoothing"] = smoothing;
    j["aimkey"] = aimkey;
    j["keymode"] = keymode;
    j["MemoryMode"] = MemoryMode;
    j["ToggleorHold"] = ToggleorHold;
    j["showFOVCircle"] = showFOVCircle;
    j["AimSpeed"] = AimSpeed;
    j["selectedAimDevice"] = selectedAimDevice;
    j["AimPosition"] = AimPosition;
    j["TargetSwitch"] = TargetSwitch;
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


    json weaponConfigsJson;
    
    for (int w = (int)WeaponType::Pistol; w <= (int)WeaponType::RAPPEL; ++w)
    {
        WeaponType wType = (WeaponType)w;
      
        const auto& cfg = g_weaponAimbotConfigs[wType];

       
        json weaponObj;
        weaponObj["enableAimbot"] = cfg.enableAimbot;
        weaponObj["AimFov"] = cfg.AimFov;
        weaponObj["MemoryMode"] = cfg.MemoryMode;
        weaponObj["ToggleorHold"] = cfg.ToggleorHold;
        weaponObj["AimSpeed"] = cfg.AimSpeed;
        weaponObj["aimkey"] = cfg.aimkey;
        weaponObj["keymode"] = cfg.keymode;
        weaponObj["smoothing"] = cfg.smoothing;
        weaponObj["selectedAimDevice"] = cfg.selectedAimDevice;
        weaponObj["AimPosition"] = cfg.AimPosition;
        weaponObj["TargetSwitch"] = cfg.TargetSwitch;
        weaponObj["MaxAimDistance"] = cfg.MaxAimDistance;
       

        weaponConfigsJson[std::to_string(w)] = weaponObj;
    }
    j["weaponAimbotConfigs"] = weaponConfigsJson;

    SaveColors(j);

    {
        std::vector<bool> flogsVec(std::begin(Flogs), std::end(Flogs));
        j["Flogs"] = flogsVec;
    }

    return j;
}
 
  void DeserializeCheatConfig(const json& j) {

    showhidekey = j.value("showhidekey", showhidekey);
    memwrite = j.value("memwrite", memwrite);
    Dcheckbox = j.value("Dcheckbox", Dcheckbox);
    enableAimbot = j.value("enableAimbot", enableAimbot);
    AimFov = j.value("AimFov", AimFov);
    aimkey = j.value("aimkey", aimkey);
    keymode = j.value("keymode", keymode);
    showFOVCircle = j.value("showFOVCircle", showFOVCircle);
    MemoryMode = j.value("MemoryMode", MemoryMode);
    AimSpeed = j.value("AimSpeed", AimSpeed);
    smoothing = j.value("smoothing", smoothing);
    smoothing = std::clamp(smoothing, 1, 10);
    perWeaponConfig = j.value("perWeaponConfig", perWeaponConfig);
    AimSpeed = std::clamp(AimSpeed, 0.01f, 0.1f);
    AimPosition = j.value("AimPosition", AimPosition);
    MaxAimDistance = j.value("MaxAimDistance", MaxAimDistance);
    MaxAimDistance = std::clamp(MaxAimDistance, 0.0f, 1000.0f);
    firstHotkey = j.value("firstHotkey", firstHotkey);
    selectedAimDevice = j.value("selectedAimDevice", selectedAimDevice);
    TargetSwitch = j.value("TargetSwitch", TargetSwitch);
    ToggleorHold = j.value("ToggleorHold", ToggleorHold);
    Headcheckbox = j.value("Headcheckbox", Headcheckbox);
    Healthcheckbox = j.value("Healthcheckbox", Healthcheckbox);
    showInfoText = j.value("showInfoText", showInfoText);
    draw_radar = j.value("draw_radar", draw_radar);
    draw_enemies_as_arrows = j.value("draw_enemies_as_arrows", draw_enemies_as_arrows);
    draw_enemy_names = j.value("draw_enemy_names", draw_enemy_names);
    radar_size_factor = j.value("radar_size_factor", radar_size_factor);
    radar_elev_threshold = j.value("radar_elev_threshold", radar_elev_threshold);
    radar_bg_alpha = j.value("radar_bg_alpha", radar_bg_alpha);
    radarRadius = j.value("radarRadius", radarRadius);
    radarMaxDistance = j.value("radarMaxDistance", radarMaxDistance);
    radar_opacity = j.value("radar_opacity", radar_opacity);
    radar_zoom = j.value("radar_zoom", radar_zoom);
    camera_hacks = j.value("camera_hacks", camera_hacks);
    fast_knives = j.value("fast_knives", fast_knives);
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
    if (j.contains("weaponAimbotConfigs"))
    {
        auto wpnConfigObj = j["weaponAimbotConfigs"];
        if (wpnConfigObj.is_object())
        {
            
            for (auto& [key, value] : wpnConfigObj.items())
            {
               
                int weaponId = std::stoi(key);
                
                if (weaponId < (int)WeaponType::Pistol || weaponId >(int)WeaponType::RAPPEL)
                    continue;
                WeaponType wType = (WeaponType)weaponId;
                auto& cfg = g_weaponAimbotConfigs[wType];
                cfg.enableAimbot = value.value<bool>("enableAimbot", cfg.enableAimbot);
                cfg.AimFov = value.value<float>("AimFov", cfg.AimFov);
                cfg.aimkey = value.value<float>("aimkey", cfg.aimkey);
                cfg.smoothing = value.value<float>("smoothing", cfg.smoothing);
                cfg.keymode = value.value<float>("keymode", cfg.keymode);
                cfg.MemoryMode = value.value<int>("MemoryMode", cfg.MemoryMode);
                cfg.ToggleorHold = value.value<int>("ToggleorHold", cfg.ToggleorHold);
                cfg.AimSpeed = value.value<float>("AimSpeed", cfg.AimSpeed);
                cfg.selectedAimDevice = value.value<int>("selectedAimDevice", cfg.selectedAimDevice);
                cfg.AimPosition = value.value<int>("AimPosition", cfg.AimPosition);
                cfg.TargetSwitch = value.value<int>("TargetSwitch", cfg.TargetSwitch);
                cfg.MaxAimDistance = value.value<float>("MaxAimDistance", cfg.MaxAimDistance);
                cfg.firstHotkey = value.value<int>("firstHotkey", cfg.firstHotkey);

            }
        }
    }
    if (j.contains("color"))
    {
        auto arr = j["color"];
        if (arr.size() == 4)
        {
            color[0] = arr[0];
            color[1] = arr[1];
            color[2] = arr[2];
            color[3] = arr[3];
        }
    }

    LoadColors(j);

    if (j.contains("Flogs") && j["Flogs"].is_array())
    {
        auto arr = j["Flogs"];
        for (size_t i = 0; i < arr.size() && i < 6; i++)
        {
            Flogs[i] = arr[i].get<bool>();
        }
    }
}
 