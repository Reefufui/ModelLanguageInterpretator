#ifndef PARSER_HPP
#define PARSER_HPP

#include "Token.hpp"
#include "Scanner.hpp"

namespace mli {

    class Parser
    {
        private:
            Token     m_currentToken;
            Token::Type m_currentType;
            int       m_currentValue;
            Scanner   m_scanner;

            void getToken()
            {
                m_currentToken = m_scanner.getToken();
                m_currentType  = m_currentToken.getType();
                m_currentValue = m_currentToken.getValue();
            }

            void value()
            {
                if ( m_currentType == Token::Type::ID )
                {
                    getToken();
                }
                else if ( m_currentType == Token::Type::STRING
                        || m_currentType == Token::Type::INT
                        || m_currentType == Token::Type::REAL)
                {
                    getToken();
                }
                else if ( m_currentType == Token::Type::POINT )
                {
                    getToken(); 
                }
                else if ( m_currentType == Token::Type::NOT )
                {
                    getToken(); 
                    value(); 
                }
                else if ( m_currentType == Token::Type::OPEN_B )
                {
                    getToken(); 
                    expression();

                    if ( m_currentType != Token::Type::CLOSE_B)
                    {
                        throw std::runtime_error("[Parser::value]: expected ')'");
                    }

                    getToken();
                }
                else 
                {
                    throw std::runtime_error("[Parser::value]: unexpected expression");
                }
            }

            void timesLikeOperation()
            {
                value();

                while ( m_currentType == Token::Type::MULTIPLY
                        || m_currentType == Token::Type::DIVIDE
                        || m_currentType == Token::Type::AND)
                {
                    getToken();
                    value();
                }
            }

            void subexpression()
            {
                timesLikeOperation();

                while ( m_currentType == Token::Type::PLUS
                        || m_currentType == Token::Type::MINUS
                        || m_currentType == Token::Type::OR)
                {
                    getToken();
                    timesLikeOperation();
                }
            }

            void expression()
            {
                subexpression();

                if ( m_currentType == Token::Type::EQ
                        || m_currentType == Token::Type::NEQ
                        || m_currentType == Token::Type::LESS
                        || m_currentType == Token::Type::GREATER
                        || m_currentType == Token::Type::LEQ
                        || m_currentType == Token::Type::GEQ)
                {
                    getToken();
                    subexpression();
                }
            }

            void sentence()
            {
                if ( m_currentType == Token::Type::WHILE )
                {
                    getToken();
                    expression();

                    if ( m_currentType != Token::Type::DO )
                    {
                        throw std::runtime_error("[Parser::sentence]: do expected after while");
                    }

                    getToken();
                    sentence();
                }
                if ( m_currentType == Token::Type::DO )
                {
                    getToken();
                    sentence();

                    if ( m_currentType != Token::Type::WHILE )
                    {
                        throw std::runtime_error("[Parser::sentence]: while expected after do");
                    }

                    getToken();
                    expression();
                }
                else if ( m_currentType == Token::Type::READ )
                {
                    getToken();

                    if ( m_currentType != Token::Type::OPEN_B )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected '('");
                    }

                    getToken();

                    if ( m_currentType != Token::Type::ID )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected variable name");
                    }

                    getToken();

                    if ( m_currentType != Token::Type::CLOSE_B )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected ')'");
                    }

                    getToken();
                }
                else if ( m_currentType == Token::Type::WRITE )
                {
                    getToken();

                    if ( m_currentType != Token::Type::OPEN_B )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected '('");
                    }

                    getToken();
                    expression();

                    if ( m_currentType != Token::Type::CLOSE_B )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected ')'");
                    }

                    getToken();
                }
                else if ( m_currentType == Token::Type::GOTO )
                { 
                    getToken();

                    if ( m_currentType != Token::Type::ID )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected identificator");
                    }

                    getToken();

                    if ( m_currentType != Token::Type::COLON )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected ':'");
                    }
                }
                else if ( m_currentType == Token::Type::ID )
                { 
                    getToken();

                    if ( m_currentType != Token::Type::ASSIGN )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected '='");
                    }

                    getToken();
                    expression();
                }
                else
                {
                    block();
                }
            }

            void block()
            {
                if (m_currentType != Token::Type::BEGIN)
                {
                    throw std::runtime_error("[Parser::block]: '{' expected");
                }

                getToken();
                sentence();

                while (m_currentType == Token::Type::SEMICOLON)
                {
                    getToken();
                    sentence();
                }

                if (m_currentType != Token::Type::END)
                {
                    throw std::runtime_error("[Parser::block]: '}' expected");
                }

                getToken();
            }

            void variable()
            {
                if (m_currentType != Token::Type::ID)
                {
                    throw std::runtime_error("[Parser::variable]: variable name expected");
                }

                getToken();

                while (m_currentType == Token::Type::COMMA)
                {
                    getToken();

                    if (m_currentType != Token::Type::ID)
                    {
                        throw std::runtime_error("[Parser::variable]: variable name expected");
                    }

                    getToken();
                }

                if (m_currentType != Token::Type::COLON)
                {
                    throw std::runtime_error("[Parser::variable]: ':' expected");
                }

                getToken();

                bool isVariableType{ m_currentType != Token::Type::INT
                    && m_currentType != Token::Type::STRING
                        && m_currentType != Token::Type::REAL };

                if (!isVariableType)
                {
                    throw std::runtime_error("[Parser::variable]: variable type expected");
                }

                getToken();
            }

            void declare()
            {
                if (m_currentType != Token::Type::VARIABLE)
                {
                    throw std::runtime_error("[Parser::declare]: var expected");
                }

                getToken();
                variable();

                while (m_currentType == Token::Type::COMMA)
                {
                    getToken();
                    variable();
                }
            }

            void program()
            {
                if (m_currentType != Token::Type::ENTRY)
                {
                    throw std::runtime_error("[Parser::program]: program expected");
                }

                getToken();
                declare();

                if (m_currentType != Token::Type::SEMICOLON)
                {
                    throw std::runtime_error("[Parser::program]: ';' expected");
                }

                getToken();
                block();
            }


        public:

            Parser(const std::string& a_srcFileName)
                : m_scanner(a_srcFileName)
            {
            }

            void analyze()
            {
                getToken();
                program();

                if (m_currentType != Token::Type::FINISH)
                {
                    throw std::runtime_error("[Parser::analyze]: wrong sequence");
                }

                std::cout << "OK\n";
            }
    };
}

#endif // PARSER_HPP

