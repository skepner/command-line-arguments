#include "command-line-arguments.hh"

// ----------------------------------------------------------------------

void regression_test();
void test(int argc, const char** argv, const char* expected_report);

// ----------------------------------------------------------------------

int main(int argc, const char *argv[])
{
    using command_line_arguments::CommandLineArguments;
    using command_line_arguments::Arg;
    using command_line_arguments::Count;
    using command_line_arguments::PrintHelp;

    auto cl = make_command_line_arguments(
        Arg<bool>("test", false, "run kinda regression test"),
        Arg<std::string>("str-long", "str long help"),
        Arg<std::string>("str-long-default", "long-default", "str-long-default help"),
        Arg<std::string>('z', "zzz", std::string(), "zzz help"),
        Arg<std::string>('s', "str short help"),
        Arg<int>('i', "int", "int help"),
        Arg<int>("ii", 666, "ii help"),
        Arg<double>("dd", 1/3.0, "dd help"),
        Arg<bool>('t', "true", false, "true help"),
        Arg<bool>('f', "false", true, "false help"),
        Arg<Count>('v', "verbose"),
        Arg<PrintHelp>('h', "help", "Usage: {progname} [options]", "print help")  // print help
                                          );
    cl->parse(argc, argv);
    if (cl->get<bool>("test")) {
        regression_test();
    }
    else {
        cl->report(std::cout);
        std::cout << "get str-long: " << cl->get<std::string>("str-long") << std::endl;
        std::cout << "find i: " << cl->get<int>('i') << std::endl;
        std::cout << "find v: " << cl->get<Count>('v') << std::endl;
        std::cout << "find v: " << int(cl->get<Count>('v')) << std::endl;
    }

    return 0;
}

// ----------------------------------------------------------------------

void regression_test()
{
    const char* argv1[] = {"progname"};
    test(std::end(argv1) - std::begin(argv1), argv1, "aa= bb=cc ee= g= int=0 ii=666 dd=0.333333 true=0 false=1 help=\nARGS (0):");
    const char* argv2[] = {"progname", "--true", "--aa", "aaaa", "argument1", "--dd=11.11", "argument2", "-g", "ggggg gggg"};
    test(std::end(argv2) - std::begin(argv2), argv2, "aa=aaaa bb=cc ee= g=ggggg gggg int=0 ii=666 dd=11.11 true=1 false=1 help=\nARGS (2): argument1 argument2");
}

// ----------------------------------------------------------------------

void test(int argc, const char** argv, const char* expected_report)
{
    using command_line_arguments::CommandLineArguments;
    using command_line_arguments::Arg;
    using command_line_arguments::Count;
    using command_line_arguments::PrintHelp;

    auto cl = make_command_line_arguments(
        Arg<std::string>("aa"),
        Arg<std::string>("bb", "cc", "dd"),
        Arg<std::string>('e', "ee", std::string(), "ff"),
        Arg<std::string>('g', "hh"),
        Arg<int>('i', "int", "int"),
        Arg<int>("ii", 666, "ii"),
        Arg<double>("dd", 1/3.0, "dd"),
        Arg<bool>('t', "true", false, "true help"),
        Arg<bool>('f', "false", true, "false help"),
        Arg<PrintHelp>('h', "help", "print help")  // print help
                                          );
    cl->parse(argc, argv);
    std::stringstream s;
    cl->report(s);
    if (s.str() != expected_report) {
        std::cerr << s.str() << std::endl;
        throw std::runtime_error(std::string("test FAILED: ") + expected_report);
    }
}

// ----------------------------------------------------------------------
