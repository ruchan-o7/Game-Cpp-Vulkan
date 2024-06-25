#pragma once

#include "Application.h"
extern FooGame::Application* FooGame::CreateApplication(FooGame::ApplicationCommandLineArgs);

int main(int argc, char** argv)
{
    FooGame::Log::Init();
    auto app = FooGame::CreateApplication({argc, argv});

    app->Run();
    delete app;
}
