#ifndef SEMANTICAL_ERROR_HPP
#define SEMANTICAL_ERROR_HPP

#include <ostream>

namespace mli {

    template <class T>
    class SemanticError
    {
        private:
            uint32_t    m_onRow{};
            T           m_trigger{};
            std::string m_message{};

        public:
            SemanticError(uint32_t a_onRow, const T a_trigger, const std::string& a_message)
                : m_onRow(a_onRow), m_trigger(a_trigger), m_message(a_message)
            {
            }

            friend std::ostream& operator<<(std::ostream &a_out, const SemanticError& a_error)
            {
                std::cout << "[SemanticError]: ";
                std::cout << a_error.m_trigger << " " << a_error.m_message << " on line: " << a_error.m_onRow;

                return a_out;
            }
    };
}

#endif // SEMANTICAL_ERROR_HPP

