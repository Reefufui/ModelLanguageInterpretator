#include <iostream>

#include "Scanner.hpp"
#include "Parser.hpp"
#include "Token.hpp"
#include "LexicalError.hpp"

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            throw std::runtime_error("[main]: invalid number of arguments");
        }

        mli::Scanner scaner(argv[1]);

        mli::Token token{};
        while((token = scaner.getToken()).getType() != mli::Token::Type::FINISH)
        {
            std::cout << token << "\n";
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const mli::LexicalError& error)
    {
        std::cerr << error << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

