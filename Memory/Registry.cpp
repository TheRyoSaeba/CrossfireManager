#include "pch.h"
#include "Registry.h"
#include "Memory.h"

std::string c_registry::QueryValue(const char* path, e_registry_type type)
{
    if (!mem.vHandle)
        return "";

    BYTE buffer[0x128];
    DWORD regType = static_cast<DWORD>(type);
    DWORD size = sizeof(buffer);

    if (!VMMDLL_WinReg_QueryValueExU(mem.vHandle, const_cast<LPSTR>(path), &regType, buffer, &size))
    {
        LOG("[!] failed QueryValueExU call\n");
        return "";
    }

    // Check if the requested type is a string.
    if (regType == static_cast<DWORD>(e_registry_type::sz))
    {
        std::wstring wstr = std::wstring(reinterpret_cast<wchar_t*>(buffer));
        return std::string(wstr.begin(), wstr.end());
    }
    // Otherwise, if it's a DWORD, convert it.
    else if (regType == REG_DWORD)
    {
        // Assuming size is at least sizeof(DWORD)
        DWORD value = *reinterpret_cast<DWORD*>(buffer);
        return std::to_string(value);
    }
    // For other types, you might add additional handling...
    return "";
}

