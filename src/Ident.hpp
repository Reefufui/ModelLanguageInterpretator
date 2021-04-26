#ifndef IDENT_HPP
#define IDENT_HPP

#include <string>

#include "Token.hpp"

#undef NULL

namespace mli {

    class Ident {
        private:
            std::string m_name;
            Token::Type m_type;

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

            friend std::ostream& operator<<(std::ostream &a_out, const Ident& a_ident)
            {
                return a_out << a_ident.m_name << " (" << a_ident.m_type << " with ID = " << a_ident.m_id << ")";
            }

            int getID() const
            {
                return m_id;
            }

            std::string getName() const
            {
                return m_name;
            }

            Token::Type getType() const
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

            void setType(const Token::Type& a_type)
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
}

#endif // IDENT_HPP
