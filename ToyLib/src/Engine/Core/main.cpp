#include "Engine/Core/ApplicationEntry.h"
#include "Engine/Core/Application.h"
#include "Engine/Runtime/SingleInstance.h"

int main(int argc, char** argv)
{
    SingleInstance instance;
    if (!instance.IsLocked()) return 1;

    std::unique_ptr<Application> app = CreateUserApplication();

    if (app->Initialize())
    {
        app->RunLoop();
        app->Shutdown();
        return 0;
    }

    return 2;
}
