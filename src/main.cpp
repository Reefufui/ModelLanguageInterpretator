#include <iostream>

#include "Scanner.hpp"
#include "Parser.hpp"
#include "Executer.hpp"

namespace mli {

    class Interpretator
    {
        private:
            std::string m_fileName;
            Parser      m_parser;
            Executer    m_executer;

        public:

            Interpretator(const char* a_fileName)
                : m_fileName(a_fileName), m_parser(a_fileName)
            {
                m_parser.analyze();
            }

            void run()
            {
                m_executer.executePoliz(m_parser.fetchPoliz());
            }

            void lexicalUnitTest()
            {
                Scanner scaner(m_fileName);

                Token token{};
                while((token = scaner.getToken()).getType() != Token::Type::FINISH)
                {
                    std::cout << token << "\n";
                }

                std::cout << "strings:\n";
                for (auto& s : State::s_strings)
                {
                    std::cout << "\t" << s << "\n";
                }

                std::cout << "reals:\n";
                for (auto& r : State::s_realNumbers)
                {
                    std::cout << "\t" << r << "\n";
                }
            }

            void semanticalUnitTest()
            {
                m_parser.dumpPoliz();
            }
    };
}

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            throw std::runtime_error("[main]: invalid number of arguments");
        }

        mli::Interpretator app{ argv[1] };
        app.semanticalUnitTest();
        app.run();
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
    catch (const mli::SemanticError<mli::Mark>& error)
    {
        std::cerr << error << std::endl;
        return EXIT_FAILURE;
    }
    catch (const mli::SemanticError<mli::Token::Type>& error)
    {
        std::cerr << error << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

