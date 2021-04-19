#include <iostream>

#include <map>
#include <unordered_map>
#include <string>

#include <algorithm>
#include <fstream>
#include <filesystem>

#undef NULL
namespace fs = std::filesystem;

namespace mli {

    enum class TokenType {
        NULL = 0,
        ENTRY, FINISH,
        BEGIN, END,
        INT, STRING, REAL,
        VARIABLE,
        GOTO,
        CASE_OF,
        WHILE, DO,
        READ, WRITE,
        NOT, AND, OR,

        SEMICOLON, COLON, COMMA, POINT,
        ASSIGN,
        OPEN_B, CLOSE_B,
        EQ, LESS, GREATER, NEQ, LEQ, GEQ,

        PLUS, MINUS, MULTIPLY, DIVIDE,

        ID,
        POLIZ_LABEL,
        POLIZ_ADDRESS, 
        POLIZ_GO,
        POLIZ_FGO
    };

    class Ident {
        private:
            std::string m_name;
            TokenType   m_type;

            uint32_t    m_value;

            bool        m_assign;
            bool        m_declare;

            static int  m_identCount;
            int         m_id;

        public:

            Ident()
                : m_assign(false), m_declare(false)
            {
            }

            Ident(const char* a_name)
                : m_assign(false), m_declare(false)
            {
                m_name = a_name;
                m_id = m_identCount;
                ++m_identCount;
            }

            Ident(const std::string& a_name)
                : m_name(a_name), m_assign(false), m_declare(false)
            {
                m_id = m_identCount;
                ++m_identCount;
            }

            Ident(std::string&& a_name)
                : m_name(a_name), m_assign(false), m_declare(false)
            {
                m_id = m_identCount;
                ++m_identCount;
            }

            bool operator==(const std::string& a_str)
            {
                return m_name == a_str;
            }
            
            int getID() const
            {
                return m_id;
            }

            std::string getName() const
            {
                return m_name;
            }

            TokenType getType() const
            {
                return m_type;
            }

            uint32_t getValue() const
            {
                return m_value;
            }

            bool isDeclared() const
            {
                return m_declare;
            }

            bool isAssigned() const
            {
                return m_assign;
            }

            void setName(const std::string& a_name)
            {
                m_name = a_name;
            }

            void setType(const TokenType& a_type)
            {
                m_type = a_type;
            }

            void setValue(const uint32_t& a_value)
            {
                m_value = a_value;
            }

            void setDeclaration(const bool& a_declare)
            {
                m_declare = a_declare;
            }

            void setAssignment(const bool& a_assign)
            {
                m_assign = a_assign;
            }
    };

    int Ident::m_identCount{};

    class Token
    {
        private:
            TokenType m_type;
            uint32_t  m_value;

        public:

            Token(TokenType a_type = TokenType::NULL, uint32_t a_value = 0)
                : m_type(a_type), m_value(a_value)
            {
            }

            TokenType getType() const
            {
                return m_type;
            }

            uint32_t getValue() const
            {
                return m_value;
            }

            friend std::ostream& operator<<(std::ostream &a_out, const Token& a_token)
            {
                a_out << "(" << (int)a_token.m_type << ", " << a_token.m_value << ");";

                return a_out;
            }
    };

    class Scanner
    {
        private:
            static std::map<std::string, TokenType> s_reservedWords;
            static std::map<std::string, TokenType> s_delimeters;
            static std::unordered_map<std::string, Ident> s_TID;

            std::fstream m_srcFile;
            char m_currentChar;

            char getChar()
            {
                m_srcFile >> std::noskipws >> m_currentChar;
                return m_currentChar;
            }

        public:

            Scanner(const std::string& a_srcFileName)
                : m_srcFile(std::fstream(a_srcFileName))
            {
                if (!fs::exists(fs::path(a_srcFileName)))
                {
                    throw std::runtime_error("[Scanner]: source file doesnt exist");
                }
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
                // TODO: abstract class State
                enum class State {
                    INITIAL,
                    IDENT,
                    NUMBER,
                    COMMENT,
                    LESS_GREATER,
                    NEQ,
                    REAL
                } currentState{ State::INITIAL };

                std::string charBuffer{};
                uint32_t    numBuffer{};
                static int  currentLine{1};

                for (;;)
                {
                    getChar();

                    switch (currentState)
                    {
                        case State::INITIAL:
                            charBuffer = std::string("");
                            numBuffer  = uint32_t(0);

                            if (m_srcFile.eof())
                            {
                                return Token(TokenType::FINISH);
                            }
                            else if (std::isspace(m_currentChar))
                            {
                                currentLine += (m_currentChar == '\n');
                            }
                            else if (std::isalpha(m_currentChar))
                            {
                                charBuffer.push_back(m_currentChar);
                                currentState = State::IDENT; 
                            }
                            else if (std::isdigit(m_currentChar))
                            {
                                numBuffer = m_currentChar - '0';
                                currentState = State::NUMBER;
                            }
                            else if (m_currentChar == '#')
                            {
                                currentState = State::COMMENT;
                            }
                            else if (m_currentChar == '<' || m_currentChar == '>')
                            {
                                charBuffer.push_back(m_currentChar);
                                currentState = State::LESS_GREATER;
                            }
                            else if (m_currentChar == '!')
                            {
                                charBuffer.push_back(m_currentChar);
                                currentState = State::NEQ;
                            }
                            else if (m_currentChar == '.')
                            {
                                charBuffer.push_back(m_currentChar);
                                currentState = State::REAL;
                            }
                            else
                            {
                                charBuffer.push_back(m_currentChar);
                                if (s_delimeters.contains(charBuffer))
                                {
                                    return Token(s_delimeters[charBuffer]);
                                }
                                else
                                {
                                    std::string msg{ "[Scanner::getToken]: unexpected symbol '" };
                                    msg.push_back(m_currentChar);
                                    msg += "' on line: ";
                                    msg += std::to_string(currentLine);
                                    throw std::runtime_error(msg);
                                }
                            }
                            break;

                        case State::IDENT:

                            if (std::isalpha(m_currentChar) || std::isdigit(m_currentChar))
                            {
                                charBuffer.push_back(m_currentChar);
                            }
                            else
                            {
                                m_srcFile.unget();

                                if (s_reservedWords.contains(charBuffer))
                                {
                                    return Token(s_reservedWords[charBuffer]);
                                }
                                else
                                {
                                    if (s_TID.contains(charBuffer))
                                    {
                                        return Token(TokenType::ID, s_TID[charBuffer].getID());
                                    }
                                    else
                                    {
                                        s_TID[charBuffer] = Ident(charBuffer);
                                        return Token(TokenType::ID, s_TID[charBuffer].getID());
                                    }
                                }
                            }
                            break;

                        case State::NUMBER:
                            if (std::isdigit(m_currentChar))
                            {
                                numBuffer *= 10;
                                numBuffer += m_currentChar - '0';
                            }
                            else
                            {
                                if (m_currentChar == '.')
                                {
                                    // REAL : REAL POINT INT
                                    return Token(TokenType::REAL, numBuffer);
                                }
                                else
                                {
                                    return Token(TokenType::INT, numBuffer);
                                }
                                m_srcFile.unget();
                            }
                            break;

                        case State::REAL:
                            if (std::isdigit(m_currentChar))
                            {
                                ++numBuffer;
                            }
                            else
                            {
                                for (uint32_t i{}; i <= numBuffer; ++i) m_srcFile.unget();

                                return Token(TokenType::POINT, numBuffer);
                            }
                            break;

                        case State::COMMENT:
                            if (m_currentChar == '#')
                            {
                                currentState = State::INITIAL;
                            }
                            else if (m_srcFile.eof())
                            {
                                std::string msg{ "[Scanner::getToken]: unexpected eof" };
                                throw std::runtime_error(msg);
                            }
                            else
                            {
                                currentLine += (m_currentChar == '\n');
                            }
                            break;

                        case State::LESS_GREATER:
                            if (m_currentChar == '=')
                            {
                                charBuffer.push_back(m_currentChar);
                                return Token(s_delimeters[charBuffer]);
                            }
                            else
                            {
                                m_srcFile.unget();
                                return Token(s_delimeters[charBuffer]);
                            }
                            break;

                        case State::NEQ:
                            if (m_currentChar == '=')
                            {
                                charBuffer.push_back(m_currentChar);
                                return Token(s_delimeters[charBuffer]);
                            }
                            else
                            {
                                std::runtime_error("[Scanner::getToken]: unexpected '!' character");
                            }
                            break;
                    }
                }

                return TokenType::NULL;
            }
    };

    class Parser
    {
        private:
            Token     m_currentToken;
            TokenType m_currentType;
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
                if ( m_currentType == TokenType::ID )
                {
                    getToken();
                }
                else if ( m_currentType == TokenType::STRING
                        || m_currentType == TokenType::INT
                        || m_currentType == TokenType::REAL)
                {
                    getToken();
                }
                else if ( m_currentType == TokenType::POINT )
                {
                    getToken(); 
                }
                else if ( m_currentType == TokenType::NOT )
                {
                    getToken(); 
                    value(); 
                }
                else if ( m_currentType == TokenType::OPEN_B )
                {
                    getToken(); 
                    expression();

                    if ( m_currentType != TokenType::CLOSE_B)
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

                while ( m_currentType == TokenType::MULTIPLY
                        || m_currentType == TokenType::DIVIDE
                        || m_currentType == TokenType::AND)
                {
                    getToken();
                    value();
                }
            }

            void subexpression()
            {
                timesLikeOperation();

                while ( m_currentType == TokenType::PLUS
                        || m_currentType == TokenType::MINUS
                        || m_currentType == TokenType::OR)
                {
                    getToken();
                    timesLikeOperation();
                }
            }

            void expression()
            {
                subexpression();

                if ( m_currentType == TokenType::EQ
                        || m_currentType == TokenType::NEQ
                        || m_currentType == TokenType::LESS
                        || m_currentType == TokenType::GREATER
                        || m_currentType == TokenType::LEQ
                        || m_currentType == TokenType::GEQ)
                {
                    getToken();
                    subexpression();
                }
            }

            void sentence()
            {
                if ( m_currentType == TokenType::WHILE )
                {
                    getToken();
                    expression();

                    if ( m_currentType != TokenType::DO )
                    {
                        throw std::runtime_error("[Parser::sentence]: do expected after while");
                    }

                    getToken();
                    sentence();
                }
                if ( m_currentType == TokenType::DO )
                {
                    getToken();
                    sentence();

                    if ( m_currentType != TokenType::WHILE )
                    {
                        throw std::runtime_error("[Parser::sentence]: while expected after do");
                    }

                    getToken();
                    expression();
                }
                else if ( m_currentType == TokenType::READ )
                {
                    getToken();

                    if ( m_currentType != TokenType::OPEN_B )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected '('");
                    }

                    getToken();

                    if ( m_currentType != TokenType::ID )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected variable name");
                    }

                    getToken();

                    if ( m_currentType != TokenType::CLOSE_B )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected ')'");
                    }

                    getToken();
                }
                else if ( m_currentType == TokenType::WRITE )
                {
                    getToken();

                    if ( m_currentType != TokenType::OPEN_B )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected '('");
                    }

                    getToken();
                    expression();

                    if ( m_currentType != TokenType::CLOSE_B )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected ')'");
                    }

                    getToken();
                }
                else if ( m_currentType == TokenType::GOTO )
                { 
                    getToken();

                    if ( m_currentType != TokenType::ID )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected identificator");
                    }

                    getToken();

                    if ( m_currentType != TokenType::COLON )
                    {
                        throw std::runtime_error("[Parser::sentence]: expected ':'");
                    }
                }
                else if ( m_currentType == TokenType::ID )
                { 
                    getToken();

                    if ( m_currentType != TokenType::ASSIGN )
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
                if (m_currentType != TokenType::BEGIN)
                {
                    throw std::runtime_error("[Parser::block]: '{' expected");
                }

                getToken();
                sentence();

                while (m_currentType == TokenType::SEMICOLON)
                {
                    getToken();
                    sentence();
                }

                if (m_currentType != TokenType::END)
                {
                    throw std::runtime_error("[Parser::block]: '}' expected");
                }

                getToken();
            }

            void variable()
            {
                if (m_currentType != TokenType::ID)
                {
                    throw std::runtime_error("[Parser::variable]: variable name expected");
                }

                getToken();

                while (m_currentType == TokenType::COMMA)
                {
                    getToken();

                    if (m_currentType != TokenType::ID)
                    {
                        throw std::runtime_error("[Parser::variable]: variable name expected");
                    }

                    getToken();
                }

                if (m_currentType != TokenType::COLON)
                {
                    throw std::runtime_error("[Parser::variable]: ':' expected");
                }

                getToken();

                bool isVariableType{ m_currentType != TokenType::INT
                    && m_currentType != TokenType::STRING
                    && m_currentType != TokenType::REAL };

                if (!isVariableType)
                {
                    throw std::runtime_error("[Parser::variable]: variable type expected");
                }

                getToken();
            }

            void declare()
            {
                if (m_currentType != TokenType::VARIABLE)
                {
                    throw std::runtime_error("[Parser::declare]: var expected");
                }

                getToken();
                variable();

                while (m_currentType == TokenType::COMMA)
                {
                    getToken();
                    variable();
                }
            }

            void program()
            {
                if (m_currentType != TokenType::ENTRY)
                {
                    throw std::runtime_error("[Parser::program]: program expected");
                }

                getToken();
                declare();

                if (m_currentType != TokenType::SEMICOLON)
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

                if (m_currentType != TokenType::FINISH)
                {
                    throw std::runtime_error("[Parser::analyze]: wrong sequence");
                }

                std::cout << "OK\n";
            }
    };

    std::map<std::string, TokenType> Scanner::s_reservedWords {
        { "",        TokenType::NULL },
            { "program", TokenType::ENTRY },
            { "int",     TokenType::INT },
            { "string",  TokenType::STRING },
            { "real",    TokenType::REAL },
            { "var",     TokenType::VARIABLE },
            { "goto",    TokenType::GOTO },
            { "case_of", TokenType::CASE_OF },
            { "while",   TokenType::WHILE },
            { "do",      TokenType::DO },
            { "read",    TokenType::READ },
            { "write",   TokenType::WRITE },
            { "not",     TokenType::NOT },
            { "and",     TokenType::AND },
            { "or",      TokenType::OR }
    };

    std::map<std::string, TokenType> Scanner::s_delimeters {
        { "",  TokenType::NULL },
            { "{", TokenType::BEGIN },
            { "}", TokenType::END },
            { ";", TokenType::SEMICOLON },
            { ":", TokenType::COLON },
            { ",", TokenType::COMMA },
            { "=", TokenType::ASSIGN },
            { "(", TokenType::OPEN_B },
            { ")", TokenType::CLOSE_B },
            { "==", TokenType::EQ },
            { "<",  TokenType::LESS },
            { ">",  TokenType::GREATER },
            { "!=", TokenType::NEQ },
            { "<=", TokenType::LEQ },
            { ">=", TokenType::GEQ },
            { "+",  TokenType::PLUS },
            { "-",  TokenType::MINUS },
            { "*",  TokenType::MULTIPLY },
            { "/",  TokenType::DIVIDE },
            { ".",  TokenType::POINT }
    };

    std::unordered_map<std::string, Ident> Scanner::s_TID;
}

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
        while((token = scaner.getToken()).getType() != mli::TokenType::FINISH)
        {
            std::cout << token << "\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

