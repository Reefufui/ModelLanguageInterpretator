#ifndef EXECUTER_HPP
#define EXECUTER_HPP

#include "Token.hpp"
#include "Parser.hpp"
#include <vector>
#include <stack>
#include <map>

namespace mli {

    class WriteOperation;

    class Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) = 0;

            Token popOperand(std::stack<Token>& a_operands)
            {
                Token operand = a_operands.top();
                a_operands.pop();
                return operand;
            }

            void idTokenToValueToken(Token& a_token)
            {
                Token::Type type{a_token.getType()};

                if (type == Token::Type::ID)
                {
                    Ident& variable = Semantic::findIdent(State::s_TID, a_token.getValue());
                    type = variable.getType();

                    if (variable.isAssigned())
                    {
                        a_token.setValue(variable.getValue());
                        if (type == Token::Type::INT)
                        {
                            a_token.setType(Token::Type::INT_CONST);
                        }
                        else if (type == Token::Type::STRING)
                        {
                            a_token.setType(Token::Type::STRING_CONST);
                        }
                        else if (type == Token::Type::REAL)
                        {
                            a_token.setType(Token::Type::REAL_CONST);
                        }
                    }
                    else
                    {
                        throw std::runtime_error("variable is not assigned");
                    }
                }
            }

            double numericToDouble(Token a_token)
            {
                if (a_token.getType() == Token::Type::REAL_CONST)
                {
                    return State::s_realNumbers[a_token.getValue()];
                }
                else
                {
                    return static_cast<double>(a_token.getValue());
                }
            }
    };

    class WriteOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token operand = popOperand(a_operands);
                idTokenToValueToken(operand);
                Token::Type operandType = operand.getType();

                if (operandType == Token::Type::STRING_CONST)
                {
                    std::cout << State::s_strings[operand.getValue()] << "\n";
                }
                else if (operandType == Token::Type::INT_CONST)
                {
                    std::cout << operand.getValue() << "\n";
                }
                else if (operandType == Token::Type::REAL_CONST)
                {
                    std::cout << State::s_realNumbers[operand.getValue()] << "\n";
                }
                else
                {
                    assert(false && "unsupported output operation");
                }

                return Token{};
            }
    };

    class ReadOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token operand = popOperand(a_operands);

                Ident& variable = Semantic::findIdent(State::s_TID, operand.getValue());
                Token::Type variableType = variable.getType();
                variable.setAssignment(true);

                if (variableType == Token::Type::STRING)
                {
                    std::string stringConst{};
                    std::cin >> stringConst;
                    variable.setValue(State::s_strings.size());
                    State::s_strings.push_back(stringConst);
                }
                else if (variableType == Token::Type::REAL)
                {
                    double doubleConst{};
                    std::cin >> doubleConst;
                    variable.setValue(State::s_realNumbers.size());
                    State::s_realNumbers.push_back(doubleConst);
                }
                else if (variableType == Token::Type::INT)
                {
                    int intConst{};
                    std::cin >> intConst;
                    variable.setValue(intConst);
                }
                else
                {
                    assert("unsupported input operation");
                }

                return Token{};
            }
    };

    class AssignOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token srcToken = popOperand(a_operands);
                idTokenToValueToken(srcToken);
                Token::Type srcType = srcToken.getType();

                Token dstToken = popOperand(a_operands);
                Ident& dstIdent = Semantic::findIdent(State::s_TID, dstToken.getValue());
                dstIdent.setAssignment(true);
                Token::Type dstType = dstIdent.getType();

                if (dstType == Token::Type::STRING)
                {
                    dstIdent.setValue(srcToken.getValue());
                }
                else if (dstType == Token::Type::REAL)
                {
                    if (srcType == Token::Type::REAL_CONST)
                    {
                        dstIdent.setValue(srcToken.getValue());
                    }
                    else
                    {
                        dstIdent.setValue(State::s_realNumbers.size());
                        State::s_realNumbers.push_back(static_cast<double>(srcToken.getValue()));
                    }
                }
                else if (dstType == Token::Type::INT)
                {
                    if (srcType == Token::Type::REAL_CONST)
                    {
                        double value = State::s_realNumbers[srcToken.getValue()];
                        dstIdent.setValue((int)value);
                    }
                    else
                    {
                        dstIdent.setValue(srcToken.getValue());
                    }
                }
                else
                {
                    assert(false && "unsupported input operation");
                }

                return dstToken;
            }
    };

    class FalseGoOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token label = popOperand(a_operands);
                Token expression = popOperand(a_operands);
                idTokenToValueToken(expression);

                if (expression.getValue())
                {
                    label.setType(Token::Type::NULL);
                }
                else
                {
                    label.setType(Token::Type::POLIZ_GO);
                }

                return label;
            }
    };

    class TrueGoOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token label = popOperand(a_operands);
                Token expression = popOperand(a_operands);
                idTokenToValueToken(expression);

                if (expression.getValue())
                {
                    label.setType(Token::Type::POLIZ_GO);
                }
                else
                {
                    label.setType(Token::Type::NULL);
                }

                return label;
            }
    };

    class FalseLazyOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token label = popOperand(a_operands);
                Token expression = a_operands.top();
                idTokenToValueToken(expression);

                if (expression.getValue())
                {
                    label.setType(Token::Type::NULL);
                }
                else
                {
                    label.setType(Token::Type::POLIZ_GO);
                }

                return label;
            }
    };

    class TrueLazyOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token label = popOperand(a_operands);
                Token expression = a_operands.top();
                idTokenToValueToken(expression);

                if (expression.getValue())
                {
                    label.setType(Token::Type::POLIZ_GO);
                }
                else
                {
                    label.setType(Token::Type::NULL);
                }

                return label;
            }
    };

    class GoOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token label = popOperand(a_operands);
                label.setType(Token::Type::POLIZ_GO);

                return label;
            }
    };

    class PopOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                popOperand(a_operands);

                return Token::Type::NULL;
            }
    };

    class SubtractOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                idTokenToValueToken(token1);
                Token token2 = popOperand(a_operands);
                idTokenToValueToken(token2);
                Token result{token1};

                if (token1.getType() == Token::Type::INT_CONST && token2.getType() == Token::Type::INT_CONST)
                {
                    result.setType(Token::Type::INT_CONST);
                    result.setValue(token2.getValue() - token1.getValue());
                }
                else
                {
                    result.setType(Token::Type::REAL_CONST);
                    double value1 = numericToDouble(token1);
                    double value2 = numericToDouble(token2);
                    result.setValue(State::s_realNumbers.size());
                    State::s_realNumbers.push_back(value2 - value1);
                }

                return result;
            }
    };

    class PlusOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                idTokenToValueToken(token1);
                Token token2 = popOperand(a_operands);
                idTokenToValueToken(token2);
                Token result{token1};

                if (token1.getType() == Token::Type::INT_CONST && token2.getType() == Token::Type::INT_CONST)
                {
                    result.setType(Token::Type::INT_CONST);
                    result.setValue(token2.getValue() + token1.getValue());
                }
                else if (token1.getType() == Token::Type::STRING_CONST && token2.getType() == Token::Type::STRING_CONST)
                {
                    result.setType(Token::Type::STRING_CONST);
                    std::string value1 = State::s_strings[token1.getValue()];
                    std::string value2 = State::s_strings[token2.getValue()];
                    result.setValue(State::s_strings.size());
                    State::s_strings.push_back(value2 + value1);
                }
                else
                {
                    result.setType(Token::Type::REAL_CONST);
                    double value1 = numericToDouble(token1);
                    double value2 = numericToDouble(token2);
                    result.setValue(State::s_realNumbers.size());
                    State::s_realNumbers.push_back(value2 + value1);
                }

                return result;
            }
    };

    class MultiplyOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                idTokenToValueToken(token1);
                Token token2 = popOperand(a_operands);
                idTokenToValueToken(token2);
                Token result{token1};

                if (token1.getType() == Token::Type::INT_CONST && token2.getType() == Token::Type::INT_CONST)
                {
                    result.setType(Token::Type::INT_CONST);
                    result.setValue(token2.getValue() * token1.getValue());
                }
                else
                {
                    result.setType(Token::Type::REAL_CONST);
                    double value1 = numericToDouble(token1);
                    double value2 = numericToDouble(token2);
                    result.setValue(State::s_realNumbers.size());
                    State::s_realNumbers.push_back(value2 * value1);
                }

                return result;
            }
    };

    class DivideOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                idTokenToValueToken(token1);
                Token token2 = popOperand(a_operands);
                idTokenToValueToken(token2);
                Token result{token1};

                if (token1.getType() == Token::Type::INT_CONST && token2.getType() == Token::Type::INT_CONST)
                {
                    result.setType(Token::Type::INT_CONST);
                    result.setValue(token2.getValue() / token1.getValue());
                }
                else
                {
                    result.setType(Token::Type::REAL_CONST);
                    double value1 = numericToDouble(token1);
                    double value2 = numericToDouble(token2);
                    result.setValue(State::s_realNumbers.size());
                    State::s_realNumbers.push_back(value2 / value1);
                }

                return result;
            }
    };

    class UnaryPlusOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token = popOperand(a_operands);
                idTokenToValueToken(token);

                return token;
            }
    };

    class UnaryMinusOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token = popOperand(a_operands);
                idTokenToValueToken(token);

                if (token.getType() == Token::Type::INT_CONST)
                {
                    token.setValue(-token.getValue());
                }
                else
                {
                    double value = numericToDouble(token);
                    token.setValue(State::s_realNumbers.size());
                    State::s_realNumbers.push_back(-value);
                }

                return token;
            }
    };

    class OperandWrapper : public Operation
    {
        private:
            std::string m_stringValue;
            double      m_numericValue;
            bool        m_isString;

        public:
            OperandWrapper(Token a_token)
            {
                idTokenToValueToken(a_token);

                if (a_token.getType() == Token::Type::STRING_CONST)
                {
                    m_stringValue = State::s_strings[a_token.getValue()];
                    m_isString = true;
                }
                else
                {
                    m_numericValue = numericToDouble(a_token);
                    m_isString = false;
                }
            }

            std::string getString() const
            {
                return m_stringValue;
            }

            double getNumeric() const
            {
                return m_numericValue;
            }

            int operator==(OperandWrapper&& other)
            {
                if (m_isString)
                {
                    return this->m_stringValue == other.getString();
                }
                else
                {
                    return this->m_numericValue == other.getNumeric();
                }
            }

            int operator!=(OperandWrapper&& other)
            {
                if (m_isString)
                {
                    return this->m_stringValue != other.getString();
                }
                else
                {
                    return this->m_numericValue != other.getNumeric();
                }
            }

            int operator>(OperandWrapper&& other)
            {
                if (m_isString)
                {
                    return this->m_stringValue > other.getString();
                }
                else
                {
                    return this->m_numericValue > other.getNumeric();
                }
            }

            int operator<(OperandWrapper&& other)
            {
                if (m_isString)
                {
                    return this->m_stringValue < other.getString();
                }
                else
                {
                    return this->m_numericValue < other.getNumeric();
                }
            }

            int operator>=(OperandWrapper&& other)
            {
                if (m_isString)
                {
                    return this->m_stringValue >= other.getString();
                }
                else
                {
                    return this->m_numericValue >= other.getNumeric();
                }
            }

            int operator<=(OperandWrapper&& other)
            {
                if (m_isString)
                {
                    return this->m_stringValue <= other.getString();
                }
                else
                {
                    return this->m_numericValue <= other.getNumeric();
                }
            }

            int operator||(OperandWrapper&& other)
            {
                return this->m_numericValue || other.getNumeric();
            }

            int operator&&(OperandWrapper&& other)
            {
                return this->m_numericValue && other.getNumeric();
            }

            virtual Token perform(std::stack<Token>& a_operands) override
            {
                assert(false && "unaviable option");
            }
    };

    class EqualOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                Token token2 = popOperand(a_operands);
                Token result{token1};
                result.setType(Token::Type::INT_CONST);
                result.setValue(OperandWrapper(token2) == OperandWrapper(token1));

                return result;
            }
    };

    class LessOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                Token token2 = popOperand(a_operands);
                Token result{token1};
                result.setType(Token::Type::INT_CONST);
                result.setValue(OperandWrapper(token2) < OperandWrapper(token1));

                return result;
            }
    };

    class GreaterOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                Token token2 = popOperand(a_operands);
                Token result{token1};
                result.setType(Token::Type::INT_CONST);
                result.setValue(OperandWrapper(token2) > OperandWrapper(token1));

                return result;
            }
    };

    class NotEqualOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                Token token2 = popOperand(a_operands);
                Token result{token1};
                result.setType(Token::Type::INT_CONST);
                result.setValue(OperandWrapper(token2) != OperandWrapper(token1));

                return result;
            }
    };

    class GreaterEqualOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                Token token2 = popOperand(a_operands);
                Token result{token1};
                result.setType(Token::Type::INT_CONST);
                result.setValue(OperandWrapper(token2) >= OperandWrapper(token1));

                return result;
            }
    };

    class LessEqualOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                Token token2 = popOperand(a_operands);
                Token result{token1};
                result.setType(Token::Type::INT_CONST);
                result.setValue(OperandWrapper(token2) <= OperandWrapper(token1));

                return result;
            }
    };

    class AndOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                Token token2 = popOperand(a_operands);
                Token result{token1};
                result.setType(Token::Type::INT_CONST);
                result.setValue(OperandWrapper(token2) && OperandWrapper(token1));

                return result;
            }
    };

    class OrOperation : public Operation
    {
        public:
            virtual Token perform(std::stack<Token>& a_operands) override
            {
                Token token1 = popOperand(a_operands);
                Token token2 = popOperand(a_operands);
                Token result{token1};
                result.setType(Token::Type::INT_CONST);
                result.setValue(OperandWrapper(token2) || OperandWrapper(token1));

                return result;
            }
    };

    class Executer
    {
        private:
            WriteOperation     writeOperation;
            ReadOperation      readOperation;
            AssignOperation    assignOperation;
            FalseGoOperation   falseGoOperation;
            TrueGoOperation    trueGoOperation;
            FalseLazyOperation falseLazyOperation;
            TrueLazyOperation  trueLazyOperation;
            GoOperation        goOperation;
            PopOperation       popOperation;
            SubtractOperation  subtractOperation;
            PlusOperation      plusOperation;
            MultiplyOperation  multiplyOperation;
            DivideOperation    divideOperation;

            UnaryMinusOperation unaryMinusOperation;
            UnaryPlusOperation  unaryPlusOperation;

            EqualOperation        equalOperation;
            LessOperation         lessOperation;
            GreaterOperation      greaterOperation;
            NotEqualOperation     notEqualOperation;
            LessEqualOperation    lessEqualOperation;
            GreaterEqualOperation greaterEqualOperation;
            OrOperation           orOperation;
            AndOperation          andOperation;

            std::map<Token::Type, Operation*> operations
            {
                { Token::Type::WRITE,                &writeOperation },
                    { Token::Type::READ,             &readOperation },
                    { Token::Type::ASSIGN,           &assignOperation },
                    { Token::Type::POLIZ_FALSE_GO,   &falseGoOperation },
                    { Token::Type::POLIZ_TRUE_GO,    &trueGoOperation },
                    { Token::Type::POLIZ_FALSE_LAZY, &falseLazyOperation },
                    { Token::Type::POLIZ_TRUE_LAZY,  &trueLazyOperation },
                    { Token::Type::POLIZ_GO,         &goOperation },
                    { Token::Type::SEMICOLON,        &popOperation },
                    { Token::Type::MINUS,            &subtractOperation },
                    { Token::Type::PLUS,             &plusOperation },
                    { Token::Type::MULTIPLY,         &multiplyOperation },
                    { Token::Type::DIVIDE,           &divideOperation },
                    { Token::Type::UNARY_PLUS,       &unaryPlusOperation },
                    { Token::Type::UNARY_MINUS,      &unaryMinusOperation },
                    { Token::Type::EQ,               &equalOperation },
                    { Token::Type::LESS,             &lessOperation },
                    { Token::Type::GREATER,          &greaterOperation },
                    { Token::Type::NEQ,              &notEqualOperation },
                    { Token::Type::LEQ,              &lessEqualOperation },
                    { Token::Type::GEQ,              &greaterEqualOperation },
                    { Token::Type::OR,               &orOperation },
                    { Token::Type::AND,              &andOperation }
            };

        public:

            void executePoliz(std::vector<Token>& a_poliz)
            {
                std::stack<Token> operands;

                int polizIndex = 0;
                const int polizSize = a_poliz.size();

                while (polizIndex < polizSize)
                {
                    const Token       currentToken = a_poliz[polizIndex];
                    const Token::Type currentType  = currentToken.getType();

                    Operation* operation = operations[currentType];

                    if (!operation)
                    {
                        operands.push(currentToken);
                    }
                    else
                    {
                        Token result = operation->perform(operands);

                        if (result.getType() == Token::Type::POLIZ_GO)
                        {
                            polizIndex = result.getValue() - 1;
                        }
                        else if (result.getType() != Token::Type::NULL)
                        {
                            operands.push(result);
                        }
                    }

                    ++polizIndex;
                }
            }

    };
}

#endif // EXECUTER_HPP

