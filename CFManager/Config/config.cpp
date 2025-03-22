#include "Config.h"
#include"../Config/globals.h"
#include "../ESP/ESP.h"


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

 std::string getCheatConfigPath(const std::string& configName) {
    fs::path p = getCheatConfigDir();
    p /= configName + ".json";
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

 std::string getConfigPath() {
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
 

std::string aesEncrypt(const std::string& plaintext, const std::string& key, const std::string& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        throw std::runtime_error("Failed to create context");

    int ret = EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
        reinterpret_cast<const unsigned char*>(key.data()),
        reinterpret_cast<const unsigned char*>(iv.data()));
    if (ret != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EncryptInit failed");
    }

    std::vector<unsigned char> ciphertext(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int out_len1 = 0;
    ret = EVP_EncryptUpdate(ctx, ciphertext.data(), &out_len1,
        reinterpret_cast<const unsigned char*>(plaintext.data()),
        static_cast<int>(plaintext.size()));
    if (ret != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EncryptUpdate failed");
    }

    int out_len2 = 0;
    ret = EVP_EncryptFinal_ex(ctx, ciphertext.data() + out_len1, &out_len2);
    if (ret != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EncryptFinal failed");
    }

    EVP_CIPHER_CTX_free(ctx);
    ciphertext.resize(out_len1 + out_len2);
    return std::string(ciphertext.begin(), ciphertext.end());
}


std::string aesDecrypt(const std::string& ciphertext, const std::string& key, const std::string& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        throw std::runtime_error("Failed to create context");

    int ret = EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
        reinterpret_cast<const unsigned char*>(key.data()),
        reinterpret_cast<const unsigned char*>(iv.data()));
    if (ret != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("DecryptInit failed");
    }

    std::vector<unsigned char> plaintext(ciphertext.size());
    int out_len1 = 0;
    ret = EVP_DecryptUpdate(ctx, plaintext.data(), &out_len1,
        reinterpret_cast<const unsigned char*>(ciphertext.data()),
        static_cast<int>(ciphertext.size()));
    if (ret != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("DecryptUpdate failed");
    }

    int out_len2 = 0;
    ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + out_len1, &out_len2);
    if (ret != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("DecryptFinal failed");
    }

    EVP_CIPHER_CTX_free(ctx);
    plaintext.resize(out_len1 + out_len2);
    return std::string(plaintext.begin(), plaintext.end());
}
 json SerializeCheatConfig() {
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

    SaveColors(j);

    {
        std::vector<bool> flogsVec(std::begin(Flogs), std::end(Flogs));
        j["Flogs"] = flogsVec;
    }

    return j;
}

  void DeserializeCheatConfig(const json& j) {
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
 