#pragma once

/*
    ================================================================================
    FILE: triggerbot.h
    DESC: Public interface for your “triggerbot” logic—angle calculations,
          crosshair checks, and an actual Triggerbot function that loops and shoots.
    ================================================================================
*/

#include <cmath>       / 
#include <chrono>
#include <thread>
#include <atomic>   " 
#include <DirectXMath.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9math.h> 
#include "Classes.h" 
namespace KLASSES {
    namespace Triggerbot {

        /*
            ----------------------------------------------------------------------------
            CalcAngle():
            - Given src/dst coordinates, returns a D3DVECTOR3 with crossfire-specific angles.
            ----------------------------------------------------------------------------
        */
        D3DXVECTOR3 CalcAngle(D3DXVECTOR3 src, D3DXVECTOR3 dst);

        /*
            ----------------------------------------------------------------------------
            IsEnemyInCrosshair():
            - Checks if difference between (localYaw, localPitch) and aimAngles is within FOV.
            ----------------------------------------------------------------------------
        */
        bool IsEnemyInCrosshair(float localYaw, float localPitch, D3DXVECTOR3  aimAngles);

        /*
            ----------------------------------------------------------------------------
            Triggerbot(...):
            - A loop that scans for valid enemies, checks if they're in crosshair, and shoots.
            ----------------------------------------------------------------------------
        */
        void Triggerbot(Memory& mem);

    } // namespace Triggerbot
} // namespace KLASSES
