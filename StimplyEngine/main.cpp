#include "stdafx.h"

#include "Application.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    Application app(L"Stimply Engine", hInstance);

    app.Run();

    return FALSE;
}

int main()
{
    Application* app = new Application(L"Stimply Engine", GetModuleHandleW(0));

    app->Run();

    delete app;

    return 0;
}