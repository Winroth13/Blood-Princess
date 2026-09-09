#pragma once

#include "rendering/render_commands.hpp"

#include <vector>
#include <optional>

class RenderQueue {
public:
    std::vector<GeometryCommand> geometryCommands;
    std::vector<PointLightCommand> pointLightCommands;
    std::optional<SkyboxCommand> skyboxCommand;

    void Clear() {
        this->geometryCommands.clear();
        this->pointLightCommands.clear();
        this->skyboxCommand.reset();
    }

    void Submit(const GeometryCommand &command) {
        this->geometryCommands.push_back(command);
    }

    void Submit(const PointLightCommand &command) {
        this->pointLightCommands.push_back(command);
    }

    void Submit(const SkyboxCommand &command) {
        this->skyboxCommand = command;
    }
};