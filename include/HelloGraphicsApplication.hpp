#pragma once

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

class HelloGraphicsApplication
{
public:
    HelloGraphicsApplication();
    ~HelloGraphicsApplication();

    bool isQuitRequested() const;

private:
    void initSDL();

    void createWindow();
    void createGPUDevice();

    void claimWindowForGPUDevice();

    void createGraphicsPipeline();

    SDL_GPUShader *createShader(const std::string &path, SDL_GPUShaderStage stage);
    void mainLoop();
    void pollEvents();
    std::vector<char> readFile(const std::string &fullFilePath);

    SDL_Window *m_window;
    SDL_GPUDevice *m_device;
    SDL_GPUGraphicsPipeline *m_pipeline;

#if defined(NDEBUG)
    static constexpr bool m_GPUDebugEnable = false;
#else
    static constexpr bool m_GPUDebugEnable = true;
#endif

    bool m_quitRequested = false;
};