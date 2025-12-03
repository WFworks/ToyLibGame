#include "Engine/Core/ApplicationEntry.h"
#include "Engine/Core/Application.h"
#include "Engine/Runtime/SingleInstance.h"

int main(int argc, char** argv)
{
    toy::SingleInstance instance;
    if (!instance.IsLocked()) return 1;

    std::unique_ptr<toy::Application> app = CreateUserApplication();

    if (app->Initialize())
    {
        app->RunLoop();
        app->Shutdown();
        return 0;
    }

    return 2;
}
