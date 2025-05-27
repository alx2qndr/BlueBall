#include <iostream>
#include <stdexcept>
#include <memory>

#include "HelloGraphicsApplication.hpp"

int main(int argc, char **argv)
{
    try
    {
        auto app = std::make_unique<HelloGraphicsApplication>();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}