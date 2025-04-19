#include "Misc.h"


using namespace std::chrono_literals;

constexpr int MAX_TEXTURES = 64;
static std::vector<bool> backupOne(MAX_TEXTURES);
static std::vector<bool> backupTwo(MAX_TEXTURES);
static std::vector<bool> backupThree(MAX_TEXTURES);
static bool backupDone = false;



    
    void SuperKill(Memory& mem) {
        static std::unordered_map<uintptr_t, D3DXVECTOR3> headBackup;
        static bool patched = false;
        static bool nodesScanned = false;
        static const D3DXVECTOR3 ENLARGED_HEAD = { 150.0f, 150.0f, 150.0f };

        auto snapshot = g_cacheManager.GetSnapshot();
        bool inGame = snapshot->m_clientShell.inGame();

       
        if (!nodesScanned) {
            LOG("[SuperKill] Scanning for head nodes...\n");

            uintptr_t NODE_MANAGER = mem.Read<uintptr_t>(CFSHELL + NODE_OFFSET);
            if (!NODE_MANAGER) {
                LOG("[SuperKill] Failed to find node manager.\n");
                return;
            }

            
            int validNodes = 0;
            for (int i = 0; i < 100; ++i) {
                uintptr_t nodeAddr = NODE_MANAGER + (i * NODE_SIZE);

                Node node;
                if (!mem.Read(nodeAddr, &node, NODE_SIZE))
                    continue;

               
                if (strstr(node.name, "M-bone Head")) {
                   
                    headBackup[nodeAddr] = node.dimension;

                    LOG("[SuperKill] Found head node @ 0x%p: Dimension = { %.2f, %.2f, %.2f }\n",
                        reinterpret_cast<void*>(nodeAddr),
                        node.dimension.x, node.dimension.y, node.dimension.z);

                    validNodes++;
                }
            }

            LOG("[SuperKill] Found %d valid head nodes to modify.\n", validNodes);
            nodesScanned = true;
        }

        
        if (inGame && !patched) {
             
            Sleep(5000);  

            LOG("[SuperKill] Applying head enlargement to %zu nodes\n", headBackup.size());
            for (const auto& [addr, _] : headBackup) {
                mem.Write<D3DXVECTOR3>(addr + offsetof(Node, dimension), ENLARGED_HEAD);
            }

            patched = true;
        }
        else if (!inGame && patched) {
            
            LOG("[SuperKill] Restoring original head dimensions for %zu nodes\n", headBackup.size());

            for (const auto& [addr, backup] : headBackup) {
                mem.Write<D3DXVECTOR3>(addr + offsetof(Node, dimension), backup);
            }

            patched = false;
        }
    }

 
 

void FastKnives(Memory& mem) {
    static std::unordered_map<uintptr_t, KnifeRates> knifeBackup;
    static bool patched = false;
    static bool knivesScanned = false;
    static bool lmbWasPressed = false;
    static bool delayStarted = false;
    static std::chrono::steady_clock::time_point releaseTime;
    static const float FAST_RATE = 20.0f;

    constexpr int KnifeRateCount = 4;
    constexpr uintptr_t KnifeRate1Offset = 0x4AC;
    constexpr uintptr_t KnifeRate2Offset = 0x50C;
    constexpr int MAX_WEAPON_INDEX = 50;

    auto snapshot = g_cacheManager.GetSnapshot();
    bool inGame = snapshot->m_clientShell.inGame();
    bool lmbPressed =(GetAsyncKeyState(0x01) & 0x8000);

    if (!inGame) {
        if (patched) {
            LOG("[FastKnife] Left game, restoring original knife rates\n");

            auto scatterHandle = mem.CreateScatterHandle();

            for (const auto& [weaponAddr, backup] : knifeBackup) {
                for (int i = 0; i < KnifeRateCount; ++i) {
                    mem.AddScatterWriteRequest(scatterHandle,
                        weaponAddr + KnifeRate1Offset + (i * sizeof(float)),
                        (void*)&backup.rates1[i], sizeof(float));

                    mem.AddScatterWriteRequest(scatterHandle,
                        weaponAddr + KnifeRate2Offset + (i * sizeof(float)),
                        (void*)&backup.rates2[i], sizeof(float));
                }
            }

            mem.ExecuteWriteScatter(scatterHandle);
            mem.CloseScatterHandle(scatterHandle);

            patched = false;
        }

        lmbWasPressed = false;
        delayStarted = false;
        return;
    }

    if (!knivesScanned) {
        LOG("[FastKnife] Scanning for knife weapons (max index: %d)...\n", MAX_WEAPON_INDEX);

        uintptr_t weaponManagerAddress = CFSHELL + WEAPON_MGR;
        uintptr_t weaponManager = mem.Read<uintptr_t>(weaponManagerAddress);

        if (!weaponManager) {
            LOG("[FastKnife] Failed to find weapon manager\n");
            knivesScanned = true;
            return;
        }

        std::vector<uintptr_t> weaponPointers(MAX_WEAPON_INDEX, 0);
        auto scatterHandle = mem.CreateScatterHandle();

        for (int i = 0; i < MAX_WEAPON_INDEX; ++i) {
            mem.AddScatterReadRequest(scatterHandle,
                weaponManager + (i * sizeof(uintptr_t)),
                &weaponPointers[i], sizeof(uintptr_t));
        }

        mem.ExecuteReadScatter(scatterHandle);
        mem.CloseScatterHandle(scatterHandle);

        std::vector<int16_t> weaponClasses;
        std::vector<int> knifeIndices;

        for (int i = 0; i < MAX_WEAPON_INDEX; ++i) {
            if (weaponPointers[i] == 0) continue;

            int16_t weaponClass = mem.Read<int16_t>(weaponPointers[i] + offsetof(CWeapon, WeaponClass));

            if (weaponClass == static_cast<int16_t>(WeaponType::Knife)) {
                knifeIndices.push_back(i);
            }
        }

        int validKnives = 0;

        for (int knifeIndex : knifeIndices) {
            uintptr_t weaponAddr = weaponPointers[knifeIndex];
            KnifeRates rates;

            auto ratesScatterHandle = mem.CreateScatterHandle();

            for (int j = 0; j < KnifeRateCount; ++j) {
                mem.AddScatterReadRequest(ratesScatterHandle,
                    weaponAddr + KnifeRate1Offset + (j * sizeof(float)),
                    &rates.rates1[j], sizeof(float));

                mem.AddScatterReadRequest(ratesScatterHandle,
                    weaponAddr + KnifeRate2Offset + (j * sizeof(float)),
                    &rates.rates2[j], sizeof(float));
            }

            mem.ExecuteReadScatter(ratesScatterHandle);
            mem.CloseScatterHandle(ratesScatterHandle);

            knifeBackup[weaponAddr] = rates;

            LOG("[FastKnife] Found knife @ 0x%p (Index %d)\n",
                reinterpret_cast<void*>(weaponAddr), knifeIndex);

            validKnives++;
        }

        LOG("[FastKnife] Found %d knife weapons to modify.\n", validKnives);
        knivesScanned = true;
    }

    if (knifeBackup.empty()) return;

    if (lmbPressed && !lmbWasPressed) {

        LOG("[FastKnife] Applying fast knife rates\n");

        auto scatterHandle = mem.CreateScatterHandle();

        for (const auto& [weaponAddr, _] : knifeBackup) {
            for (int i = 0; i < KnifeRateCount; ++i) {
                mem.AddScatterWriteRequest(scatterHandle,
                    weaponAddr + KnifeRate1Offset + (i * sizeof(float)),
                    (void*)&FAST_RATE, sizeof(float));

                mem.AddScatterWriteRequest(scatterHandle,
                    weaponAddr + KnifeRate2Offset + (i * sizeof(float)),
                    (void*)&FAST_RATE, sizeof(float));
            }
        }

        mem.ExecuteWriteScatter(scatterHandle);
        mem.CloseScatterHandle(scatterHandle);

        patched = true;
        delayStarted = false;
    }
    else if (!lmbPressed && lmbWasPressed) {

       
        releaseTime = std::chrono::steady_clock::now();
        delayStarted = true;
    }
    else if (!lmbPressed && delayStarted && patched) {

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - releaseTime).count();

        if (elapsed >= 1.5) {

            LOG("[FastKnife] Restoring original knife rates\n");

            auto scatterHandle = mem.CreateScatterHandle();

            for (const auto& [weaponAddr, backup] : knifeBackup) {
                for (int i = 0; i < KnifeRateCount; ++i) {
                    mem.AddScatterWriteRequest(scatterHandle,
                        weaponAddr + KnifeRate1Offset + (i * sizeof(float)),
                        (void*)&backup.rates1[i], sizeof(float));

                    mem.AddScatterWriteRequest(scatterHandle,
                        weaponAddr + KnifeRate2Offset + (i * sizeof(float)),
                        (void*)&backup.rates2[i], sizeof(float));
                }
            }

            mem.ExecuteWriteScatter(scatterHandle);
            mem.CloseScatterHandle(scatterHandle);

            patched = false;
            delayStarted = false;
        }
    }

    lmbWasPressed = lmbPressed;
}
 

void ShootThroughWall(Memory& mem) {
    static uintptr_t wallMgr = 0;
    if (!wallMgr) {
        wallMgr = mem.Read<uintptr_t>(CFSHELL + WALLADDRESS);
        if (!wallMgr) return;
        LOG("[WallPen] wallMgr: 0x%p\n", (void*)wallMgr);
    }
    static WallPenState state = WallPenState::Uninitialized;

    static bool backupOne[MAX_TEXTURES]{};
    static bool backupTwo[MAX_TEXTURES]{};
    static bool backupThree[MAX_TEXTURES]{};

    auto snapshot = g_cacheManager.GetSnapshot();
    bool inGame = snapshot->m_clientShell.inGame();

    if (state == WallPenState::Uninitialized) {
        struct TextureBackup {
            bool one, two, three;
        };

        std::vector<TextureBackup> tempBackup(MAX_TEXTURES);
        auto scatterHandle = mem.CreateScatterHandle();
        for (int i = 0; i < MAX_TEXTURES; ++i) {
            uintptr_t base = wallMgr + (i * TEXTURE_STRUCT_SIZE);
            mem.AddScatterReadRequest(scatterHandle, base + OFFSET_ONE, &tempBackup[i].one, sizeof(bool));
            mem.AddScatterReadRequest(scatterHandle, base + OFFSET_TWO, &tempBackup[i].two, sizeof(bool));
            mem.AddScatterReadRequest(scatterHandle, base + OFFSET_THREE, &tempBackup[i].three, sizeof(bool));
        }
        mem.ExecuteReadScatter(scatterHandle);
        mem.CloseScatterHandle(scatterHandle);

        for (int i = 0; i < MAX_TEXTURES; ++i) {
            backupOne[i] = tempBackup[i].one;
            backupTwo[i] = tempBackup[i].two;
            backupThree[i] = tempBackup[i].three;
        }

        state = WallPenState::Restored;
        LOG("[WallPen] Backup complete.\n");
    }

    if (state == WallPenState::Restored && inGame) {
 
        auto scatterHandle = mem.CreateScatterHandle();
        bool enable = true;

        for (int i = 0; i < MAX_TEXTURES; ++i) {
            uintptr_t base = wallMgr + (i * TEXTURE_STRUCT_SIZE);
            mem.AddScatterWriteRequest(scatterHandle, base + OFFSET_ONE, &enable, sizeof(bool));
            mem.AddScatterWriteRequest(scatterHandle, base + OFFSET_TWO, &enable, sizeof(bool));
            mem.AddScatterWriteRequest(scatterHandle, base + OFFSET_THREE, &enable, sizeof(bool));
        }

        mem.ExecuteWriteScatter(scatterHandle);
        mem.CloseScatterHandle(scatterHandle);
        state = WallPenState::Patched;
        LOG("[WallPen] Patched wall penetration.\n");
    }

    if (state == WallPenState::Patched && !inGame) {
        auto scatterHandle = mem.CreateScatterHandle();
        for (int i = 0; i < MAX_TEXTURES; ++i) {
            uintptr_t base = wallMgr + (i * TEXTURE_STRUCT_SIZE);
            mem.AddScatterWriteRequest(scatterHandle, base + OFFSET_ONE, &backupOne[i], sizeof(bool));
            mem.AddScatterWriteRequest(scatterHandle, base + OFFSET_TWO, &backupTwo[i], sizeof(bool));
            mem.AddScatterWriteRequest(scatterHandle, base + OFFSET_THREE, &backupThree[i], sizeof(bool));
        }
        mem.ExecuteWriteScatter(scatterHandle);
        mem.CloseScatterHandle(scatterHandle);
        state = WallPenState::Restored;
        LOG("[WallPen] Restored wall penetration values.\n");
    }
}

void GodMode(Memory& mem) {
    static bool patched = false;

    struct AnimPatch {
        uintptr_t offset;
        std::string original;
        std::string replacement;  
    };

   
    static std::vector<AnimPatch> animations = {
        
        { aMRun,      "M-run",            "M-HE3Bdown_d" },
        { aMRunR,     "M-run-Rside",      "\x0" },
        { aMRunL,     "M-run-Lside",      "\x0" },
        { aMRunB,     "M-run-Bside",      "\x0" },
        { aMWalk,     "M-walk",           "M-HE3Bdown_d" },
        { aMWalkR,    "M-walk-Rside",     "\x0" },
        { aMWalkL,    "M-walk-Lside",     "\x0" },
        { aMWalkB,    "M-walk-Bside",     "\x0" },
        { aMCWalk,    "M-c-walk",         "\x0" },
        { aMCWalkR,   "M-c-walk-Rside",   "\x0" },
        { aMCWalkL,   "M-c-walk-Lside",   "\x0" },
        { aMCWalkB,   "M-c-walk-Bside",   "\x0" },
        { aMJump,     "M-jump",           "\x0" },
        { aMHitOne,   "M-hit01",          "\x0" },
        { aMHitTwo,   "M-hit02",          "\x0" },
    };

    const uintptr_t CShellBase = mem.GetBaseDaddy("CShell_x64.dll");
    auto snapshot = g_cacheManager.GetSnapshot();
    bool inGame = snapshot->m_clientShell.inGame();

    
    if (inGame && !patched) {
        LOG("[GodMode] Activating godmode\n");

        for (const auto& anim : animations) {
          
            mem.Write(CShellBase + anim.offset, (void*)anim.replacement.c_str(), anim.replacement.length() + 1);
        }

        patched = true;
    }
    
    else if (!inGame && patched) {
        LOG("[GodMode] Restoring normal animations\n");

        for (const auto& anim : animations) {
            mem.Write(CShellBase + anim.offset, (void*)anim.original.c_str(), anim.original.length() + 1);
        }

        patched = false;
    }
}


void noreload(Memory& mem) {
    static bool patched = false;

   
    auto snapshot = g_cacheManager.GetSnapshot();
    bool inGame = snapshot->m_clientShell.inGame();

    struct AnimPatch {
        uintptr_t offset;
        const char* value;
    };

    static std::vector<AnimPatch> noreload = {
        { aSelect,    "fire" },
        { aPostFire,  "asdf" },
        { aReload,    "fire" },
    };

    static std::vector<AnimPatch> restore = {
        { aSelect,    "select" },
        { aPostFire,  "fire" },
        { aReload,    "reload" },
    };

   
    if (inGame && !patched) {
        LOG("[NoReload] Activating no-reload\n");

        for (const auto& p : noreload) {
            for (int x = 0; x < 3; x++)
            {
                mem.Write(CFSHELL + p.offset, (void*)p.value, strlen(p.value) + 4);
            }
            
        }

        patched = true;
    }
    
    else if (!inGame && patched) {
        LOG("[NoReload] Restoring normal animations\n");

        for (const auto& p : restore) {
            mem.Write(CFSHELL + p.offset, (void*)p.value, strlen(p.value) + 4);
        }

        patched = false;
    }
}


void BugDamage(Memory& mem) {
    static double originalValue = -1.0;
    static bool patched = false;

    auto snapshot = g_cacheManager.GetSnapshot();
    bool inGame = snapshot->m_clientShell.inGame();

    uintptr_t zoneMgr = mem.Read<uintptr_t>(CFSHELL + ZONE_MGR);
    if (!zoneMgr) return;

    uintptr_t bugDamageAddr = zoneMgr + BUG_DAMAGE;

    if (inGame && !patched) {
      
        Sleep(5000);   
        originalValue = mem.Read<double>(bugDamageAddr);
        LOG("[CZoneMan] BugDamage @ 0x%llX = %.3f (backing up as double after delay)\n",
            bugDamageAddr, originalValue);

        double zeroValue = 0.0;
        mem.Write<double>(bugDamageAddr, zeroValue);

        patched = true;
    }
    else if (!inGame && patched) {
        LOG("[CZoneMan] Restoring BugDamage @ 0x%llX to %.3f (as double)\n",
            bugDamageAddr, originalValue);

        mem.Write<double>(bugDamageAddr, originalValue);

        double currentValue = mem.Read<double>(bugDamageAddr);
        LOG("[CZoneMan] Verified BugDamage value: %.3f\n", currentValue);

        patched = false;
    }
}
 void SetCameraPerspective(int32_t perspectiveMode, D3DXVECTOR3 camOffset)
{

    auto snapshot = g_cacheManager.GetSnapshot();

    static bool wasInGame = false;
    bool inGame = snapshot->m_clientShell.inGame();

    if (inGame != wasInGame) {
        BugDamage(mem);
        wasInGame = inGame;
    }


    static uintptr_t perspAddr = 0, camPosAddr = 0;
    static int32_t lastMode = -1;
    static D3DXVECTOR3 currentOffset{ 0,0,0 };
    static D3DXVECTOR3 lastOffset{ FLT_MAX, FLT_MAX, FLT_MAX };
    static auto lastCheck = std::chrono::steady_clock::now();
    static auto lastUpdate = std::chrono::steady_clock::now();
    static auto lastApply = std::chrono::steady_clock::now();

    if (!snapshot || !snapshot->m_clientShell.CCamera)
        return;

    uintptr_t camBase = reinterpret_cast<uintptr_t>(snapshot->m_clientShell.CCamera);

    if (!perspAddr || !camPosAddr)
    {
        perspAddr = camBase + offsetof(KLASSES::pCamera, Perspective);
        camPosAddr = camBase + offsetof(KLASSES::pCamera, camerapos);
    }

    if (perspectiveMode != lastMode)
    {
        mem.Write(perspAddr, perspectiveMode);

        lastMode = perspectiveMode;
    }

    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCheck) >= 50ms)
    {
        if (mem.Read<int32_t>(perspAddr) != perspectiveMode)
        {
            mem.Write(perspAddr, perspectiveMode);

            lastMode = perspectiveMode;
        }
        lastCheck = now;
    }



}
 

 
 void ShowFPS(ImDrawList* drawList)
 {
     
     ImFont* font = ImGui::GetFont();
     float originalFontSize = font->FontSize;
     float largeFontSize = originalFontSize * 2.0f;

 
     ImVec2 pos(GetSystemMetrics(SM_CXSCREEN) - 200.0f, 50.0f);

     
     drawList->AddText(font, largeFontSize, pos, IM_COL32(255, 255, 255, 255),
         std::format("FPS: {:.1f}", ImGui::GetIO().Framerate).c_str());
 }


static void TryBoneArray()
{
    static bool hasRun = false;
    if (hasRun) return;
    hasRun = true;

    auto snapshot = g_cacheManager.GetSnapshot();
    if (!snapshot || !snapshot->LocalPlayer.hObject)
        return;

    uintptr_t hObject = reinterpret_cast<uintptr_t>(snapshot->LocalPlayer.hObject);

    constexpr uintptr_t startOffset = 0x2900;
    constexpr uintptr_t endOffset = 0x2AB0;
    constexpr int boneIndex = 6;

    std::unordered_set<uintptr_t> seenPtrs;

    for (uintptr_t offset = startOffset; offset < endOffset; offset += sizeof(uintptr_t))
    {
        uintptr_t bonePtr = mem.Read<uintptr_t>(hObject + offset);
        if (!bonePtr || seenPtrs.count(bonePtr)) continue;

        seenPtrs.insert(bonePtr);

        D3DXMATRIX mat = mem.Read<D3DXMATRIX>(bonePtr + sizeof(D3DXMATRIX) * boneIndex);

        float x = mat._14;
        float y = mat._24;
        float z = mat._34;

        if (abs(x) > 10000.f || abs(y) > 10000.f || abs(z) > 10000.f)
            continue;

        printf("[BONE] Offset 0x%04llX → Bone[6] Pos = { %.2f, %.2f, %.2f }\n",
            offset, x, y, z);
    }

    printf("TryBoneArray finished.\n");
}

