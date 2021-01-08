#ifndef __ARGPARSE_H__
#define __ARGPARSE_H__

#include <any>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdarg>
#include <algorithm>
#include <sstream>
#include <functional>
#include <filesystem>

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

        template<typename CharT>
        std::basic_string<CharT> to_upper(const std::basic_string<CharT>& str)
        {
            std::basic_string<CharT> upper_str(str);
            for (auto& c : upper_str)
            {
                c = std::toupper(c);
            }
            return upper_str;
        }

        template<typename CharT>
        std::basic_string<CharT> to_lower(const std::basic_string<CharT>& str)
        {
            std::basic_string<CharT> lower_str(str);
            for (auto& c : lower_str)
            {
                c = std::tolower(c);
            }
            return lower_str;
        }

        std::string get_string_with_max_size(const std::vector<std::string>& strs)
        {
            return *std::max_element(strs.begin(), strs.end(), std::less<std::string>());
        }

        size_t get_max_string_size(const std::vector<std::string>& strs)
        {
            return get_string_with_max_size(strs).size();
        }
    }

    namespace exceptions
    {
        class unknown_argument : public std::exception
        {
        public:
            explicit unknown_argument(const std::string& unknown_argument_name) : m_unknown_argument_name(unknown_argument_name)
            {
            }

            virtual char const* what() const noexcept
            {
                std::stringstream error_msg;
                error_msg << "Error: Attempt to Access Unknown Argument: " << m_unknown_argument_name << std::endl;
                return error_msg.str().c_str();
            }
        private:
            std::string m_unknown_argument_name;
        };
    }

    namespace validate
    {
        bool is_optional(const std::string& argument_name)
        {
            return string_utils::starts_with(argument_name, std::string("-"));
        }

        bool is_valid_argument_flags(const std::vector<std::string>& argument_names)
        {
            bool positional = false;
            bool optional = false;
            for (auto& name : argument_names)
            {
                if (is_optional(name))
                {
                    optional = true;
                }
                else
                {
                    positional = true;
                }
            }
            if (optional && positional) return false;
            return true;
        }
    }

    namespace utils
    {
        // Primary template that supports types that arent containers
        template<typename T, typename = void>
        struct is_container : std::false_type {};

        // Specialisation that says strings are not classed as containers here
        template<>
        struct is_container<std::string> : std::false_type {};

        // Specialisation that recognises types that are containers
        template<typename T>
        struct is_container<T,
                std::void_t<typename T::value_type,
                        decltype(std::declval<T>().begin()),
                        decltype(std::declval<T>().end())>
        > : std::true_type {};

        // Convenience for testing types
        template <typename T>
        static constexpr bool is_container_value = is_container<T>::value;
    }

    class argument
    {
    public:
        argument(const std::vector<std::string>& names) : m_flags(names), m_nargs(1)
        {
            // Validate that if one name starts with - then all must!
            if (!validate::is_valid_argument_flags(m_flags))
            {
                throw std::runtime_error(
                        "Error: Invalid option string: all names must start with a character '-' or none of them.");
            }
            // Use the longest name as the destination
            std::string longest_arg = string_utils::get_string_with_max_size(m_flags);
            m_destination = longest_arg;
            if (validate::is_optional(longest_arg))
            {
                m_destination = string_utils::trim_left(longest_arg, '-');
            }

        }

        explicit argument(const std::string& name) : argument(std::vector<std::string>(1, name))
        {
        }

        template<typename ArgT>
        void value(ArgT value)
        {
            if (m_values.size() < m_nargs)
            {
                m_values.push_back(value);
                return;
            }
            std::stringstream ss;
            ss << "Error: Attempt to store more than " << m_nargs << " values in argument " << m_destination << ".";
            throw std::runtime_error(ss.str());
        }

        template <typename T>
        static T any_container_cast(const std::vector<std::any>& container)
        {
            using value_t = typename T::value_type;
            T result;
            std::transform(container.begin(), container.end(), std::back_inserter(result),
                   [](const auto& c_value)
                   {
                       return std::any_cast<value_t>(c_value);
                   }
               );
            return result;
        }

        template<typename ReturnArgT>
        ReturnArgT get() const
        {
            if (!m_values.empty())
            {
                if (utils::is_container_value<ReturnArgT>)
                {
                    // Return all the values
                    return any_container_cast<ReturnArgT>(m_values);
                }
                else
                {
                    return std::any_cast<ReturnArgT>(m_values.front());
                }
            }
            if (m_default_value.has_value())
            {
                return std::any_cast<ReturnArgT>(m_default_value);
            }
            throw std::runtime_error("No value provided for argument.");
        }

        // The number of command-line arguments that should be consumed.
        argument &num_args(size_t n)
        {
            m_nargs = n;
            return *this;
        }

        size_t num_args() const
        {
            return m_nargs;
        }

        // The value produced if the argument is absent from the command line and if it is absent from the namespace object.
        argument& default_value()
        {
            m_default_value = false;
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

        std::string dest() const
        {
            return m_destination;
        }

        bool is_optional() const
        {
            return validate::is_optional(m_flags[0]);
        }

//        friend std::ostream &operator<<(std::ostream &os, const argument &arg);

        std::string get_name_string() const
        {
            return string_utils::join(m_flags, std::string(", "));
        }

        std::string get_longest_name_string() const
        {
            return string_utils::get_string_with_max_size(m_flags);
        }

        bool matches_arg_name(const std::string& arg_name) const
        {
            return std::find_if(
                    m_flags.begin(),
                    m_flags.end(),
                    [&arg_name](const std::string& flag)
                    {
                        return flag == arg_name;
                    }
            ) != m_flags.end();
        }

        bool is_set() const noexcept
        {
            if (m_values.size() < m_nargs) return false;
            bool is_set = true;
            for (auto& val : m_values)
            {
                if (!val.has_value())
                {
                    is_set = false;
                }
            }
            return is_set;
        }

    private:
        std::vector<std::any> m_values;
        std::any m_default_value;
        std::string m_destination;
        std::vector<std::string> m_flags;
        std::string m_help;
        size_t m_nargs;
    };

    class argument_parser
    {
    public:
        argument_parser() : argument_parser("", "")
        {
        }

        argument_parser(std::string program_name, std::string description)
                : m_program_name(std::move(program_name))
                , m_description(std::move(description))
        {
            add_argument({"-h", "--help"})
                    .num_args(0)
                    .help("Show this help message and exit.");
        }

        argument_parser& description(const std::string &description)
        {
            m_description = description;
            return *this;
        }

        argument_parser& name(const std::string& name)
        {
            m_program_name = name;
            return *this;
        }

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

        template <typename ArgT>
        ArgT get(std::string name)
        {
            auto pos_it = std::find_if(
                    m_positional_arguments.begin(),
                    m_positional_arguments.end(),
                    [&name](const argument& arg) -> bool
                    {
                        return arg.dest() == name;
                    }
            );
            auto opt_it = std::find_if(
                    m_optional_arguments.begin(),
                    m_optional_arguments.end(),
                    [&name](const argument& arg) -> bool
                    {
                        return arg.dest() == name;
                    }
            );

            if (pos_it != m_positional_arguments.end())
            {
                return pos_it->get<ArgT>();
            }

            if (opt_it != m_optional_arguments.end())
            {
                return opt_it->get<ArgT>();
            }
            throw exceptions::unknown_argument(name);
        }

        void parse_args(int argc, char *argv[])
        {
            std::vector<std::string> command_line_args;
            std::copy(argv, argv + argc, std::back_inserter(command_line_args));

            // Check if we have a name, if not, then take it from the arguments list
            if (m_program_name.empty() && !command_line_args.empty())
            {
                m_program_name = std::filesystem::path(command_line_args.front()).filename().string();
            }

            // Iterate over the given arguments
            size_t pos_index = 0;
            for (auto it = command_line_args.begin() + 1; it < command_line_args.end(); ++it)
            {
                // Check if its positional or not
                if (validate::is_optional(*it))
                {
                    // If asked for help then print usage and help and exit
                    if (get_help_argument().matches_arg_name(*it))
                    {
                        std::cout << get_usage_and_help_string();
                        std::exit(0);
                    }
                    // Search optional args for the name, increment the iterator and grab the value
                    // if the next value is another optional flag (i.e. no value has been specified)
                    // and nargs is > 0 then ERROR
                    auto arg_it = std::find_if(
                            m_optional_arguments.begin(),
                            m_optional_arguments.end(),
                            [&it](const argument& arg)
                            {
                                return arg.matches_arg_name(*it);
                            }
                    );
                    if (arg_it == m_optional_arguments.end())
                    {
                        std::stringstream ss;
                        ss << "Error: Unknown optional argument. " << *it << std::endl;
                        print_error_usage_and_exit(ss.str());
                    }
                    else
                    {
                        // Get the number of args meant to be consumed
                        if (arg_it->num_args() > 0)
                        {
                            // Get the next num_arg arguments
                            auto count = arg_it->num_args();
                            while (count != 0)
                            {
                                // Get the next arg
                                it++;
                                // check each is not another flag or we havent hit the end
                                if (it == command_line_args.end() || validate::is_optional(*it))
                                {
                                    std::stringstream ss;
                                    ss << "Error: Insufficient optional arguments. " << arg_it->dest() << " expected " << count << " more input(s) (" << arg_it->num_args() << " total)." << std::endl;
                                    print_error_usage_and_exit(ss.str());
                                }
                                else
                                {
                                    arg_it->value(*it);
                                }
                                --count;
                            }
                        }
                        else
                        {
                            arg_it->value(true);
                        }
                    }
                }
                else
                {
                    // Verify we have enough positional arguments
                    if (pos_index >= m_positional_arguments.size())
                    {
                        std::stringstream ss;
                        ss << "Error: Too many positional arguments." << std::endl;
                        print_error_usage_and_exit(ss.str());
                    }
                    // Consume the required number of arguments
                    // Get the next num_arg arguments
                    argument& pos_arg = m_positional_arguments[pos_index];
                    auto count = pos_arg.num_args();
                    while (count != 0)
                    {
                        // check each is not another flag or we've reached the end (not enough args)
                        if (it == command_line_args.end() || validate::is_optional(*it))
                        {
                            std::stringstream ss;
                            ss << "Error: Insufficient positional arguments. " << pos_arg.dest() << " expected " << count << " more input(s) (" << pos_arg.num_args() << " total)." << std::endl;
                            print_error_usage_and_exit(ss.str());
                        }
                        else
                        {
                            // Grab the value and stick it in the next positional argument
                            pos_arg.value(*it);
                        }
                        // Advance to the next arg unless its the last one
                        if (count > 1)
                        {
                            ++it;
                        }
                        --count;
                    }
                    pos_index++;
                }
            }

            // Now we need to check that all positional arguments have been filled
            std::vector<std::string> missing_arguments;
            for (const auto& pos_arg : m_positional_arguments)
            {
                if (!pos_arg.is_set())
                {
                    missing_arguments.push_back(pos_arg.dest());
                }
            }
            if (!missing_arguments.empty())
            {
                std::stringstream ss;
                ss << "Error: The following arguments are required: ";
                for (auto& arg : missing_arguments)
                {
                    ss << arg << " ";
                }
                ss << std::endl;
                print_error_usage_and_exit(ss.str());
            }
        }

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

        argument& get_help_argument()
        {
            return m_optional_arguments[0];
        }

        std::string get_usage_string() const
        {
            // Optional arguments are surrounded by [], with the name being the longest flag available
            // Positional arguments are printed as the dest name
            std::stringstream ss;
            ss << "Usage: " << m_program_name << " ";
            for (auto& arg : m_optional_arguments)
            {
                // If single -- name show that, if only - name, show that arg.
                ss << "[" << arg.get_longest_name_string();
                size_t count = arg.num_args();
                while (count > 0)
                {
                    ss << " " << string_utils::to_upper(string_utils::trim_left(arg.get_longest_name_string(), '-'));
                    --count;
                }
                ss << "] ";
            }
            for (auto& arg : m_positional_arguments)
            {
                size_t count = arg.num_args();
                while (count > 0)
                {
                    // Output the dest name
                    ss << arg.dest() << " ";
                    --count;
                }
            }
            return ss.str();
        }

        std::string get_help_string() const
        {
            std::stringstream ss;
            ss << std::endl;
            if (!m_description.empty())
            {
                ss << "\t" << m_description << std::endl << std::endl;
            }

            if (m_positional_arguments.size() > 0)
            {
                ss << "Positional Arguments: " << std::endl;
                for (auto& pos : m_positional_arguments)
                {
                    ss << pos.get_name_string() << ": " << pos.help() << std::endl;
                }
                ss << std::endl;
            }

            if (m_optional_arguments.size() > 0)
            {
                ss << "Optional Arguments: " << std::endl;
                for (auto& opt : m_optional_arguments)
                {
                    ss << opt.get_name_string() << ": " << opt.help() << std::endl;
                }
                ss << std::endl;
            }
            return ss.str();
        }

        std::string get_usage_and_help_string() const
        {
            std::stringstream ss;
            ss << get_usage_string() << std::endl;
            ss << get_help_string() << std::endl;
            return ss.str();
        }

        void print_error_usage_and_exit(std::string message) const
        {
            std::cout << message << std::endl;
            std::cout << get_usage_string() << std::endl;
            std::exit(1);
        }

        private:
        std::string m_program_name;
        std::string m_description;
        std::vector<argument> m_positional_arguments;
        std::vector<argument> m_optional_arguments;
    };
}

#endif // __ARGPARSE_H__
