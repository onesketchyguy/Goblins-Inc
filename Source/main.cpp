#include <iostream>
#include "GoblinsMain.hpp"

int main(int argc, char* argv[])
{
    srand(static_cast<unsigned int>(time(0)));
    GoblinsMain game{};
    game.Launch();

    return 0;
}