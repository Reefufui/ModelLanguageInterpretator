#include <iostream>

#include "Scanner.hpp"
#include "Parser.hpp"

#define DEBUG

#ifdef DEBUG
void lexicalUnitTest(const char* a_testFileName)
{
    mli::Scanner scaner(a_testFileName);

    mli::Token token{};
    while((token = scaner.getToken()).getType() != mli::Token::Type::FINISH)
    {
        std::cout << token << "\n";
    }

    std::cout << "strings:\n";
    for (auto& s : mli::State::s_strings)
    {
        std::cout << "\t" << s << "\n";
    }

    std::cout << "reals:\n";
    for (auto& r : mli::State::s_realNumbers)
    {
        std::cout << "\t" << r << "\n";
    }
}

void semanticalUnitTest(const char* a_testFileName)
{
    mli::Parser parser(a_testFileName);

    parser.analyze();
}
#endif

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            throw std::runtime_error("[main]: invalid number of arguments");
        }

        //lexicalUnitTest(argv[1]);
        semanticalUnitTest(argv[1]);
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
    catch (const mli::SyntaxError& error)
    {
        std::cerr << error << std::endl;
        return EXIT_FAILURE;
    }
    catch (const mli::SemanticError<mli::Ident>& error)
    {
        std::cerr << error << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

