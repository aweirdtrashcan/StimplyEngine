#include "application.h"

#include "game_interface.h"
#include "core/logger.h"
#include "platform/platform.h"
#include "renderer/renderer_frontend.h"
#include "renderer/vulkan/vulkan_backend.h"
#include "window/window.h"

#include <cstdint>
#include <exception>

Application::Application() {}

Application::~Application() {}

int Application::Run() {
    int ret_val = 0;

    if (m_Game == nullptr) {
        Logger::Fatal("Failed to initialize engine: IGame* is nullptr");
        return -1;
    }

    try {
        m_Platform = new Platform();
        Logger::InitializeLogging();
        m_Window = Platform::Construct<Window>(100, 100, 800, 600, "Stimply Engine");
        m_RendererBackend = Platform::Construct<VulkanBackend>("Stimply Engine", *m_Window);
        m_Renderer = Platform::Construct<RendererFrontend>(*m_RendererBackend);
        
        // By this point, the engine is all initialized.

        m_Game->OnBegin();

        while (m_Window->ProcessMessages()) {

            int64_t current_time = Platform::GetTime();
            static int64_t last_time = current_time;
            
            // nanoseconds to seconds
            m_DeltaTime = float(current_time - last_time) / 1e+9;

            last_time = current_time;

            m_Game->OnUpdate(m_DeltaTime);

            RenderPacket packet;
            packet.deltaTime = m_DeltaTime;

            if (!m_Renderer->DrawFrame(packet)) {
                Logger::Fatal("Failed trying to render the frame");
                ret_val = -2;
                break;
            }
        }

        m_Renderer->WaitDeviceIdle();
        m_Game->OnShutdown();
        
        Logger::ShutdownLogging();        
    }
    catch (const std::exception& exception) {
        Logger::Fatal("Error: %s", exception.what());
        Window::MessageBox("Fatal error", exception.what());
        ret_val = -3;
    }

    Platform::Destroy(m_Renderer);
    Platform::Destroy(m_RendererBackend);
    Platform::Destroy(m_Window);
    delete m_Platform;

    Logger::Info("Leaving engine...");

	return ret_val;
}
