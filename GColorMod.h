#include <iostream>
#include "include/GPixel.h"

struct GColorMod
{
    int r, g, b, a;

    void printCol() const
    {
        std::cout << a << " " << r << " " << g << " " << b << "\n";
    }
};
