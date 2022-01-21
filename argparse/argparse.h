#ifndef ARGPARSE_H__
#define ARGPARSE_H__

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
#include <map>
#include <fstream>

namespace argparse
{
    namespace string_utils
    {
        template<typename CharT>
        inline bool starts_with(const std::basic_string<CharT> &str, const std::basic_string<CharT> &prefix)
        {
            return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
        }

        template<typename CharT>
        inline std::vector<std::basic_string<CharT>> split(const std::basic_string<CharT> &string_to_split, const std::basic_string<CharT> &delimiter)
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
        inline std::basic_string<CharT> join(const std::vector<std::basic_string<CharT>> &split_string, const std::basic_string<CharT> &delimiter)
        {
            if (split_string.empty()) return std::basic_string<CharT>();
            if (split_string.size() == 1) return split_string[0];
            auto it = split_string.begin();
            std::basic_string<CharT> joined_string = *it++;
            for (; it != split_string.end(); ++it)
            {
                joined_string += delimiter;
                joined_string += *it;
            }
            return joined_string;
        }

        template<typename CharT>
        inline std::basic_string<CharT> trim_left(const std::basic_string<CharT>& string_to_trim, const CharT delimiter)
        {
            if (string_to_trim.empty()) return std::basic_string<CharT>();
            typename std::basic_string<CharT>::const_iterator p = string_to_trim.cbegin();
            while (*p == delimiter) ++p;
            return p == string_to_trim.cend() ? std::basic_string<CharT>() : std::basic_string<CharT>(p, string_to_trim.end());
        }

        template<typename CharT>
        inline std::basic_string<CharT> to_upper(const std::basic_string<CharT>& str)
        {
            std::basic_string<CharT> upper_str(str);
            for (auto& c : upper_str)
            {
                c = std::toupper(c);
            }
            return upper_str;
        }

        template<typename CharT>
        inline std::basic_string<CharT> to_lower(const std::basic_string<CharT>& str)
        {
            std::basic_string<CharT> lower_str(str);
            for (auto& c : lower_str)
            {
                c = std::tolower(c);
            }
            return lower_str;
        }

        inline std::string get_string_with_max_size(const std::vector<std::string>& strs)
        {
            return *std::max_element(strs.begin(), strs.end(), [](const std::string& a, const std::string& b){ return a.size() < b.size(); });
        }

        inline size_t get_max_string_size(const std::vector<std::string>& strs)
        {
            return get_string_with_max_size(strs).size();
        }

        inline bool to_bool(std::string str)
        {
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
            std::istringstream is(str);
            bool b;
            is >> std::boolalpha >> b;
            return b;
        }
    }

    namespace exceptions
    {
        class invalid_config_file_contents : public std::exception
        {
        public:
            explicit invalid_config_file_contents(const std::string& config_value)
            {
                std::stringstream error_msg;
                error_msg << "Error: Config file has invalid format " << config_value << std::endl;
                m_error_message = error_msg.str();
            }

            [[nodiscard]] char const* what() const noexcept override
            {
                return m_error_message.c_str();
            }
        private:
            std::string m_error_message;
        };

        class config_file_does_not_exist_exception : public std::exception
        {
        public:
            explicit config_file_does_not_exist_exception(const std::string& config_file_path)
            {
                std::stringstream error_msg;
                error_msg << "Error: Config file " << config_file_path << " does not exist" << std::endl;
                m_error_message = error_msg.str();
            }

            [[nodiscard]] char const* what() const noexcept override
            {
                return m_error_message.c_str();
            }
        private:
            std::string m_error_message;
        };
        
        class unknown_argument_exception : public std::exception
        {
        public:
            explicit unknown_argument_exception(const std::string& unknown_argument_name)
            {
                std::stringstream error_msg;
                error_msg << "Error: Attempt to Access Unknown Argument: " << unknown_argument_name << std::endl;
                m_error_message = error_msg.str();
            }

            [[nodiscard]] char const* what() const noexcept override
            {
                return m_error_message.c_str();
            }
        private:
            std::string m_error_message;
        };

        class invalid_narg_mode_exception : public std::exception
        {
        public:
            explicit invalid_narg_mode_exception(const std::string& narg_mode)
            {
                std::stringstream error_msg;
                error_msg << "Error: Invalid NARGs specification: " << narg_mode << std::endl;
                m_error_message = error_msg.str();
            }

            [[nodiscard]] char const* what() const noexcept override
            {
                return m_error_message.c_str();
            }
        private:
            std::string m_error_message;
        };

        class incorrect_num_args_exception : public std::exception
        {
        public:
            explicit incorrect_num_args_exception(uint32_t expected_num, uint32_t actual_num)
            {
                std::stringstream error_msg;
                error_msg << "Error: Incorrect number of arguments given - expected: " << expected_num << ", given: " << actual_num << std::endl;
                m_error_message = error_msg.str();
            }

            [[nodiscard]] char const* what() const noexcept override
            {
                return m_error_message.c_str();
            }
        private:
            std::string m_error_message;
        };
    }

    namespace validate
    {
        inline bool is_optional(const std::string& argument_name)
        {
            return string_utils::starts_with(argument_name, std::string("-"));
        }

        inline bool is_valid_argument_flags(const std::vector<std::string>& argument_names)
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

    namespace constants
    {
        const std::string HELP_FLAG = "--help";
        const std::string HELP_SHORT_FLAG = "-h";
    }

    enum class NargsMode
    {
        Integer, // N - consumes N arguments into a list
        Single, // ? - consumes one arg if possible and produces a single item - if no arg specified then default value will be used
        All, // * - all args are gathered into a list.
        AtLeastOne, // + - (like *) all args are gathered into a list. Error message generated if there wasn't at least one arg present
    };

    // For now we just support : or = delimited values (non nested) with each on a separate line
    class config_file_reader
    {
    public:
        std::map<std::string, std::string> read_args(const std::string& filename)
        {
            std::cout << "[DEBUG] Opening: " << filename << std::endl;
            std::map<std::string, std::string> config;
            std::ifstream infile(filename);
            std::string line;
            while (std::getline(infile, line))
            {
                std::cout << "[DEBUG] Config line: " << line << std::endl;
                auto line_split_by_equals = string_utils::split(line, std::string("="));
                if (line_split_by_equals.size() != 2)
                {
                    throw exceptions::invalid_config_file_contents(line);
                }
                config[line_split_by_equals[0]] = line_split_by_equals[1];
            }
            return config;
        }
    };

    class argument
    {
    public:
        argument(const std::vector<std::string>& names) : m_flags(names), m_nargs(1), m_nargs_mode(NargsMode::Integer)
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
            std::cout << "[DEBUG] Attempting to add " << value << " to argument " << get_name_string() << std::endl;
            if (m_nargs_mode == NargsMode::All || m_values.size() < m_nargs)
            {
                m_values.push_back(value);
                std::cout << "[DEBUG] Added " << value << " to argument." << std::endl;
                return;
            }
            std::stringstream ss;
            ss << "Error: Attempt to store more than " << m_nargs << " values in argument " << m_destination << ".";
            throw std::runtime_error(ss.str());
        }

        void clear_values()
        {
            m_values.clear();
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
                if (utils::is_container_value<ReturnArgT>)
                {
                    // Return all the values
                    return any_container_cast<ReturnArgT>({m_default_value});
                }
                else
                {
                    return std::any_cast<ReturnArgT>(m_default_value);
                }
            }
            if (m_nargs_mode == NargsMode::All || m_nargs_mode == NargsMode::Single)
            {
                return ReturnArgT();
            }
            // TODO: Custom exception
            throw std::runtime_error("No value provided for argument.");
        }

        // The number of command-line arguments that should be consumed.
        argument& num_args(size_t n)
        {
            m_nargs_mode = NargsMode::Integer;
            m_nargs = n;
            return *this;
        }

        argument& num_args(std::string n)
        {
            if (n == "?")
            {
                m_nargs_mode = NargsMode::Single;
            }
            else if (n == "*")
            {
                m_nargs_mode = NargsMode::All;
            }
            else if (n == "+")
            {
                m_nargs_mode = NargsMode::AtLeastOne;
            }
            else
            {
                throw exceptions::invalid_narg_mode_exception(n);
            }
            return *this;
        }

        size_t num_args() const
        {
            return m_nargs;
        }

        NargsMode num_args_mode() const
        {
            return m_nargs_mode;
        }

        // The value produced if the argument is absent from the command line and if it is absent from the namespace object.
        // This works for all optional arguments but only works
        // for positional args set to Single (?) or All (*)
        template<typename ArgT>
        argument& default_value(ArgT value)
        {
            m_default_value = value;
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

        // TODO: Implement this to simplify the usage string output
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
            bool is_flag = std::find_if(
                    m_flags.begin(),
                    m_flags.end(),
                    [&arg_name](const std::string& flag)
                    {
                        return flag == arg_name;
                    }
            ) != m_flags.end();
            bool is_dest = arg_name == m_destination;
            return is_flag || is_dest;
        }

        bool is_set() const noexcept
        {
            if (m_nargs_mode == NargsMode::All || m_nargs_mode == NargsMode::Single || m_default_value.has_value())
            {
                return true;
            }
            if (m_values.size() < m_nargs)
            {
                return false;
            }
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
        NargsMode m_nargs_mode;
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
                , m_environment_prefix("APP_ENV_")
                , m_config_filename("app_config")
        {
            add_argument({constants::HELP_SHORT_FLAG, constants::HELP_FLAG})
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

        argument_parser& set_environment_prefix(const std::string& prefix)
        {
            m_environment_prefix = prefix;
            return *this;
        }

        argument_parser& set_config_filename(const std::string& filename)
        {
            m_config_filename = filename;
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
            throw exceptions::unknown_argument_exception(name);
        }

        // Support a cascading level of configuration
        // config file > environment variables > commandline
        void parse_args(int argc, char *argv[])
        {
            std::vector<std::string> command_line_args;
            // Always get command line input
            std::copy(argv, argv + argc, std::back_inserter(command_line_args));
            std::cout << "[DEBUG] " << string_utils::join(command_line_args, std::string(", ")) << std::endl;
            // Check if we have a name, if not, then take it from the arguments list
            if (m_program_name.empty() && !command_line_args.empty())
            {
                m_program_name = std::filesystem::path(command_line_args.front()).filename().string();
            }
            // Process each of the inputs in order so that those with lesser preference are overwritten
            process_command_line_arguments(command_line_args);
            process_environment_arguments();
            process_config_file_arguments();

            // Now we need to check that all positional arguments have been filled
            check_for_missing_arguments();
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

        void consume_n_args(
                argument& arg,
                std::vector<std::string>& command_line_args,
                std::vector<std::string>::iterator& current_arg)
        {
            // Integer, N - consumes N arguments into a list
            auto count = arg.num_args();
            while (count != 0)
            {
                // check each arg is not another flag or we've reached the end (not enough args)
                if (current_arg == command_line_args.end() || validate::is_optional(*current_arg))
                {
                    std::stringstream ss;
                    ss << "Error: Insufficient positional arguments. " << arg.dest() << " expected " << count << " more input(s) (" << arg.num_args() << " total)." << std::endl;
                    print_error_usage_and_exit(ss.str());
                }
                else
                {
                    // Grab the value and stick it in the next positional argument
                    arg.value(*current_arg);
                }
                // Advance to the next arg unless its the last one
                if (count > 1)
                {
                    ++current_arg;
                }
                --count;
            }
        }

        void consume_all_args(
                argument& arg,
                std::vector<std::string>& command_line_args,
                std::vector<std::string>::iterator& current_arg)
        {
            // All - * - all args are gathered into a list.
            bool still_consuming = true;
            while (still_consuming)
            {
                if ((current_arg+1 == command_line_args.end())
                    || (!validate::is_optional(*current_arg) && validate::is_optional(*(current_arg+1))))
                {
                    arg.value(*current_arg);
                    still_consuming = false;
                    break;
                }
                else
                {
                    arg.value(*current_arg);
                    ++current_arg;
                }
            }
        }

        void consume_at_least_one_arg(
                argument& arg,
                std::vector<std::string>& command_line_args,
                std::vector<std::string>::iterator& current_arg)
        {
            // AtLeastOne - + - (like *) all args are gathered into a list. Error message generated if there wasn't at least one arg present

            // check each arg is not another flag or we've reached the end (not enough args)
            if (current_arg == command_line_args.end() || validate::is_optional(*current_arg))
            {
                std::stringstream ss;
                ss << "Error: Insufficient positional arguments. " << arg.dest() << " expected one or more (+) input(s)." << std::endl;
                print_error_usage_and_exit(ss.str());
            }

            bool still_consuming = true;
            while (still_consuming)
            {
                if ((current_arg+1 == command_line_args.end())
                    || (!validate::is_optional(*current_arg) && validate::is_optional(*(current_arg+1))))
                {
                    arg.value(*current_arg);
                    still_consuming = false;
                    break;
                }
                else
                {
                    arg.value(*current_arg);
                    ++current_arg;
                }
            }
        }

        void consume_single_arg(
                argument& arg,
                std::vector<std::string>& command_line_args,
                std::vector<std::string>::iterator& current_arg)
        {
            // Single - ? - consumes one arg if possible and produces a single item - if no arg specified then default value will be used
            // Check next arg is not another flag or if we've reached the end (not enough args)
            if (current_arg == command_line_args.end() || validate::is_optional(*current_arg))
            {
                // pass
            }
            else
            {
                // Grab the value and stick it in the next positional argument
                std::cout << "[DEBUG] Consume single arg: adding " << *current_arg << " to argument " << arg.dest() << std::endl;
                arg.value(*current_arg);
            }
        }

        std::vector<argument>::iterator get_optional_argument_by_name(const std::string& name)
        {
            auto arg_it = std::find_if(
                    m_optional_arguments.begin(),
                    m_optional_arguments.end(),
                    [&name](const argument& arg)
                    {
                        return arg.matches_arg_name(name);
                    });
            return arg_it;
        }

        std::vector<argument>::iterator get_positional_argument_by_name(const std::string& name)
        {
            auto arg_it = std::find_if(
                    m_positional_arguments.begin(),
                    m_positional_arguments.end(),
                    [&name](const argument& arg)
                    {
                        return arg.matches_arg_name(name);
                    });
            return arg_it;
        }

        std::vector<argument>::iterator get_argument_by_name(const std::string& name)
        {
            auto pos_arg_it = get_positional_argument_by_name(name);
            if (pos_arg_it != m_positional_arguments.end())
            {
                return pos_arg_it;
            }
            auto opt_arg_it = get_optional_argument_by_name(name);
            if (opt_arg_it != m_optional_arguments.end())
            {
                return opt_arg_it;
            }
            throw exceptions::unknown_argument_exception(name);
        }

        void check_for_missing_arguments()
        {
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

        argument& process_optional_command_line_argument(const std::vector<std::string>& command_line_args, std::vector<std::string>::iterator& it)
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
                        // check each is not another flag or we haven't hit the end
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
                return *arg_it;
            }
        }

        void process_num_args(argument& argToProcess, std::vector<std::string>& values, std::vector<std::string>::iterator& current_arg)
        {
            switch (argToProcess.num_args_mode())
            {
                case NargsMode::Integer:
                    consume_n_args(argToProcess, values, current_arg);
                    break;
                case NargsMode::All:
                    consume_all_args(argToProcess, values, current_arg);
                    break;
                case NargsMode::AtLeastOne:
                    consume_at_least_one_arg(argToProcess, values, current_arg);
                    break;
                case NargsMode::Single:
                    consume_single_arg(argToProcess, values, current_arg);
                    break;
            }
        }

        void process_positional_command_line_argument(size_t& pos_arg_index, std::vector<std::string>& command_line_args, std::vector<std::string>::iterator& current_arg)
        {
            // Verify we have enough positional arguments
            if (pos_arg_index >= m_positional_arguments.size())
            {
                std::stringstream ss;
                ss << "Error: Too many positional arguments." << std::endl;
                print_error_usage_and_exit(ss.str());
            }
            // Consume the required number of arguments
            // Get the next num_arg arguments
            argument& pos_arg = m_positional_arguments[pos_arg_index];
            process_num_args(pos_arg, command_line_args, current_arg);
            pos_arg_index++;
        }

        void process_optional_config_file_argument(argument& argToProcess, std::vector<std::string>& values)
        {
            auto num_args = argToProcess.num_args();
            argToProcess.clear_values();
            if (num_args > 0)
            {
                if (values.size() != num_args)
                {
                    throw exceptions::incorrect_num_args_exception(num_args, values.size());
                }
                for (auto value : values)
                {
                    argToProcess.value(value);
                }
            }
            else
            {
                if (values.size() != 1)
                {
                    throw exceptions::incorrect_num_args_exception(1, values.size());
                }
                argToProcess.value(string_utils::to_bool(values[0]));
            }
        }

        void process_positional_config_file_argument(argument& argToProcess, std::vector<std::string>& values)
        {
            process_num_args(argToProcess, values, values.begin());
        }

        std::string get_usage_string() const
        {
            // TODO: Alter this to alert what the current config file is and the current env prefix
            // Optional arguments are surrounded by [], with the name being the longest flag available
            // Positional arguments are printed as the dest name
            std::stringstream ss;
            ss << "Usage: " << m_program_name << " ";
            for (auto& arg : m_optional_arguments)
            {
                // If single -- name show that, if only - name, show that arg.
                std::string longest_arg_name = arg.get_longest_name_string();
                std::string arg_output_str = string_utils::to_upper(string_utils::trim_left(longest_arg_name, '-'));
                ss << "[" << longest_arg_name;
                size_t count = arg.num_args();
                while (count > 0)
                {
                    ss << " " << arg_output_str;
                    --count;
                }
                ss << "] ";
            }
            for (auto& arg : m_positional_arguments)
            {
                switch (arg.num_args_mode())
                {
                    case NargsMode::Integer:
                    {
                        size_t count = arg.num_args();
                        while (count > 0)
                        {
                            // Output the dest name
                            ss << arg.dest() << " ";
                            --count;
                        }
                        break;
                    }
                    case NargsMode::All:
                    {
                        ss << "[" << arg.dest() << " [" << string_utils::to_upper(arg.dest()) << " ...]] ";
                        break;
                    }
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

        void process_config_file_arguments()
        {
            // See if file exists in executable directory
            auto abs_config_filepath = std::filesystem::absolute(m_config_filename);
            bool config_file_exists = std::filesystem::exists(abs_config_filepath);
            if (!config_file_exists)
                // Nothing to do
                return;
            std::cout << "[DEBUG] Reading " << abs_config_filepath << std::endl;
            config_file_reader reader;
            auto config_results = reader.read_args(abs_config_filepath.string());
            for (auto& entry : config_results)
            {
                std::cout << "[DEBUG] Name: " << entry.first << ", Value: " << entry.second << std::endl;
                try
                {
                    auto arg = get_argument_by_name(entry.first);
                    std::vector<std::string> arg_list = string_utils::split(entry.second, std::string(","));
                    std::cout << "[DEBUG] Found matching argument: " << arg->dest() << std::endl;
                    if (arg->is_optional())
                    {
                        std::cout << "[DEBUG] Processing optional config file arg: " << arg->dest() << std::endl;
                        process_optional_config_file_argument(*arg, arg_list);
                    }
                    else
                    {
                        std::cout << "[DEBUG] Processing positional config file arg: " << arg->dest() << std::endl;
                        process_positional_config_file_argument(*arg, arg_list);
                    }
                }
                catch (exceptions::unknown_argument_exception& e)
                {
                    // Unknown arguments in config files can just be ignored
                    // Continue on!
                    std::cout << "[DEBUG] Unknown argument detected " << e.what() << std::endl;
                }
            }
        }

        std::string get_environment_variable_value(const std::string& key)
        {
            //                char * val = getenv( key.c_str() );
//                return val == NULL ? std::string("") : std::string(val);
        }

        void process_environment_arguments()
        {
            // Get values from environment
        }

        void process_command_line_arguments(std::vector<std::string>& command_line_args)
        {
            // Iterate over the given arguments
            size_t pos_index = 0;
            for (auto it = command_line_args.begin() + 1; it < command_line_args.end(); ++it)
            {
                // Check if its positional or not
                if (validate::is_optional(*it))
                {
                    process_optional_command_line_argument(command_line_args, it);
                }
                else
                {
                    process_positional_command_line_argument(pos_index, command_line_args, it);
                }
            }
        }

    private:
        std::string m_program_name;
        std::string m_description;
        std::vector<argument> m_positional_arguments;
        std::vector<argument> m_optional_arguments;
        std::string m_environment_prefix;
        std::string m_config_filename;
    };
}

#endif // ARGPARSE_H__
