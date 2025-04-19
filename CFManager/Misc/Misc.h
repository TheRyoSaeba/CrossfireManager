#pragma once
#include "../ESP/ESP.h"
#include <cmath>
#include <unordered_set>
#include <cstring>   

#define aMidle      0x251B7A8
#define aMCidle     0x247f830
#define aMRun       0x247F130
#define aMRunL      0x247FC30
#define aMRunR      0x247FCB0
#define aMRunB      0x247FBB0
#define aMWalk      0x247F0B0
#define aMWalkR     0x247fb30
#define aMWalkL     0x247fab0
#define aMWalkB     0x247fa30
#define aMCWalk     0x247f7b0
#define aMCWalkR    0x247fd30
#define aMCWalkL    0x247fdb0
#define aMCWalkB    0x247fd30
#define aMJump      0x247F1B0
#define aMHitOne    0x247f230
#define aMHitTwo    0x28BF7D0

#define aSelect   0x26c060c
#define aPostFire 0x17daeda
#define aReload   0x17daf00
 
//'M-HEBdown
using namespace std::chrono_literals;
 
void BugDamage();
void TryBoneArray();
void SetCameraPerspective(int32_t perspectiveMode, D3DXVECTOR3 camOffset);
void SuperKill(Memory& mem);
void FastKnives(Memory& mem);
void ShowFPS(ImDrawList* drawList);
void ShootThroughWall(Memory& mem);
void Runcheats();
void GodMode(Memory& mem);
void noreload(Memory& mem);
enum class WallPenState {
    Uninitialized,
    Patched,
    Restored
};
void initMiscCheats();
 void Superkill2();
struct KnifeRates {
    float rates1[4];
    float rates2[4];
};

 

