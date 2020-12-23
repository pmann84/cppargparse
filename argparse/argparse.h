#ifndef __ARGPARSE_H__
#define __ARGPARSE_H__

#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdarg>
#include <algorithm>
#include <sstream>
#include <functional>

// Arguments must maintain the order that they are added in
// Display optional args first then positional

namespace argparse
{
    namespace string_utils
    {
        template<typename CharT>
        bool starts_with(const std::basic_string<CharT> &str, const std::basic_string<CharT> &prefix)
        {
            return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
        }

        template<typename CharT>
        std::vector<std::basic_string<CharT>> split(const std::basic_string<CharT> &string_to_split, const std::basic_string<CharT> &delimiter)
        {
            std::vector<std::basic_string<CharT>> split_string;
            if (string_to_split.empty()) return split_string;
            std::size_t pos = string_to_split.find(delimiter);
            std::size_t initial_pos = 0;

            // Decompose statement
            while (pos != std::basic_string<CharT>::npos)
            {
                split_string.push_back(string_to_split.substr(initial_pos, pos - initial_pos));
                initial_pos = pos + delimiter.size();
                pos = string_to_split.find(delimiter, initial_pos);
            }

            // Add the last one
            split_string.push_back(
                    string_to_split.substr(initial_pos, std::min(pos, string_to_split.size()) - initial_pos + 1));

            return split_string;
        }

        template<typename CharT>
        std::basic_string<CharT> join(const std::vector<std::basic_string<CharT>> &split_string, const std::basic_string<CharT> &delimiter)
        {
            if (split_string.empty()) return std::basic_string<CharT>();
            if (split_string.size() == 1) return split_string[0];
            typename std::vector<std::basic_string<CharT>>::const_iterator it = split_string.begin();
            std::basic_string<CharT> joined_string = *it++;
            for (; it != split_string.end(); ++it)
            {
                joined_string += delimiter;
                joined_string += *it;
            }
            return joined_string;
        }

        template<typename CharT>
        std::basic_string<CharT> trim_left(const std::basic_string<CharT>& string_to_trim, const CharT delimiter)
        {
            if (string_to_trim.empty()) return std::basic_string<CharT>();
            typename std::basic_string<CharT>::const_iterator p = string_to_trim.cbegin();
            while (*p == delimiter) ++p;
            return p == string_to_trim.cend() ? std::basic_string<CharT>() : std::basic_string<CharT>(p, string_to_trim.end());
        }

    }

    std::string get_string_with_max_size(const std::vector<std::string>& strs)
    {
        return *std::max_element(strs.begin(), strs.end(), std::greater<std::string>());
    }

    size_t get_max_string_size(const std::vector<std::string>& strs)
    {
        return std::max_element(strs.begin(), strs.end(), std::greater<std::string>())->size();
    }

    class invalid_argument_exception :public std::exception
    {
    public:
        invalid_argument_exception(const std::string& invalid_arg_name, const std::string& usage_str) : m_usage_str(usage_str), m_invalid_arg_name(invalid_arg_name) {}

        virtual char const* what() const noexcept
        {
            std::stringstream error_msg;
            error_msg << m_usage_str << std::endl;
            error_msg << "Error: Unrecognised argument: " << m_invalid_arg_name << std::endl;
            return error_msg.str().c_str();
        }
    private:
        std::string m_usage_str;
        std::string m_invalid_arg_name;
    };

    class argument {
    public:
        argument(const std::vector<std::string>& names) : m_flags(names), m_nargs(0)
        {
            std::string longest_arg = get_string_with_max_size(names);
            m_destination = longest_arg;
            if (is_optional(longest_arg))
            {
                m_destination = string_utils::trim_left(longest_arg, '-');
            }
        }

        explicit argument(const std::string& name) : argument(std::vector<std::string>(1, name))
        {
        }

        // The number of command-line arguments that should be consumed.
        argument &nargs(size_t n)
        {
            m_nargs = n;
            return *this;
        }

        // The value produced if the argument is absent from the command line and if it is absent from the namespace object.
        argument& default_value()
        {
            return *this;
        }

        argument& help(const std::string& help)
        {
            m_help = help;
            return *this;
        }

        std::string help() const
        {
            return m_help;
        }

        argument& dest(const std::string& dest)
        {
            m_destination = dest;
            return *this;
        }

        static bool is_optional(const std::string& argument_name)
        {
            return string_utils::starts_with(argument_name, std::string("-"));
        }

        bool is_optional() const
        {
            return is_optional(m_flags[0]);
        }

//        friend std::ostream &operator<<(std::ostream &os, const argument &arg);

        std::string get_name_string() const
        {
            return string_utils::join(m_flags, std::string(", "));
        }

    private:
        std::string m_destination;
        std::vector<std::string> m_flags;
        std::string m_help;
        size_t m_nargs;
    };

//    std::ostream &operator<<(std::ostream &os, const argument &arg)
//    {
//        os << << ":" << std::setw(50)  << arg.m_description;
//        return os;
//    }

//name or flags - Either a name or a list of option strings, e.g. foo or -f, --foo.
//action - The basic type of action to be taken when this argument is encountered at the command line.
//const - A constant value required by some action and nargs selections.
//type - The type to which the command-line argument should be converted.
//choices - A container of the allowable values for the argument.
//required - Whether or not the command-line option may be omitted (optionals only).
//help - A brief description of what the argument does.
//metavar - A name for the argument in usage messages.
//dest - The name of the attribute to be added to the object returned by parse_args().


    class argument_parser
    {
    public:
        argument_parser(std::string program_name, std::string description)
                : m_program_name(std::move(program_name))
                , m_description(std::move(description))
        {
            add_argument({"-h", "--help"}).help("Show this help message and exit.");
        }

        argument_parser &description(const std::string &description)
        {
            m_description = description;
            return *this;
        }

//    argument& add_argument(const std::string& flags)
//    {
//        std::vector<std::string> flags_or_names;
//        std::va_list inputs;
//        va_start(inputs, flags);
//        for (int i = 0; i < flags; ++i)
//        {
//
//        }
//
//        int result = 0;
//        std::va_list args;
//        va_start(args, count);
//        for (int i = 0; i < count; ++i) {
//            result += va_arg(args, int);
//        }
//        va_end(args);
//        return result;
//    }

        argument& add_argument(const std::initializer_list<std::string> names)
        {
            argument arg(names);
            return add_argument(arg);
        }

        argument& add_argument(const std::string &name)
        {
            argument arg(name);
            return add_argument(arg);
        }

        void parse_args(int argc, char *argv[])
        {
            std::vector<std::string> command_line_args;
            std::copy(argv, argv + argc, std::back_inserter(command_line_args));
            // Iterate over the arguments in order and process them as such, if we hit an error
            // then we throw with error message
            for (auto& arg : command_line_args)
            {
                // Search arguments and
                std::cout << arg << std::endl;
            }

//            for (auto& arg_received : command_line_args)
//            {
//                if (std::find(m_positional_arguments.begin(), m_positional_arguments.end(), arg_received) != m_positional_arguments.end())
//                {
//
//                }
//            }
            // TODO: Only do this if help is passed in or invalid argument is input
//            if (true)
//            {
//                print_args();
//            }
        }

//        std::string get_usage_string() const
//        {
//            std::stringstream ss;
//            ss << m_program_name << " ";
//            for (auto& arg : m_positional_arguments)
//            {
//                if (arg.is_optional())
//                {
//                    // If single -- name show that, if only - name, show that
////                    arg.
//                }
//                else
//                {
//                    ss <<
//                }
//                ss << " ";
//            }
//            return ss.str();
//        }

    private:
        argument& add_argument(argument arg)
        {
            if (arg.is_optional())
            {
                m_optional_arguments.push_back(arg);
                return m_optional_arguments.back();
            }
            m_positional_arguments.push_back(arg);
            return m_positional_arguments.back();
        }
//        std::string get_parser_details() const
//        {
//            std::stringstream ss;
//            // Print name of parser
//            ss << m_program_name << " - " << m_description << std::endl;
//            // Print out arguments
//            std::vector<std::string> arg_names;
//            std::vector<std::string> arg_descs;
//            for (auto& arg : m_positional_arguments)
//            {
//                arg_names.push_back(arg.get_name_string());
//                arg_descs.push_back(arg.description());
//            }
//
//            size_t max_arg_size =
//                    std::max_element(arg_names.begin(), arg_names.end(), std::greater<std::string>())->size() + 1;
//            size_t description_padding = 50;
//            size_t max_desc_size =
//                    std::max_element(arg_descs.begin(), arg_descs.end(), std::greater<std::string>())->size() +
//                    description_padding;
//            for (auto& arg : m_positional_arguments)
//            {
//                ss << std::setw(max_arg_size) << arg.get_name_string() << ":" << std::setw(max_desc_size)
//                          << arg.description() << std::endl;
//            }
//            return ss.str();
//        }

    private:
        std::string m_program_name;
        std::string m_description;
        std::vector<argument> m_positional_arguments;
        std::vector<argument> m_optional_arguments;
    };
}

#endif // __ARGPARSE_H__
