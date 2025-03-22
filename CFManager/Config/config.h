#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

static std::string g_encryptedOffsets;

void SaveColors(json& j);
void LoadColors(const json& j);
std::string aesEncrypt(const std::string& plaintext, const std::string& key, const std::string& iv);
std::string aesDecrypt(const std::string& ciphertext, const std::string& key, const std::string& iv);
std::string getConfigPath();
std::string toHex(uintptr_t value);
std::vector<std::string> GetCheatConfigList();
void SaveCheatConfig(const std::string& configName);
bool LoadCheatConfig(const std::string& configName);
std::string getCheatConfigPath(const std::string& configName);
std::string getCheatConfigDir();
json SerializeCheatConfig();
void DeserializeCheatConfig(const json& j);
