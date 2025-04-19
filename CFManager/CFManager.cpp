#pragma once 
#include "../cache/CacheManager.h"
#include "CFManager.h"
#include "render.h"
#include "Authentication/auth.hpp"
#include "../region_header.h"
 

CacheManager g_cacheManager;

int main()
{
    SetConsoleTitleA(xorstr_("MakimuraLoader"));
    LOG_INFO("Connecting to DMA....\n");
init:
    while (!mem.Init(GAME_NAME, true))
    {
        logFailure();
        Sleep(2000);
    }

    CFBASE = mem.GetBaseDaddy(GAME_NAME);
    CFSHELL = mem.GetBaseDaddy(SHELLNAME);

    if (CFBASE == 0 || CFSHELL == 0)
    {
        if (mem.vHandle)
        {
            VMMDLL_Close(mem.vHandle);
            mem.vHandle = nullptr;
            Memory::DMA_INITIALIZED = FALSE;
            Memory::PROCESS_INITIALIZED = FALSE;
        }
        LOG_ERROR2("Failed to Find CFSHELL...\nPlease make sure game is running!\n");
        Sleep(2000);
        goto init;
    }

    if (!mem.GetKeyboard()->InitKeyboard())
    {
        LOG_ERROR2("[!] Keyboard initialization failed.\n");
    }
    else
    {
        LOG_SUCCESS("Keyboard initialized.\n");
    }

    int result = mem.FixCr3();
    if (result == -1)
    {
        LOG_ERROR2("CR3 fix failed....\n");
        Sleep(3000);
        exit(1);
    }

    if (result == 0)
    {
        if (!mem.cachePML4())
        {
            LOG_ERROR2("Failed to Fix CR3 !\n");
            Sleep(3000);
            exit(1);
        }
        LOG_SUCCESS("Fixed CR3!");
    }
    LOG_SUCCESS("Initializing Cheat....\n");

    bool success = false;
    std::thread offsetThread([&]() {
        success = PopulateOffsets();
        });
    offsetThread.join();

    if (!success)
    {
        LOG_ERROR3("Cheat Failed to Update... Exiting... \n");
        Sleep(3000);
        exit(EXIT_FAILURE);
    }

    LOG_SUCCESS("Cheat Initialized... Loading Menu..\n");

    g_cacheManager.StartUpdateThread(mem);

     
     
    mem.Refreshing();

    MainThread::Render_Loop();
    return 0;
}

/*if (mem.DumpMemory(mem.GetBaseDaddy(GAME_NAME), "cf_dumps.exe") &&
        mem.DumpMemory(mem.GetBaseDaddy(SHELLNAME), "cshell_dump.dll"))
    {
        LOG_SUCCESS("Dumped Game Successfully..");
    }   */