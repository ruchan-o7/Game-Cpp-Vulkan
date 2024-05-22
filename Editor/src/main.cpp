#include <exception>
#include "Editor.h"
#include <Log.h>
int main(int argc, const char** argv)
{
    try
    {
        FooGame::Editor editor{
            {argc, argv}
        };
        editor.Run();
    }
    catch (const std::exception& e)
    {
        FOO_EDITOR_CRITICAL("Exception occured \n \t {0}", e.what());
    }
    return 0;
}
