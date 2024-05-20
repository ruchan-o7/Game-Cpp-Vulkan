#include "Editor.h"
int main(int argc, const char** argv)
{
    FooGame::Editor editor{
        {argc, argv}
    };
    editor.Run();
}
