#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include <vector>
#include <stack>
#include <cassert>
#include <sstream>

#include "Token.hpp"
#include "Scanner.hpp"
#include "SemanticError.hpp"

namespace mli {

    class Semantic
    {
        private:
            std::vector<Ident> m_declaredVariables;
            std::vector<Ident> m_gotoMarks;

            std::stack<Token::Type> m_typesStack;
            bool                    m_rValueFlag;

            Ident& findIdent(std::unordered_map<std::string, Ident>& a_map, int a_id)
            {
                auto mapCheck = [a_id](const std::pair<std::string, Ident>& elem) -> bool
                {
                    return elem.second.getID() == a_id;
                };

                auto mapPair = std::find_if(a_map.begin(), a_map.end(), mapCheck);

                assert(mapPair != a_map.end());

                return a_map[(*mapPair).first];
            }

        public:

            void pushType(Token::Type a_type)
            {
                m_typesStack.push(a_type);
            }

            void popWithOperator(Token::Type a_type)
            {
                bool isUnary = (a_type == Token::Type::NOT)
                    || (a_type == Token::Type::UNARY_MINUS)
                    || (a_type == Token::Type::UNARY_PLUS);

                if (isUnary)
                {
                    if (m_typesStack.top() != Token::Type::INT_CONST)
                    {
                        throw SemanticError(State::s_currentLine, a_type, "for non-int operand");
                    }
                    m_rValueFlag = true;

                    return;
                }

                Token::Type rightOperand = m_typesStack.top();
                m_typesStack.pop();

                Token::Type leftOperand = m_typesStack.top();
                m_typesStack.pop();

                if (a_type == Token::Type::MULTIPLY
                        || a_type == Token::Type::DIVIDE
                        || a_type == Token::Type::MINUS
                        || a_type == Token::Type::PLUS)
                {
                    m_rValueFlag = true;

                    if (rightOperand == Token::Type::STRING_CONST || leftOperand == Token::Type::STRING_CONST)
                    {
                        if (a_type == Token::Type::PLUS)
                        {
                            if (leftOperand != rightOperand)
                            {
                                throw SemanticError(State::s_currentLine, a_type, "for numerical and string");
                            }
                            else
                            {
                                std::cout << leftOperand << " + " << rightOperand << "\n";
                                m_typesStack.push(Token::Type::STRING_CONST);
                            }
                        }
                        else
                        {
                            throw SemanticError(State::s_currentLine, a_type, "for string operand");
                        }
                    }
                    else if (rightOperand == Token::Type::INT_CONST && leftOperand == Token::Type::INT_CONST)
                    {
                        m_typesStack.push(Token::Type::INT_CONST);
                    }
                    else
                    {
                        m_typesStack.push(Token::Type::REAL_CONST);
                    }
                }
                else if (a_type >= Token::Type::EQ && a_type <= Token::Type::GEQ)
                {
                    m_rValueFlag = true;

                    if (rightOperand == Token::Type::STRING_CONST || leftOperand == Token::Type::STRING_CONST)
                    {
                        if (rightOperand != leftOperand)
                        {
                            throw SemanticError(State::s_currentLine, a_type, "got unexpected string as second operand");
                        }
                    }
                    m_typesStack.push(Token::Type::INT_CONST);
                }
                else if (a_type == Token::Type::AND || a_type == Token::Type::OR)
                {
                    m_rValueFlag = true;

                    if (leftOperand == rightOperand && leftOperand == Token::Type::INT_CONST)
                    {
                        m_typesStack.push(Token::Type::INT_CONST);
                    }
                    else
                    {
                        std::cout << leftOperand << " or " << rightOperand << "\n";
                        throw SemanticError(State::s_currentLine, a_type, "for non-int operand");
                    }
                }
                else if (a_type == Token::Type::ASSIGN)
                {
                    if (leftOperand == Token::Type::STRING_CONST || rightOperand == Token::Type::STRING_CONST)
                    {
                        if (leftOperand != rightOperand)
                        {
                            throw SemanticError(State::s_currentLine, a_type, "type mismatch");
                        }
                    }
                    m_typesStack.push(leftOperand);
                }
            }

            void isLValue()
            {
                if (m_rValueFlag)
                {
                    throw SemanticError(State::s_currentLine, Token::Type::ASSIGN, "tried to assign to rValue");
                }
                m_rValueFlag = false;
            }

            void setRValueFlag(bool a_flag)
            {
                m_rValueFlag = a_flag;
            }

            void init()
            {
                m_typesStack = std::stack<Token::Type>{};
            }

            void outputTypeStack()
            {
                while (!m_typesStack.empty())
                {
                    std::cout << m_typesStack.top() << "\n";
                    m_typesStack.pop();
                }
            }

            Ident& fetchVariable(const Token& a_token)
            {
                return m_declaredVariables[a_token.getValue()];
            }

            void declarationCheck(uint32_t a_variableID)
            {
                if (a_variableID >= m_declaredVariables.size())
                {
                    throw SemanticError(State::s_currentLine, findIdent(State::s_TID, a_variableID), "not declared");
                }
            }

            void typeCheck(uint32_t a_variableID, Token::Type a_type)
            {
                Token::Type srcType = (a_type == Token::Type::STRING_CONST) ? Token::Type::STRING
                    : (a_type == Token::Type::INT_CONST)  ? Token::Type::INT
                    : (a_type == Token::Type::REAL_CONST) ? Token::Type::REAL : Token::Type::NULL;

                Token::Type dstType = m_declaredVariables[a_variableID].getType();

                if (srcType != dstType)
                {
                    std::stringstream msg{};
                    msg << "tried to assign " << a_type << " to " << m_declaredVariables[a_variableID].getType();
                    throw SemanticError(State::s_currentLine, m_declaredVariables[a_variableID], msg.str());
                }
            }

            void declaration(uint32_t a_variableID, Token::Type a_type)
            {
                auto& variable = findIdent(State::s_TID, a_variableID);

                m_declaredVariables.push_back(variable);

                if (variable.isDeclared())
                {
                    throw SemanticError(State::s_currentLine, variable, "declared twice");
                }

                m_declaredVariables[a_variableID].setDeclaration(true);
                m_declaredVariables[a_variableID].setType(a_type);
            }

            void mark(uint32_t a_markID)
            {
                auto& gotoMark = findIdent(State::s_gotoMarks, a_markID);

                if (gotoMark.isDeclared())
                {
                    throw SemanticError(State::s_currentLine, gotoMark, "found twice");
                }
                else
                {
                    gotoMark.setDeclaration(true);
                }

                gotoMark.setType(Token::Type::GOTO_MARK);
                m_gotoMarks.push_back(gotoMark);
            }

            void init(uint32_t a_variableID, Token::Type a_type)
            {
                declarationCheck(a_variableID);
                typeCheck(a_variableID, a_type);
                auto& variable = m_declaredVariables[a_variableID];

                variable.setAssignment(true);
            }
    };
}

#endif // SEMANTIC_HPP

