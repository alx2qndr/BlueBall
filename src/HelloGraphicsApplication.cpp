#include "HelloGraphicsApplication.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

HelloGraphicsApplication::HelloGraphicsApplication()
{
    initSDL();
    createWindow();
    createGPUDevice();
    claimWindowForGPUDevice();
    createGraphicsPipeline();
    mainLoop();
}

HelloGraphicsApplication::~HelloGraphicsApplication()
{
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool HelloGraphicsApplication::isQuitRequested() const
{
    return m_quitRequested;
}

void HelloGraphicsApplication::initSDL()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        throw std::runtime_error("Could not initialize SDL. SDL error: " + std::string(SDL_GetError()));
    }
}

void HelloGraphicsApplication::createWindow()
{
    m_window = SDL_CreateWindow("Hello, Graphics Application", 1024, 576, SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        throw std::runtime_error("Could not create window. SDL error: " + std::string(SDL_GetError()));
    }
}

void HelloGraphicsApplication::createGPUDevice()
{
    m_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, m_GPUDebugEnable, nullptr);
    if (!m_device)
    {
        throw std::runtime_error("Could not create GPU device. SDL error: " + std::string(SDL_GetError()));
    }
}

void HelloGraphicsApplication::claimWindowForGPUDevice()
{
    if (!SDL_ClaimWindowForGPUDevice(m_device, m_window))
    {
        throw std::runtime_error("Could not claim window for GPU device. SDL error: " + std::string(SDL_GetError()));
    }
}

SDL_GPUShader *HelloGraphicsApplication::createShader(const std::string &path, SDL_GPUShaderStage stage)
{
    auto shaderCode = readFile(path);

    SDL_GPUShaderCreateInfo shaderCreateInfo = {
        .code_size = static_cast<size_t>(shaderCode.size()),
        .code = reinterpret_cast<const uint8_t *>(shaderCode.data()),
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage,
    };

    SDL_GPUShader *shader = SDL_CreateGPUShader(m_device, &shaderCreateInfo);
    if (!shader)
    {
        throw std::runtime_error("Could not create GPU shader from file " + path +
                                 ". SDL error: " + std::string(SDL_GetError()));
    }

    return shader;
}

void HelloGraphicsApplication::createGraphicsPipeline()
{
    SDL_GPUShader *vertexShader = createShader("../shaders/bin/triangle.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX);
    SDL_GPUShader *fragmentShader = createShader("../shaders/bin/triangle.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT);

    SDL_GPUColorTargetDescription colorTargetDescription = {
        .format = SDL_GetGPUSwapchainTextureFormat(m_device, m_window),
    };

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info =
            {
                .color_target_descriptions = &colorTargetDescription,
                .num_color_targets = 1,
            },
    };

    m_pipeline = SDL_CreateGPUGraphicsPipeline(m_device, &pipelineCreateInfo);
    if (!m_pipeline)
    {
        throw std::runtime_error("Could not create GPU graphics pipeline. SDL error: " + std::string(SDL_GetError()));
    }

    SDL_ReleaseGPUShader(m_device, vertexShader);
    SDL_ReleaseGPUShader(m_device, fragmentShader);
}

void HelloGraphicsApplication::mainLoop()
{
    while (!isQuitRequested())
    {
        pollEvents();

        SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(m_device);

        SDL_GPUTexture *swapchainTexture;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, m_window, &swapchainTexture, nullptr, nullptr))
        {
            throw std::runtime_error("Could not acquire GPU swapchain texture. SDL error: " +
                                     std::string(SDL_GetError()));
        }

        if (swapchainTexture)
        {
            SDL_GPUColorTargetInfo colorTargetInfo = {
                .texture = swapchainTexture,
                .clear_color = {0.1f, 0.2f, 0.3f, 1.0f},
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
            };

            SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);
            if (!renderPass)
            {
                throw std::runtime_error("Could not begin render pass. SDL error: " + std::string(SDL_GetError()));
            }

            SDL_BindGPUGraphicsPipeline(renderPass, m_pipeline);
            SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
            SDL_EndGPURenderPass(renderPass);
        }

        if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
        {
            throw std::runtime_error("Could not submit GPU command buffer. SDL error: " + std::string(SDL_GetError()));
        }
    }
}

void HelloGraphicsApplication::pollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            m_quitRequested = true;
            break;
        default:
            break;
        }
    }
}

std::vector<char> HelloGraphicsApplication::readFile(const std::string &fullFilePath)
{
    std::ifstream file(fullFilePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file " + fullFilePath + " for reading.");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}
