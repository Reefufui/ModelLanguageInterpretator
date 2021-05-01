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

    class Mark {
        private:
            std::string m_name;
            uint32_t    m_value;

            bool        m_isMet{0};
            size_t      m_polizID;

            static int  m_markCount;
            int         m_id;

        public:

            Mark()
            {
            }

            Mark(const char* a_name)
            {
                m_name = a_name;
                m_id = m_markCount;
                ++m_markCount;
            }

            Mark(const std::string& a_name)
                : m_name(a_name)
            {
                m_id = m_markCount;
                ++m_markCount;
            }

            Mark(std::string&& a_name)
                : m_name(a_name)
            {
                m_id = m_markCount;
                ++m_markCount;
            }

            bool operator==(const std::string& a_str)
            {
                return m_name == a_str;
            }

            friend std::ostream& operator<<(std::ostream &a_out, const Mark& a_ident)
            {
                return a_out << a_ident.m_name << " (" << " with ID = " << a_ident.m_id << ")";
            }

            int getID() const
            {
                return m_id;
            }

            bool isMet() const
            {
                return m_isMet;
            }

            void setMet()
            {
                m_isMet = true;
            }

            size_t getPolizID() const
            {
                return m_polizID;
            }

            void setPolizID(size_t a_id)
            {
                m_polizID = a_id;
            }

            uint32_t getValue() const
            {
                return m_value;
            }
    };

    int Mark::m_markCount{};
}

#endif // IDENT_HPP
