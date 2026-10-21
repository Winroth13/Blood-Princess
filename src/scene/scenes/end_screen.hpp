#pragma once

#include "scene/scenes/scene_headers.hpp"

inline void DrawScores(Entity* parent, GameState *gameState, UI::Factory &ui, UI::Anchor centre)
{
    Entity* scoresGroup = ui.CreateGroup(parent, { 250, 350 }, "scores_group");
    UI::Factory::Place(scoresGroup, centre, { 0, 0 });

    UI::Factory::Place(ui.CreateText(scoresGroup, "Score: " + std::to_string(gameState->GetTotalScore()), 70.0f, "score"), UI::Anchor::TOP, { 0.0f, 0.0f });

    std::vector<uint32_t> &floorScores = gameState->GetScores();
    std::string floorScoreTextLeft;
    std::string floorScoreTextRight;
    uint32_t NumFloorsComplete = gameState->GetNumFloorsComplete();

    for (uint32_t i = NumFloorsComplete; i > 0; i--)
    {
        floorScoreTextLeft += "Floor " + std::to_string(i) + ":\n";
        floorScoreTextRight += std::to_string(floorScores[i - 1]) + "\n";
    }

    // Designed for five floors, but stil works if you add more
    float floorScoreSize = min(30.0f, 150.f / static_cast<float>(NumFloorsComplete));

    UI::Factory::Place(ui.CreateText(scoresGroup, floorScoreTextLeft, floorScoreSize, "score_left"), UI::Anchor::TOP_LEFT, { 0.0f, 125.0f });
    UI::Factory::Place(ui.CreateText(scoresGroup, floorScoreTextRight, floorScoreSize, "score_right"), UI::Anchor::TOP_RIGHT, { 0.0f, 125.0f });
}