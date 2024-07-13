#pragma once

#include <exception>
#include "Application.h"
#include "src/Profile/Profiling.h"
extern FooGame::Application* FooGame::CreateApplication(FooGame::ApplicationCommandLineArgs);

int main(int argc, char** argv)
{
    FooGame::Log::Init();
    FOO_PROFILE_BEGIN_SESSION("Startup", "FooGameProfile-Startup.json");
    auto app = FooGame::CreateApplication({argc, argv});
    FOO_PROFILE_END_SESSION();

    FOO_PROFILE_BEGIN_SESSION("Runtime", "FooGameProfile-Runtime.json");
    try
    {
        app->Run();
    }
    catch (const std::exception& e)
    {
        FOO_CORE_ERROR("Exception occured: {0}", e.what());
    }
    FOO_PROFILE_END_SESSION();

    FOO_PROFILE_BEGIN_SESSION("Shutdown", "FooGameProfile-Shutdown.json");
    delete app;
    FOO_PROFILE_END_SESSION();
}
