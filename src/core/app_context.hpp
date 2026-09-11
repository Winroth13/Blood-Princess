#pragma once

class Application;
class Window;
class Renderer;
class AssetManager;
class SceneManager;
class AudioManager;
class ImGuiLayer;
class Editor;

struct AppContext
{
    Application *application;
	Window *window;
	Renderer *renderer;
    AssetManager *assetManager;
    SceneManager *sceneManager;
    AudioManager *audioManager;
    ImGuiLayer *imGuiLayer;
    Editor *editor;
};