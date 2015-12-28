# command-line-arguments
C++14 include-only library to parse command line arguments.

## Usage

    #include "command-line-arguments.hh"

    int main(int argc, const char *argv[])
    {
        using command_line_arguments::CommandLineArguments;
        using command_line_arguments::Arg;
        using command_line_arguments::Count;
        using command_line_arguments::PrintHelp;
        using command_line_arguments::CommandLineError;

        auto cl = make_command_line_arguments(
            // Switches requiring string arguments
            Arg<std::string>("str-long", "help message"), // long switch, empty default
            Arg<std::string>("str-long-default", "default value", "help message"), // long switch, specified default
            Arg<std::string>('z', "zzz", std::string(), "help message"), // short and long switches argument, specified default
            Arg<std::string>('s', "help message"), // short switch, empty default

            // Switches requiring numeric arguments
            Arg<int>('i', "int", "help message"), // short switch, requires int argument, default is 0 (aka int())
            Arg<int>("ii", 666, "help message"), // long switch, requires int argument, default is 666
            Arg<double>("dd", 1/3.0, "help message"), // long switch, requires double argument, default is 0.333...

            // Switches without arguments, toggle boolean value
            Arg<bool>('t', "true", false, "help message"),
            Arg<bool>('f', "false", true, "help message"),

            // Switch without argument, counts occurences
            Arg<Count>('v', "verbose"),

            // Special switch to print help message and exit with code 1
            Arg<PrintHelp>('h', "help", "prints this help text")  // print help
        );
        cl->parse(argc, argv);
        cl->report(std::cout); // reports resulting (after parsing command line) values

        // get individual values
        // providing wrong template argument for get<> leads to CommandLineError thrown
        std::cout << "get str-long: " << cl->get<std::string>("str-long") << std::endl;
        std::cout << "find i: " << cl->get<int>('i') << std::endl;
        std::cout << "find v: " << cl->get<Count>('v') << std::endl;
        // Count can be converted to size_t or int
        std::cout << "find v: " << int(cl->get<Count>('v')) << std::endl;
        return 0;
    }
