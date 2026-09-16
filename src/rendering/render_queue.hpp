#pragma once

#include "rendering/render_commands.hpp"

#include <vector>
#include <optional>

class RenderQueue {
public:
    // TODO: Be better.
    // TODO: Add getters for these
    std::vector<GeometryCommand> geometryCommands;
    std::vector<PointLightCommand> pointLightCommands;
    std::vector<DirectionalLightCommand> directionalLightCommands;
    std::vector<SpotLightCommand> spotLightCommands;

    std::optional<SkyboxCommand> skyboxCommand;

    void Clear() {
        this->geometryCommands.clear();
        this->pointLightCommands.clear();
        this->directionalLightCommands.clear();
        this->spotLightCommands.clear();
        this->skyboxCommand.reset();
    }

    void Submit(const GeometryCommand &command)
    {
        this->geometryCommands.push_back(command);
    }

    void Submit(const PointLightCommand &command)
    {
        this->pointLightCommands.push_back(command);
    }

    void Submit(const DirectionalLightCommand& command)
    {
        this->directionalLightCommands.push_back(command);
    }

    void Submit(const SpotLightCommand& command)
    {
        this->spotLightCommands.push_back(command);
    }

    void Submit(const SkyboxCommand &command)
    {
        this->skyboxCommand = command;
    }

    const std::vector<GeometryCommand> &GetGeometryCommands()
    {
        return this->geometryCommands;
    }

    const std::vector<PointLightCommand> &GetPointLightCommands()
    {
        return this->pointLightCommands;
    }

    const std::vector<DirectionalLightCommand>& GetDirectionalLightCommands()
    {
        return this->directionalLightCommands;
    }

    const std::vector<SpotLightCommand>& GetSpotLightCommands()
    {
        return this->spotLightCommands;
    }

    const std::optional<SkyboxCommand> &GetSkyboxCommand()
    {
        return this->skyboxCommand;
    }
};