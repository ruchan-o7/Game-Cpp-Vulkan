#include <exception>
#include "Editor.h"
int main(int argc, const char** argv)
{
    std::cout << "Argv " << argv << std::endl;
    try
    {
        FooGame::Editor editor{
            {argc, argv}
        };
        editor.Run();
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception " << e.what();
    }
    return 0;
}
