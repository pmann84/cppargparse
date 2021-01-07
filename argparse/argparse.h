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

// TODO: Must support negative numbers as arguments! This isnt supported currently! :S
// TODO: Must add proper support for multi args options, currently the values are overwritten everytime!
// TODO: Nested parsers - make sure you sort out help strings for this
// Arguments must maintain the order that they are added in
// Display optional args first then positional
// Optional arguments can be input in any order around positional arguments, but positional args must be in order relative to themselves

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

        std::string to_upper(const std::string& str)
        {
            std::string upper_str = str;
            std::for_each(
                    upper_str.begin(),
                    upper_str.end(),
                    [](char & c)
                    {
                        c = std::toupper(c);
                    }
                );
            return upper_str;
        }

        std::string to_lower(const std::string& str)
        {
            std::string lower_str = str;
            std::for_each(
                    lower_str.begin(),
                    lower_str.end(),
                    [](char & c)
                    {
                        c = std::tolower(c);
                    }
            );
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

    namespace exception
    {
        class invalid_argument : public std::exception
        {
        public:
            explicit invalid_argument(const std::string& message, const std::string& usage_str)
                    : invalid_argument("", message, usage_str)
            {
            }

            explicit invalid_argument(const std::string& invalid_arg_name, const std::string& message, const std::string& usage_str)
                    : m_invalid_arg_name(invalid_arg_name)
                    , m_message(message)
                    , m_usage_str(usage_str)
            {
            }

            virtual char const* what() const noexcept
            {
                std::stringstream error_msg;
                error_msg << "Error: Invalid Argument: ";
                if (!m_invalid_arg_name.empty())
                {
                    error_msg << m_invalid_arg_name << std::endl;
                }
                error_msg << m_message << std::endl;
                error_msg << m_usage_str << std::endl;
                return error_msg.str().c_str();
            }
        private:
            std::string m_usage_str;
            std::string m_invalid_arg_name;
            std::string m_message;
        };

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

        class insufficient_arguments : public std::exception
        {
        public:
            explicit insufficient_arguments(
                    const std::vector<std::string>& missing_argument_names,
                    const std::string& usage_str)
                    : m_missing_argument_names(missing_argument_names)
                    , m_usage_str(usage_str)
            {
            }

            virtual char const* what() const noexcept
            {
                std::stringstream error_msg;
                error_msg << "Error: The following arguments are required: " << string_utils::join(m_missing_argument_names, std::string(", ")) << std::endl;
                return error_msg.str().c_str();
            }
        private:
            std::vector<std::string> m_missing_argument_names;
            std::string m_usage_str;
        };
    }

    namespace utils
    {
        bool is_optional(const std::string& argument_name)
        {
            return string_utils::starts_with(argument_name, std::string("-"));
        }

        bool validate_optional(const std::vector<std::string>& argument_names)
        {
            bool positional = false;
            bool optional = false;
            for (auto& name : argument_names)
            {
                if (is_optional(name))
                {
                    optional = true;
                } else
                {
                    positional = true;
                }
            }
            if (optional && positional) return false;
            return true;
        }
    }

    class argument
    {
    public:
        argument(const std::vector<std::string>& names) : m_flags(names), m_nargs(1)
        {
            // Validate that if one name starts with - then all must!
            if (!utils::validate_optional(m_flags))
            {
               throw std::runtime_error("Error: Invalid option string: all names must start with a character '-' or none of them.");
            }
            // Use the longest name as the destination
            std::string longest_arg = string_utils::get_string_with_max_size(m_flags);
            m_destination = longest_arg;
            if (utils::is_optional(longest_arg))
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
            m_value = value;
        }

        template <typename ArgT>
        ArgT value()
        {
            return std::any_cast<ArgT>(m_value);
        }

        // The number of command-line arguments that should be consumed.
        argument &num_args(size_t n)
        {
            // TODO: Validate - defaults to 1 but can be set to zero for a true flag
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
            m_value = false;
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
            return utils::is_optional(m_flags[0]);
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
            return m_value.has_value();
        }
    private:
        // TODO: this needs to be a vector of values so we can store num_args
        std::any m_value;
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

        template <typename ArgT>
        ArgT get(std::string name)
        {
            std::cout << "Getting Argument: " << name << std::endl;
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
                std::cout << "Found Positional Argument: " << pos_it->dest() << std::endl;
                return pos_it->value<ArgT>();
            }

            if (opt_it != m_optional_arguments.end())
            {
                std::cout << "Found Optional Argument: " << opt_it->dest() << std::endl;
                return opt_it->value<ArgT>();
            }
            throw exception::unknown_argument(name);
        }

        void parse_args(int argc, char *argv[])
        {
            std::vector<std::string> command_line_args;
            std::copy(argv, argv + argc, std::back_inserter(command_line_args));

            // Check if we have a name, if not, then take it from the arguments list
            if (m_program_name.empty() && !command_line_args.empty())
            {
                m_program_name = command_line_args.front();
            }

            // Iterate over the given arguments
            size_t pos_index = 0;
            for (auto it = command_line_args.begin() + 1; it < command_line_args.end(); ++it)
            {
                // Check if its positional or not
                if (utils::is_optional(*it))
                {
                    std::cout << "Optional Argument Detected: " << *it << std::endl;
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
                                std::cout << "Comparing " << arg.get_name_string() << " against " << *it << std::endl;
                                return arg.matches_arg_name(*it);
                            }
                    );
                    if (arg_it == m_optional_arguments.end())
                    {
                        std::cout << "Error: Unknown optional argument. " << *it << std::endl;
                        std::cout << get_usage_and_help_string() << std::endl;
                        std::exit(1);
                        // throw exception::invalid_argument(*it, "Invalid Optional Argument", get_usage_string());
                    }
                    else
                    {
                        // Check if we want the
                        std::cout << "Matched argument " << *it << " with optional argument " << arg_it->get_name_string() << std::endl;
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
                                if (it == command_line_args.end() || utils::is_optional(*it))
                                {
                                    std::cout << "Error: Insufficient optional arguments. " << arg_it->dest() << " expected " << count << " more inputs (" << arg_it->num_args() << " total)." << std::endl;
                                    std::cout << get_usage_and_help_string() << std::endl;
                                    std::exit(1);
//                                  throw exception::invalid_argument(*it, "Not expecting optional argument!", get_usage_string());
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
                    std::cout << "Positional Argument Detected: " << *it << std::endl;
                    // Verify we have enough positional arguments
                    if (pos_index >= m_positional_arguments.size())
                    {
                        std::cout << "Error: Too many positional arguments." << std::endl;
                        std::cout << get_usage_and_help_string() << std::endl;
                        std::exit(1);
                        //throw exception::invalid_argument("Too many positional arguments.", get_usage_string());
                    }
                    // Consume the required number of arguments
                    // Get the next num_arg arguments
                    argument& pos_arg = m_positional_arguments[pos_index];
                    auto count = pos_arg.num_args();
                    while (count != 0)
                    {
                        // check each is not another flag or we've reached the end (not enough args)
                        if (it == command_line_args.end() || utils::is_optional(*it))
                        {
                            std::cout << "Error: Insufficient positional arguments. " << pos_arg.dest() << " expected " << count << " more inputs (" << pos_arg.num_args() << " total)." << std::endl;
                            std::cout << get_usage_and_help_string() << std::endl;
                            std::exit(1);
                            // throw exception::invalid_argument(*it, "Not expecting optional argument!", get_usage_string());
                        }
                        else
                        {
                            std::cout << "Adding arg " << pos_arg.num_args() - count << " to " <<  pos_arg.dest() << std::endl;
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
                throw exception::insufficient_arguments(missing_arguments, get_usage_string());
            }
        }

        void print_help() const
        {

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
            // TODO: Space the output of this function better
            std::stringstream ss;
            ss << std::endl;
            if (!m_description.empty())
            {
                ss << m_description << std::endl << std::endl;
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
                    // TODO: Add generation of default info, specify this in brackets after the above info
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

    private:
    std::string m_program_name;
    std::string m_description;
    std::vector<argument> m_positional_arguments;
    std::vector<argument> m_optional_arguments;
};
}

#endif // __ARGPARSE_H__
