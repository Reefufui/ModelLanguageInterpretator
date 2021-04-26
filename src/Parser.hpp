#ifndef PARSER_HPP
#define PARSER_HPP

#include "Token.hpp"
#include "Scanner.hpp"
#include "SyntaxError.hpp"

namespace mli {

    class Parser
    {
        private:
            Token       m_currentToken;
            Token::Type m_currentType;
            int         m_currentValue;
            Scanner     m_scanner;

            void getToken()
            {
                m_currentToken = m_scanner.getToken();
                m_currentType  = m_currentToken.getType();
                m_currentValue = m_currentToken.getValue();
            }

            void checkToken(Token::Type a_expectedType)
            {
                if (m_currentType != a_expectedType)
                {
                    throw SyntaxError(m_currentToken, a_expectedType);
                }
            }

            void leftPart()
            {
            }

            void rightPart()
            {
            }

            void assign()
            {
                leftPart();

                if (m_currentType == Token::Type::ASSIGN)
                {
                    getToken();
                    rightPart();
                }
            }

            void statement()
            {
                if ( m_currentType == Token::Type::READ )
                {
                    getToken();
                    checkToken(Token::Type::OPEN_B);

                    getToken();
                    checkToken(Token::Type::ID);

                    getToken();
                    checkToken(Token::Type::CLOSE_B);

                    getToken();
                    checkToken(Token::Type::SEMICOLON);
                }
                else if ( m_currentType == Token::Type::WRITE )
                {
                    getToken();
                    checkToken(Token::Type::OPEN_B);

                    do
                    {
                        getToken();
                        checkToken(Token::Type::ID);

                        getToken();
                    }
                    while (m_currentType == Token::Type::COMMA);

                    checkToken(Token::Type::CLOSE_B);

                    getToken();
                    checkToken(Token::Type::SEMICOLON);
                }
                else if ( m_currentType == Token::Type::WHILE )
                {
                    getToken();
                    checkToken(Token::Type::OPEN_B);

                    getToken();
                    //assign();

                    checkToken(Token::Type::CLOSE_B);

                    getToken();
                    statement();
                }
                else if ( m_currentType == Token::Type::DO )
                {
                    getToken();
                    statement();

                    getToken();
                    checkToken(Token::Type::WHILE);

                    getToken();
                    checkToken(Token::Type::OPEN_B);

                    getToken();
                    //assign();

                    checkToken(Token::Type::CLOSE_B);

                    getToken();
                    checkToken(Token::Type::SEMICOLON);
                }
                else if ( m_currentType == Token::Type::IF )
                {
                    getToken();
                    checkToken(Token::Type::OPEN_B);

                    getToken();
                    //assign();

                    checkToken(Token::Type::CLOSE_B);

                    getToken();
                    statement();

                    getToken();
                    checkToken(Token::Type::ELSE);
                    getToken();
                    statement();
                }
                else if ( m_currentType == Token::Type::GOTO_MARK )
                {
                    getToken();
                    checkToken(Token::Type::COLON);
                }
                else if ( m_currentType == Token::Type::GOTO )
                {
                    getToken();
                    checkToken(Token::Type::GOTO_MARK);

                    getToken();
                    checkToken(Token::Type::SEMICOLON);
                }
                else if (m_currentType == Token::Type::BEGIN)
                {
                    getToken();
                    statements();
                }
                else
                {
                    std::cout << m_currentToken << "\n";
                }
            }

    void statements()
    {
        while (m_currentType != Token::Type::END)
        {
            statement();
            getToken();
        }
    }

    void constant()
    {
        auto isNumerical = [&]() -> bool
        {
            return m_currentType == Token::Type::INT_CONST
                || m_currentType == Token::Type::REAL_CONST;
        };

        if (m_currentToken == Token::Type::MINUS || m_currentToken == Token::Type::PLUS)
        {
            getToken();

            if (!isNumerical())
            {
                throw SyntaxError(m_currentToken, Token::Type::INT_CONST);
            }
        }
        else if (!isNumerical() && (m_currentType != Token::Type::STRING_CONST))
        {
            throw SyntaxError(m_currentToken, Token::Type::VALUE);
        }
    }

    void declarations()
    {
        auto isDeclaration = [&]() -> bool
        {
            return m_currentType == Token::Type::INT
                || m_currentType == Token::Type::STRING
                || m_currentType == Token::Type::REAL;
        };

        while (isDeclaration())
        {
            do
            {
                getToken();
                checkToken(Token::Type::ID);

                getToken();

                if (m_currentType == Token::Type::ASSIGN)
                {
                    getToken();
                    constant();
                    getToken();
                }
            }
            while (m_currentType == Token::Type::COMMA);

            checkToken(Token::Type::SEMICOLON);
            getToken();
        }
    }

    void program()
    {
        getToken();
        checkToken(Token::Type::ENTRY);
        getToken();
        checkToken(Token::Type::BEGIN);

        getToken();
        declarations();
        statements();

        getToken();
        checkToken(Token::Type::FINISH);
    }

        public:

    Parser(const std::string& a_srcFileName)
        : m_scanner(a_srcFileName)
    {
    }

    void analyze()
    {
        program();

        checkToken(Token::Type::FINISH);
        std::cout << "OK\n";
    }
    };
}

#endif // PARSER_HPP

