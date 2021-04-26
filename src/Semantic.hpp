#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include <vector>
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
            // TODO: from_st analog:
            // push x in dst, pop x in src

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
                else
                {
                    m_declaredVariables[a_variableID].setDeclaration(true);
                }

                m_declaredVariables[a_variableID].setType(a_type);
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

