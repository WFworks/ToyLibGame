#include "ApplicationEntry.h"
#include "Application.h"
#include "SingleInstance.h"

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
