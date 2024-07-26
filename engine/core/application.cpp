#include "application.h"

#include "game_interface.h"
#include "core/logger.h"
#include "platform/platform.h"
#include "renderer/renderer.h"
#include "window/window.h"

#include <cstdint>
#include <exception>

Application::Application() {}

Application::~Application() {}

int Application::Run() {
    // Logger::fatal("This is a test message!");
    // Logger::warning("This is a test message!");
    // Logger::debug("This is a test message!");
    // Logger::info("This is a test message!");

    int ret_val = 0;

    if (m_Game == nullptr) {
        Logger::fatal("Failed to initialize engine: IGame* is nullptr");
        return -1;
    }

    try {
        // TODO: Construct those objects with the allocator.
        m_Platform = new Platform();
        Logger::InitializeLogging();
        m_Window = new (Platform::UAlloc(sizeof(Window))) Window(100, 100, 800, 600, "Stimply Engine");
        m_Renderer = new (Platform::UAlloc(sizeof(Renderer))) Renderer(RendererType::VULKAN, m_Window);
        
        // By this point, the engine is all initialized.

        m_Game->OnBegin();

        float deltaTime = 0.0f;

        while (m_Window->ProcessMessages()) {

            int64_t current_time = Platform::GetTime();
            static int64_t last_time = current_time;
            
            // nanoseconds to seconds
            deltaTime = float(current_time - last_time) / 1e+9;

            last_time = current_time;

            m_Game->OnUpdate(deltaTime);

            if (!m_Renderer->Draw()) {
                Logger::fatal("Some error happened while Drawing, closing the engine...");
                ret_val = -2;
                break;
            }
        }

        m_Game->OnShutdown();
        delete m_Game;
        
        Logger::ShutdownLogging();

        m_Renderer->~Renderer();
        Platform::UFree(m_Renderer);
        m_Window->~Window();
        Platform::UFree(m_Window);
        delete m_Platform;
        
    }
    catch (const std::exception& exception) {
        Logger::fatal("Error: %s", exception.what());
        Window::MessageBox("Fatal error", exception.what());
        ret_val = -3;
    }

    Logger::info("Leaving engine...");

	return ret_val;
}
