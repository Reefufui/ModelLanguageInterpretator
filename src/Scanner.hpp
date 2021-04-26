#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include "Token.hpp"
#include "Ident.hpp"
#include "LexicalError.hpp"

namespace fs = std::filesystem;

namespace mli {

    class InitialState;
    class IdentState;
    class NumberState;
    class StringState;
    class CommentState;
    class LessGreaterState;
    class NotEqualState;
    class AssignOrEqual;
    class RealState;

    class State
    {
        public:

            struct Machine
            {
                InitialState*     pInitialState{};
                IdentState*       pIdentState{};
                NumberState*      pNumberState{};
                StringState*      pStringState{};
                CommentState*     pCommentState{};
                LessGreaterState* pLessGreaterState{};
                NotEqualState*    pNotEqualState{};
                AssignOrEqual*    pAssignOrEqual{};
                RealState*        pRealState{};
            };

            virtual State* determineToken() = 0;

            void setSrcFile(std::ifstream* a_pSrcFile)
            {
                s_charBuffer = "";
                s_numBuffer  = 0;
                s_pSrcFile   = a_pSrcFile;
            }

            void setStateMachine(Machine a_stateMachine)
            {
                s_stateMachine = a_stateMachine;
            }

            Token getToken() const
            {
                return s_token;
            }

            static std::unordered_map<std::string, Ident> s_TID;
            static std::unordered_map<std::string, Ident> s_gotoMarks;
            static std::vector<std::string>               s_strings;
            static std::vector<double>                    s_realNumbers;

        protected:
            static std::string    s_charBuffer;
            static uint32_t       s_numBuffer;
            static std::ifstream* s_pSrcFile;
            static Machine        s_stateMachine;

            static Token s_token;
            static int   s_currentLine;

            char m_currentChar;

            char getChar()
            {
                *s_pSrcFile >> std::noskipws >> m_currentChar;
                return m_currentChar;
            }
    };

    std::string    State::s_charBuffer{};
    uint32_t       State::s_numBuffer{};
    std::ifstream* State::s_pSrcFile{nullptr};
    State::Machine State::s_stateMachine{};
    Token          State::s_token{};
    int            State::s_currentLine{1};

    std::unordered_map<std::string, Ident> State::s_TID;
    std::unordered_map<std::string, Ident> State::s_gotoMarks;
    std::vector<std::string> State::s_strings;
    std::vector<double> State::s_realNumbers;

    class InitialState : public State
    {
        public:

            State* determineToken()
            {
                getChar();

                s_charBuffer = std::string("");
                s_numBuffer  = uint32_t(0);
                s_token      = Token::Type::NULL;

                if (s_pSrcFile->eof())
                {
                    s_token = Token(Token::Type::FINISH, s_currentLine);
                }
                else if (std::isspace(m_currentChar))
                {
                    s_currentLine += (m_currentChar == '\n');
                }
                else if (std::isalpha(m_currentChar))
                {
                    s_charBuffer.push_back(m_currentChar);
                    return (State*)s_stateMachine.pIdentState;
                }
                else if (std::isdigit(m_currentChar))
                {
                    s_numBuffer = m_currentChar - '0';
                    return (State*)s_stateMachine.pNumberState;
                }
                else if (m_currentChar == '"')
                {
                    return (State*)s_stateMachine.pStringState;
                }
                else if (m_currentChar == '/')
                {
                    s_charBuffer.push_back(m_currentChar);
                    return (State*)s_stateMachine.pCommentState;
                }
                else if (m_currentChar == '<' || m_currentChar == '>')
                {
                    s_charBuffer.push_back(m_currentChar);
                    return (State*)s_stateMachine.pLessGreaterState;
                }
                else if (m_currentChar == '!')
                {
                    s_charBuffer.push_back(m_currentChar);
                    return (State*)s_stateMachine.pNotEqualState;
                }
                else if (m_currentChar == '=')
                {
                    s_charBuffer.push_back(m_currentChar);
                    return (State*)s_stateMachine.pAssignOrEqual;
                }
                else
                {
                    s_charBuffer.push_back(m_currentChar);
                    if (Token::Token::s_delimeters.contains(s_charBuffer))
                    {
                        s_token = Token(Token::s_delimeters[s_charBuffer], s_currentLine);
                    }
                    else
                    {
                        throw LexicalError(s_currentLine, m_currentChar);
                    }
                }

                return this;
            }
    };

    class IdentState : public State
    {
        public:

            State* determineToken() override
            {
                getChar();

                if (std::isalpha(m_currentChar) || std::isdigit(m_currentChar))
                {
                    s_charBuffer.push_back(m_currentChar);
                }
                else
                {
                    s_pSrcFile->unget();

                    if (Token::s_reservedWords.contains(s_charBuffer))
                    {
                        s_token = Token(Token::s_reservedWords[s_charBuffer], s_currentLine);
                    }
                    else if (s_gotoMarks.contains(s_charBuffer))
                    {
                        s_token = Token(Token::Type::GOTO_MARK, s_currentLine, s_gotoMarks[s_charBuffer].getID());
                    }
                    else if (s_TID.contains(s_charBuffer))
                    {
                        s_token = Token(Token::Type::ID, s_currentLine, s_TID[s_charBuffer].getID());
                    }
                    else
                    {
                        if (m_currentChar != ':')
                        {
                            s_TID[s_charBuffer] = Ident(s_charBuffer);
                            s_token = Token(Token::Type::ID, s_currentLine, s_TID[s_charBuffer].getID());
                        }
                        else
                        {
                            s_gotoMarks[s_charBuffer] = Ident(s_charBuffer);
                            s_token = Token(Token::Type::GOTO_MARK, s_currentLine, s_gotoMarks[s_charBuffer].getID());
                        }
                    }

                    return (State*)s_stateMachine.pInitialState;
                }

                return this;
            }
    };

    class NumberState : public State
    {
        public:

            State* determineToken()
            {
                getChar();

                if (std::isdigit(m_currentChar))
                {
                    s_numBuffer *= 10;
                    s_numBuffer += m_currentChar - '0';
                }
                else
                {
                    if (m_currentChar == '.')
                    {
                        return (State*)s_stateMachine.pRealState;
                    }
                    else if (std::isalpha(m_currentChar))
                    {
                        throw LexicalError(s_currentLine, m_currentChar);
                    }
                    else
                    {
                        s_token = Token(Token::Type::INT_CONST, s_currentLine, s_numBuffer);
                    }

                    s_pSrcFile->unget();
                    return (State*)s_stateMachine.pInitialState;
                }

                return this;
            }
    };

    class StringState : public State
    {
        public:

            State* determineToken()
            {
                getChar();

                if (m_currentChar == '"')
                {
                    s_strings.push_back(s_charBuffer);
                    s_token = Token(Token::Type::STRING_CONST, s_currentLine, s_strings.size() - 1);
                    return (State*)s_stateMachine.pInitialState;
                }
                else
                {
                    s_charBuffer.push_back(m_currentChar);
                }

                return this;
            }
    };

    class CommentState : public State
    {
        public:

            State* determineToken() override
            {
                if (s_pSrcFile->eof())
                {
                    throw LexicalError(s_currentLine, 0);
                }

                getChar();

                if (s_charBuffer == "/")
                {
                    if (m_currentChar == '*')
                    {
                        s_currentLine += (m_currentChar == '\n');
                        s_charBuffer = "";
                    }
                    else
                    {
                        s_pSrcFile->unget();
                        s_token = Token(Token::s_delimeters[s_charBuffer], s_currentLine);
                        return (State*)s_stateMachine.pInitialState;
                    }

                }
                else if (m_currentChar == '*')
                {
                    getChar();
                    if (m_currentChar == '/')
                    {
                        return (State*)s_stateMachine.pInitialState;
                    }
                }

                s_currentLine += (m_currentChar == '\n');

                return this;
            }
    };

    class LessGreaterState : public State
    {
        public:

            State* determineToken() override
            {
                getChar();

                if (m_currentChar == '=')
                {
                    s_charBuffer.push_back(m_currentChar);
                    s_token = Token(Token::s_delimeters[s_charBuffer], s_currentLine);
                }
                else
                {
                    s_pSrcFile->unget();
                    s_token = Token(Token::s_delimeters[s_charBuffer], s_currentLine);
                }

                return (State*)s_stateMachine.pInitialState;
            }
    };

    class NotEqualState : public State
    {
        public:

            State* determineToken() override
            {
                getChar();

                if (m_currentChar == '=')
                {
                    s_charBuffer.push_back(m_currentChar);
                    s_token = Token(Token::s_delimeters[s_charBuffer], s_currentLine);
                }
                else
                {
                    std::runtime_error("[Scanner::getToken]: unexpected '!' character");
                }

                return (State*)s_stateMachine.pInitialState;
            }
    };

    class AssignOrEqual : public State
    {
        public:

            State* determineToken() override
            {
                getChar();

                if (m_currentChar == '=')
                {
                    s_charBuffer.push_back(m_currentChar);
                    s_token = Token(Token::s_delimeters[s_charBuffer], s_currentLine);
                }
                else
                {
                    s_pSrcFile->unget();
                    s_token = Token(Token::s_delimeters[s_charBuffer], s_currentLine);
                }

                return (State*)s_stateMachine.pInitialState;
            }
    };

    class RealState : public State
    {
        public:

            State* determineToken() override
            {
                getChar();

                if (std::isdigit(m_currentChar))
                {
                    s_numBuffer *= 10;
                    s_numBuffer += m_currentChar - '0';
                    s_charBuffer.push_back(m_currentChar);
                }
                else
                {
                    double real = (double)s_numBuffer * std::pow(0.1, s_charBuffer.size());
                    s_realNumbers.push_back(real);
                    s_token = Token(Token::Type::REAL_CONST, s_currentLine, s_realNumbers.size() - 1);

                    s_pSrcFile->unget();
                    return (State*)s_stateMachine.pInitialState;
                }

                return this;
            }
    };

    class Scanner
    {
        private:
            std::ifstream  m_srcFile;
            State::Machine m_stateMachine;

            InitialState     m_initialState{};
            IdentState       m_identState{};
            NumberState      m_numberState{};
            StringState      m_stringState{};
            CommentState     m_commentState{};
            LessGreaterState m_lessGreaterState{};
            NotEqualState    m_notEqualState{};
            AssignOrEqual    m_assignOrEqualState{};
            RealState        m_realState{};

            State* m_currentState{};

        public:

            Scanner(const std::string& a_srcFileName)
                : m_srcFile(std::ifstream(a_srcFileName))
            {
                if (!fs::exists(fs::path(a_srcFileName)))
                {
                    throw std::runtime_error("[Scanner]: source file doesnt exist");
                }

                m_stateMachine = State::Machine{
                    &m_initialState,
                        &m_identState,
                        &m_numberState,
                        &m_stringState,
                        &m_commentState,
                        &m_lessGreaterState,
                        &m_notEqualState,
                        &m_assignOrEqualState,
                        &m_realState
                };

                m_currentState = &m_initialState;
                m_currentState->setSrcFile(&m_srcFile);
                m_currentState->setStateMachine(m_stateMachine);
            }

            ~Scanner()
            {
                if (m_srcFile.is_open())
                {
                    m_srcFile.close();
                }
            }

            Token getToken()
            {
                Token unknownToken{};

                do
                {
                    m_currentState = m_currentState->determineToken();
                }
                while (m_currentState->getToken() == unknownToken);

                return m_currentState->getToken();
            }
    };
}

#endif // SCANNER_HPP
