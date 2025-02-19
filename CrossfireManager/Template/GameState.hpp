#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "Memory.h"
#include <classes.h>
#include <vector>

namespace KLASSES {

    // Shared GameState (Thread-Safe)
    struct GameState {
        LTClientShell clientShell;
        pPlayer localPlayer;
        std::vector<pPlayer> players;
        std::vector<D3DXVECTOR3> headPositions;

        GameState() : players(24), headPositions(24) {}
    };

    using GameStatePtr = std::shared_ptr<GameState>;

} // namespace KLASSES

#endif // GAME_STATE_HPP
