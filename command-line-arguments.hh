#pragma once

#include <typeinfo>
#include <functional>
#include <algorithm>
#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <tuple>

// ----------------------------------------------------------------------

namespace command_line_arguments
{
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif
    class CommandLineError : public std::runtime_error
    {
     public: using std::runtime_error::runtime_error;
    };
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

      // special argument type to generate help
    class PrintHelp
    {
     public:
        inline PrintHelp() : mUsage("Usage: {progname} [options]") {}
        template <typename Text> inline PrintHelp(Text aUsage) : mUsage(aUsage) {}

        inline std::string message(std::string progname) const
            {
                std::string msg = mUsage;
                const std::string::size_type pos = msg.find("{progname}");
                if (pos != std::string::npos)
                    msg.erase(pos, 10).insert(pos, progname);
                return msg;
            }

     private:
        std::string mUsage;
    };

    inline std::ostream& operator << (std::ostream& out, const PrintHelp&) { return out; }

// ----------------------------------------------------------------------

      // argument type to count occurences of the corresponding switch
    class Count
    {
     public:
        inline Count() : mValue(0) {}
        inline Count& operator ++ () { ++mValue; return *this; }
        inline operator int () const { return static_cast<int>(mValue); }
        inline operator std::size_t () const { return mValue; }

     private:
        std::size_t mValue;
    };
    inline std::ostream& operator << (std::ostream& out, const Count& c) { out << "Count(" << std::size_t(c) << ')'; return out; }

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif
    class ArgBase
    {
     public:
        virtual inline ~ArgBase() {}
        ArgBase() = default;
        ArgBase(const ArgBase&) = default;
        virtual const std::type_info& arg_type() const = 0;
    };
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

    class Help
    {
     public:
        inline Help() {}
        template <typename Text> explicit inline Help(Text aMessage) : mMessage(aMessage) {}
        inline bool has_message() const { return !mMessage.empty(); }
        friend inline std::ostream& operator << (std::ostream& out, const Help& aHelp) { return out << aHelp.mMessage; }
     private:
        std::string mMessage;
    };

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
    template <typename ArgT> class Arg : public ArgBase
    {
     public:
        inline Arg(char aShortName, std::string aLongName, const ArgT& aDefault, const Help& aHelp)
            : mShortName(aShortName), mLongName(aLongName), mValue(aDefault), mHelp(aHelp) {}
        inline Arg(char aShortName, const ArgT& aDefault, const Help& aHelp)
            : mShortName(aShortName), mValue(aDefault), mHelp(aHelp) {}
        inline Arg(std::string aLongName, const ArgT& aDefault, const Help& aHelp)
            : mShortName(0), mLongName(aLongName), mValue(aDefault), mHelp(aHelp) {}

          // returns if value was consumed
        template <typename NameT> std::pair<bool, bool> match_consume(NameT aName, const char* aValue)
            {
                auto r = std::make_pair(match(aName), false);
                r.second  = r.first && consume(aValue);
                return r;
            }

        inline bool match(std::string aName) const
            {
                return aName == mLongName;
            }

        inline bool match(char aName) const
            {
                return aName == mShortName;
            }

          // inline std::string name() const { return mLongName; }
        inline const ArgT& value() const { return mValue; }

        friend inline std::ostream& operator << (std::ostream& out, const Arg<ArgT>& aArg)
            {
                if (aArg.mLongName.empty())
                    out << aArg.mShortName;
                else
                    out << aArg.mLongName;
                out << '=' << aArg.mValue;
                return out;
            }

        inline std::string help() const
            {
                std::string at = arg_type_s();
                std::stringstream out;
                if (mShortName) {
                    out << '-' << mShortName << ' ';
                    if (!at.empty())
                        out << '<' << at << "> ";
                }
                if (!mLongName.empty()) {
                    out << "--" << mLongName;
                    if (!at.empty())
                        out << "=<" << at << '>';
                    out << ' ';
                }
                if (mHelp.has_message())
                    out << " -- " << mHelp;
                return out.str();
            }

        virtual const std::type_info& arg_type() const { return typeid(ArgT); }
        virtual std::string arg_type_s() const { return typeid(ArgT).name(); }

     private:
        char mShortName;
        std::string mLongName;
        ArgT mValue;
        Help mHelp;

        inline bool consume(const char* aValue)
            {
                if (aValue == nullptr)
                    throw CommandLineError(std::string("Switch --") + mLongName + " requires a parameter, but end of command line arguments reached");
                assign(aValue);
                return true;

            }

        inline void assign(const char* aValue)
            {
                mValue = aValue;
            }
    };
#pragma GCC diagnostic pop

    template<> inline bool Arg<bool>::consume(const char*)
    {
        mValue = !mValue;
        return false;               // just inverts value
    }

    template<> inline void Arg<bool>::assign(const char*)
    {
          // nothing to assign
    }

    template<> inline bool Arg<Count>::consume(const char*)
    {
        ++mValue;
        return false;
    }

    template<> inline void Arg<Count>::assign(const char*)
    {
          // nothing to assign
    }

    template<> inline bool Arg<PrintHelp>::consume(const char*)
    {
        throw PrintHelp();
    }

    template<> inline void Arg<PrintHelp>::assign(const char*)
    {
          // nothing to assign
    }

    template <typename R, typename FF, typename S> inline void _extract_from_string(const char* aValue, FF F, S type, R& aResult)
    {
        char* end;
        const R result = static_cast<R>(F(aValue, &end));
        if (end == aValue || *end != 0)
            throw CommandLineError(std::string("Cannot parse \"") + aValue + "\" as " + type);
        aResult = result;
    }

    template<> inline std::string Arg<bool>::arg_type_s() const { return std::string(); }
    template<> inline std::string Arg<Count>::arg_type_s() const { return std::string(); }
    template<> inline std::string Arg<PrintHelp>::arg_type_s() const { return std::string(); }
    template<> inline std::string Arg<std::string>::arg_type_s() const { return "string"; }
    template<> inline std::string Arg<int>::arg_type_s() const { return "int"; }
    template<> inline std::string Arg<unsigned>::arg_type_s() const { return "unsigned"; }
    template<> inline std::string Arg<long>::arg_type_s() const { return "long"; }
    template<> inline std::string Arg<unsigned long>::arg_type_s() const { return "unsigned long"; }
    template<> inline std::string Arg<long long>::arg_type_s() const { return "long long"; }
    template<> inline std::string Arg<float>::arg_type_s() const { return "float"; }
    template<> inline std::string Arg<double>::arg_type_s() const { return "double"; }
    template<> inline std::string Arg<long double>::arg_type_s() const { return "long double"; }

    template<> inline void Arg<int>::assign(const char* aValue)  { _extract_from_string(aValue, std::bind(std::strtol, std::placeholders::_1, std::placeholders::_2, 0), arg_type_s(), mValue); }
    template<> inline void Arg<unsigned>::assign(const char* aValue)  { _extract_from_string(aValue, std::bind(std::strtoul, std::placeholders::_1, std::placeholders::_2, 0), arg_type_s(), mValue); }
    template<> inline void Arg<long>::assign(const char* aValue) { _extract_from_string(aValue, std::bind(std::strtol, std::placeholders::_1, std::placeholders::_2, 0), arg_type_s(), mValue); }
    template<> inline void Arg<unsigned long>::assign(const char* aValue) { _extract_from_string(aValue, std::bind(std::strtoul, std::placeholders::_1, std::placeholders::_2, 0), arg_type_s(), mValue); }
    template<> inline void Arg<long long>::assign(const char* aValue) { _extract_from_string(aValue, std::bind(std::strtoll, std::placeholders::_1, std::placeholders::_2, 0), arg_type_s(), mValue); }
    template<> inline void Arg<float>::assign(const char* aValue)  { _extract_from_string(aValue, std::strtof, arg_type_s(), mValue); }
    template<> inline void Arg<double>::assign(const char* aValue)  { _extract_from_string(aValue, std::strtod, arg_type_s(), mValue); }
    template<> inline void Arg<long double>::assign(const char* aValue)  { _extract_from_string(aValue, std::strtold, arg_type_s(), mValue); }

// ----------------------------------------------------------------------

    template <class ... Args> class CommandLineArguments : public std::tuple<Args...>
    {
     public:
        inline CommandLineArguments(const Args&... a)
            : std::tuple<Args...>(std::forward_as_tuple(a...)), mMinArgs(0), mMaxArgs(std::size_t(-1)) {}

        inline void min_max(std::size_t aMinArgs, std::size_t aMaxArgs = std::size_t(-1))
            {
                mMinArgs = aMinArgs;
                mMaxArgs = aMaxArgs;
            }

        void parse(int argc, const char *argv[])
            {
                mProgramName = argv[0];
                try {
                    for (int arg_no = 1; arg_no < argc; ++arg_no) {
                        if (argv[arg_no][0] == '-' && argv[arg_no][1] != 0) { // single - is not an option, it is stdin or stdout
                            if (argv[arg_no][1] == '-') {
                                  // support for long option and its value separated by =
                                const char* eq_sign = std::strchr(argv[arg_no] + 2, '=');
                                const char* arg_arg;
                                std::string name;
                                if (eq_sign == nullptr) {
                                    arg_arg = (arg_no + 1) < argc ? argv[arg_no + 1] : nullptr;
                                    name = argv[arg_no] + 2;
                                }
                                else {
                                    arg_arg = eq_sign + 1;
                                    name.assign(argv[arg_no] + 2, eq_sign);
                                }
                                const bool advance_arg_no_on_consumption = eq_sign == nullptr;
                                const auto found_consumed = set_arg(name, arg_arg, std::index_sequence_for<Args...>{});
                                if (!found_consumed.first)
                                    throw CommandLineError(std::string("Unrecognized long option ") + argv[arg_no]);
                                if (advance_arg_no_on_consumption && found_consumed.second)
                                    ++arg_no;
                            }
                            else {
                                for (auto arg_pos = 1; argv[arg_no][arg_pos] != 0; ++arg_pos) {
                                    auto arg_arg = argv[arg_no][arg_pos + 1] != 0 ? (argv[arg_no] + arg_pos + 1) : ((arg_no + 1) < argc ? argv[arg_no + 1] : nullptr);
                                    const auto found_consumed = set_arg(argv[arg_no][arg_pos], arg_arg, std::index_sequence_for<Args...>{});
                                    if (!found_consumed.first)
                                        throw CommandLineError(std::string("Unrecognized short option -") + std::string(1, argv[arg_no][arg_pos]));
                                    if (found_consumed.second) {
                                        if (arg_arg == argv[arg_no + 1])
                                            ++arg_no;
                                        break;
                                    }
                                }
                            }
                        }
                        else {
                            mArgs.push_back(argv[arg_no]);
                        }
                    }
                    if (mArgs.size() < mMinArgs)
                        throw CommandLineError("Too few arguments provided");
                    if (mArgs.size() > mMaxArgs)
                        throw CommandLineError("Too many arguments provided");
                }
                catch (PrintHelp& help) {
                    print_help(std::cerr, &help);
                    exit(1);
                }
            }

        inline void print_help(std::ostream& out, const PrintHelp* aHelp = nullptr)
            {
                if (aHelp == nullptr) {
                    try {
                        aHelp = &get<PrintHelp>("help");
                    }
                    catch (CommandLineError&) {
                        aHelp = new PrintHelp();
                    }
                }
                out << aHelp->message(mProgramName) << std::endl << "Options:" << std::endl;
                help_helper(out, std::index_sequence_for<Args...>{});
            }

        template <typename ArgT, typename NameT> inline const ArgT& get(NameT aName) const
            {
                return get_helper<ArgT>(aName, std::index_sequence_for<Args...>{});
            }

        void report(std::ostream& out) const
            {
                report_helper(out, std::index_sequence_for<Args...>{}, " ");
                out << std::endl;
                out << "ARGS (" << mArgs.size() << "):";
                for (auto arg: mArgs)
                    out << ' ' << arg;
            }

        inline std::string arg(std::size_t aArgNo) const
            {
                return mArgs[aArgNo];
            }

     private:
        std::string mProgramName;
        std::vector<std::string> mArgs;
        std::size_t mMinArgs, mMaxArgs;

          // returns pair <match found, next argv was consumed>
        template <typename NameT, std::size_t ... Inds> inline std::pair<bool, bool> set_arg(NameT aName, const char* aNextArgv, std::index_sequence<Inds...>)
            {
                const std::pair<bool, bool> consumed[] = {std::get<Inds>(*this).match_consume(aName, aNextArgv) ...};
                const auto found = std::find_if(std::begin(consumed), std::end(consumed), [](auto v) { return v.first; });
                return found != std::end(consumed) ? *found : std::make_pair(false, false);
            }

        template <typename ArgT, typename NameT, std::size_t ... Inds> const ArgT& get_helper(NameT aName, std::index_sequence<Inds...>) const
            {
                const ArgBase* values[] = {(std::get<Inds>(*this).match(aName) ? &std::get<Inds>(*this) : nullptr) ...};
                for (auto p: values) {
                    if (p != nullptr) {
                        auto arg = dynamic_cast<const Arg<ArgT>*>(p);
                        if (arg == nullptr)
                            throw CommandLineError(std::string("Invalid type of option ") + aName + " requested using get(): " + typeid(ArgT).name() + ", real type is " + p->arg_type().name());
                        return arg->value();
                    }
                }
                throw CommandLineError(std::string("Cannot find option ") + aName);
            }

        template <std::size_t ... Inds> inline void report_helper(std::ostream& out, std::index_sequence<Inds...>, const char* aSeparator) const
            {
                int unused[] = {(out << (Inds == 0 ? "" : aSeparator) << std::get<Inds>(*this), 0) ...};
                (void)unused;
            }

        template <std::size_t ... Inds> inline void help_helper(std::ostream& out, std::index_sequence<Inds...>) const
            {
                int unused[] = {(out << "  " << std::get<Inds>(*this).help() << std::endl, 0) ...};
                (void)unused;
            }

    };

    template <class ... Args> std::unique_ptr<CommandLineArguments<Args...>> make_command_line_arguments(const Args& ... args)
    {
        return std::unique_ptr<CommandLineArguments<Args...>>(new CommandLineArguments<Args...>(args...));
    }

// ----------------------------------------------------------------------

} // namespace command_line_arguments

// ----------------------------------------------------------------------
