#ifndef PARSER_HPP
#define PARSER_HPP

#include "Token.hpp"
#include "Scanner.hpp"
#include "SyntaxError.hpp"
#include "Semantic.hpp"

namespace mli {

    class Parser
    {
        private:
            Token       m_currentToken;
            Token::Type m_currentType;
            int         m_currentValue;
            Scanner     m_scanner;
            Semantic    m_validator;

            std::vector<Token> m_poliz;

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

            void getToken(Token::Type a_expectedType)
            {
                getToken();
                checkToken(a_expectedType);
            }

            Token::Type toConstType(Token::Type a_token)
            {
                return (a_token == Token::Type::STRING) ? Token::Type::STRING_CONST
                    : (a_token == Token::Type::REAL) ? Token::Type::REAL_CONST
                    : (a_token == Token::Type::INT) ? Token::Type::INT_CONST : a_token;
            }

            void value()
            {
                if (m_currentType == Token::Type::ID)
                {
                    m_validator.declarationCheck(m_currentValue);
                    m_poliz.push_back(m_currentToken);

                    Ident& variable = m_validator.fetchVariable(m_currentToken);
                    m_validator.pushType(toConstType(variable.getType()));

                    getToken();
                }
                else
                {
                    constant();
                    getToken();
                }
            }

            void multiplierOperand()
            {
                std::vector<Token> localStack;

                while (m_currentType == Token::Type::NOT)
                {
                    localStack.push_back(m_currentToken);
                    getToken();
                }

                while (m_currentType == Token::Type::MINUS || m_currentType == Token::Type::PLUS)
                {
                    Token::Type unaryType = (m_currentType == Token::Type::MINUS) ? Token::Type::UNARY_MINUS
                        : (m_currentType == Token::Type::PLUS) ? Token::Type::UNARY_PLUS : m_currentType;
                    m_currentToken.setType(unaryType);
                    localStack.push_back(m_currentToken);
                    getToken();
                }

                if (m_currentType == Token::Type::OPEN_B)
                {
                    getToken();
                    expression();

                    checkToken(Token::Type::CLOSE_B);

                    getToken();
                }
                else
                {
                    value();
                }

                if (localStack.size())
                {
                    auto push = [&](Token& t)
                    {
                        m_poliz.push_back(t);
                        m_validator.popWithOperator(t.getType());
                    };

                    std::for_each(localStack.rbegin(), localStack.rend(), push);
                }
            }

            void termOperand()
            {
                multiplierOperand();

                while (m_currentType == Token::Type::MULTIPLY || m_currentType == Token::Type::DIVIDE)
                {
                    Token operand = m_currentToken;
                    getToken();
                    multiplierOperand();

                    m_poliz.push_back(operand);
                    m_validator.popWithOperator(operand.getType());
                }
            }

            void compOperand()
            {
                termOperand();

                while (m_currentType == Token::Type::PLUS || m_currentType == Token::Type::MINUS)
                {
                    Token operand = m_currentToken;
                    getToken();
                    termOperand();

                    m_poliz.push_back(operand);
                    m_validator.popWithOperator(operand.getType());
                }
            }

            void andOperand()
            {
                compOperand();

                while ( m_currentType >= Token::Type::EQ && m_currentType <= Token::Type::GEQ )
                {
                    Token operand = m_currentToken;
                    getToken();
                    compOperand();

                    m_poliz.push_back(operand);
                    m_validator.popWithOperator(operand.getType());
                }
            }

            void orOperand()
            {
                andOperand();

                while (m_currentType == Token::Type::AND)
                {
                    Token operand = m_currentToken;
                    getToken();
                    andOperand();

                    m_poliz.push_back(operand);
                    m_validator.popWithOperator(operand.getType());
                }
            }

            void assignOperand()
            {
                orOperand();

                while (m_currentType == Token::Type::OR)
                {
                    Token operand = m_currentToken;
                    getToken();
                    orOperand();

                    m_poliz.push_back(operand);
                    m_validator.popWithOperator(operand.getType());
                }
            }

            void expression()
            {
                std::vector<Token> localStack(0);

                m_validator.setRValueFlag(false);
                assignOperand();

                while (m_currentType == Token::Type::ASSIGN)
                {
                    m_validator.isLValue();

                    localStack.push_back(m_currentToken);

                    getToken();
                    assignOperand();
                }

                if (localStack.size())
                {
                    auto push = [&](Token& t)
                    {
                        m_poliz.push_back(t);
                        m_validator.popWithOperator(t.getType());
                    };

                    std::for_each(localStack.rbegin(), localStack.rend(), push);
                }
            }

            void statement()
            {
                Token operatorToPush = m_currentToken;

                if ( m_currentType == Token::Type::READ )
                {
                    getToken(Token::Type::OPEN_B);
                    {
                        getToken(Token::Type::ID);
                        m_validator.declarationCheck(m_currentValue);
                        m_poliz.push_back(m_currentToken);
                    }
                    getToken(Token::Type::CLOSE_B);

                    getToken(Token::Type::SEMICOLON);

                    m_poliz.push_back(operatorToPush);
                }
                else if ( m_currentType == Token::Type::WRITE )
                {
                    getToken(Token::Type::OPEN_B);

                    do
                    {
                        getToken();
                        expression();

                        m_validator.popWithOperator(Token::Type::SEMICOLON);
                    }
                    while (m_currentType == Token::Type::COMMA);

                    checkToken(Token::Type::CLOSE_B);

                    getToken(Token::Type::SEMICOLON);

                    m_poliz.push_back(operatorToPush);
                }
                else if ( m_currentType == Token::Type::WHILE )
                {
                    size_t startPolizID = m_poliz.size();
                    size_t endLabel = -1;

                    getToken(Token::Type::OPEN_B);
                    {
                        getToken();
                        expression();
                        m_validator.popWithOperator(Token::Type::POLIZ_FALSE_GO);

                        operatorToPush.setType(Token::Type::POLIZ_LABEL);
                        endLabel = m_poliz.size();
                        m_poliz.push_back(operatorToPush);

                        m_currentToken.setType(Token::Type::POLIZ_FALSE_GO);
                        m_poliz.push_back(m_currentToken);
                    }
                    checkToken(Token::Type::CLOSE_B);
                    {
                        getToken();
                        statement();

                        operatorToPush.setType(Token::Type::POLIZ_LABEL);
                        operatorToPush.setValue(startPolizID);
                        m_poliz.push_back(operatorToPush);

                        operatorToPush.setType(Token::Type::POLIZ_GO);
                        operatorToPush.setValue(0);
                        m_poliz.push_back(operatorToPush);
                    }

                    m_poliz[endLabel].setValue(m_poliz.size());
                }
                else if (m_currentType == Token::Type::DO)
                {
                    size_t startPolizID = m_poliz.size();

                    {
                        getToken();
                        statement();
                    }
                    getToken(Token::Type::WHILE);
                    getToken(Token::Type::OPEN_B);
                    {
                        getToken();
                        expression();
                        m_validator.popWithOperator(Token::Type::POLIZ_FALSE_GO);

                        operatorToPush.setType(Token::Type::POLIZ_LABEL);
                        operatorToPush.setValue(m_poliz.size() + 4);
                        m_poliz.push_back(operatorToPush);

                        operatorToPush.setType(Token::Type::POLIZ_FALSE_GO);
                        operatorToPush.setValue(0);
                        m_poliz.push_back(operatorToPush);

                        operatorToPush.setType(Token::Type::POLIZ_LABEL);
                        operatorToPush.setValue(startPolizID);
                        m_poliz.push_back(operatorToPush);

                        operatorToPush.setType(Token::Type::POLIZ_GO);
                        operatorToPush.setValue(0);
                        m_poliz.push_back(operatorToPush);
                    }
                    checkToken(Token::Type::CLOSE_B);

                    getToken(Token::Type::SEMICOLON);
                }
                else if (m_currentType == Token::Type::IF)
                {
                    size_t elseLabel;
                    size_t endLabel;

                    getToken(Token::Type::OPEN_B);
                    {
                        getToken();
                        expression();
                        m_validator.popWithOperator(Token::Type::POLIZ_FALSE_GO);

                        operatorToPush.setType(Token::Type::POLIZ_LABEL);
                        elseLabel = m_poliz.size();
                        m_poliz.push_back(operatorToPush);

                        operatorToPush.setType(Token::Type::POLIZ_FALSE_GO);
                        m_poliz.push_back(operatorToPush);
                    }
                    checkToken(Token::Type::CLOSE_B);
                    {
                        getToken();
                        statement();

                        operatorToPush.setType(Token::Type::POLIZ_LABEL);
                        endLabel = m_poliz.size();
                        m_poliz.push_back(operatorToPush);

                        operatorToPush.setType(Token::Type::POLIZ_GO);
                        m_poliz.push_back(operatorToPush);
                    }
                    getToken(Token::Type::ELSE);
                    {
                        m_poliz[elseLabel].setValue(m_poliz.size());
                        getToken();
                        statement();
                    }

                    m_poliz[endLabel].setValue(m_poliz.size());
                }
                else if (m_currentType == Token::Type::GOTO_MARK)
                {
                    m_validator.mark(m_currentValue, m_poliz.size());
                    getToken(Token::Type::COLON);
                }
                else if (m_currentType == Token::Type::GOTO)
                {
                    getToken(Token::Type::GOTO_MARK);
                    Mark& mark = m_validator.fetchMark(m_currentToken);

                    operatorToPush.setType(Token::Type::POLIZ_LABEL);
                    operatorToPush.setValue((uint32_t)mark.getPolizID());
                    m_poliz.push_back(operatorToPush);

                    operatorToPush.setType(Token::Type::POLIZ_GO);
                    operatorToPush.setValue(0);
                    m_poliz.push_back(operatorToPush);

                    getToken(Token::Type::SEMICOLON);
                }
                else if (m_currentType == Token::Type::BEGIN)
                {
                    getToken();
                    statements();
                }
                else
                {
                    expression();
                    checkToken(Token::Type::SEMICOLON);
                    m_validator.popWithOperator(m_currentType);
                    m_poliz.push_back(m_currentToken);
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

                m_poliz.push_back(m_currentToken);
                m_validator.pushType(m_currentType);
                m_validator.setRValueFlag(true);
            }

            void declarations()
            {
                Token::Type variableType;

                auto isDeclaration = [&]() -> bool
                {
                    variableType = m_currentType;
                    return m_currentType == Token::Type::INT
                        || m_currentType == Token::Type::STRING
                        || m_currentType == Token::Type::REAL;
                };

                while (isDeclaration())
                {
                    do
                    {
                        getToken(Token::Type::ID);
                        uint32_t varID = m_currentValue;

                        m_validator.declaration(varID, variableType);

                        getToken();

                        if (m_currentType == Token::Type::ASSIGN)
                        {
                            getToken();
                            constant();
                            m_validator.init(varID, m_currentType);
                            getToken();
                        }
                    }
                    while (m_currentType == Token::Type::COMMA);

                    checkToken(Token::Type::SEMICOLON);
                    getToken();
                }
            }

        public:

            Parser(const std::string& a_srcFileName)
                : m_scanner(a_srcFileName)
            {
            }

            void analyze()
            {
                getToken(Token::Type::ENTRY);
                getToken(Token::Type::BEGIN);
                {
                    getToken();
                    declarations();

                    m_validator.init();
                    m_poliz = std::vector<Token>(0);
                    statements();
                }
                getToken(Token::Type::FINISH);

                m_validator.outputTypeStack();
                std::cout << "Poliz stack:\n";
                int i = 0;
                for (auto& polizElem: m_poliz)
                {
                    std::cout << i++ << ":  ";
                    if (polizElem.getType() == Token::Type::ID)
                    {
                        std::cout << m_validator.fetchVariable(polizElem) << "\n";
                    }
                    else
                    {
                        std::cout << polizElem << "\n";
                    }
                }
            }
    };
}

#endif // PARSER_HPP

