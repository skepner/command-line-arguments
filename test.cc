#include "command-line-arguments.hh"

// ----------------------------------------------------------------------

void regression_test();
void test(int argc, const char** argv, const char* expected_report);

// ----------------------------------------------------------------------

int main(int argc, const char *argv[])
{
    using command_line_arguments::CommandLineArguments;
    using command_line_arguments::Help;
    using command_line_arguments::Arg;
    using command_line_arguments::Count;
    using command_line_arguments::PrintHelp;

    auto cl = make_command_line_arguments(
        Arg<bool>("test", false, Help("run kinda regression test")),
        Arg<std::string>("str-long", std::string(), Help("str long help")),
        Arg<std::string>("str-long-default", "long-default", Help("str-long-default help")),
        Arg<std::string>('z', "zzz", std::string(), Help("zzz help")),
        Arg<std::string>('s', std::string(), Help("str short help")),
        Arg<int>('i', "int", int(), Help("int help")),
        Arg<int>("ii", 666, Help("ii help")),
        Arg<double>("dd", 1/3.0, Help("dd help")),
        Arg<bool>('t', "true", false, Help("true help")),
        Arg<bool>('f', "false", true, Help("false help")),
        Arg<Count>('v', "verbose", Count(), Help()),
        Arg<PrintHelp>('h', "help", "Usage: {progname} [options]", Help("print help"))  // print help
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
    test(std::end(argv1) - std::begin(argv1), argv1, "aa= bb=cc ee= g=hh int=0 ii=666 dd=0.333333 true=0 false=1 help=\nARGS (0):");
    const char* argv2[] = {"progname", "--true", "--aa", "aaaa", "argument1", "--dd=11.11", "argument2", "-g", "ggggg gggg"};
    test(std::end(argv2) - std::begin(argv2), argv2, "aa=aaaa bb=cc ee= g=ggggg gggg int=0 ii=666 dd=11.11 true=1 false=1 help=\nARGS (2): argument1 argument2");
}

// ----------------------------------------------------------------------

void test(int argc, const char** argv, const char* expected_report)
{
    using command_line_arguments::CommandLineArguments;
    using command_line_arguments::Help;
    using command_line_arguments::Arg;
    using command_line_arguments::Count;
    using command_line_arguments::PrintHelp;

    auto cl = make_command_line_arguments(
        Arg<std::string>("aa", std::string(), Help()),
        Arg<std::string>("bb", "cc", Help("dd")),
        Arg<std::string>('e', "ee", std::string(), Help("ff")),
        Arg<std::string>('g', "hh", Help()),
        Arg<int>('i', "int", int(), Help("int")),
        Arg<int>("ii", 666, Help("ii")),
        Arg<double>("dd", 1/3.0, Help("dd")),
        Arg<bool>('t', "true", false, Help("true help")),
        Arg<bool>('f', "false", true, Help("false help")),
        Arg<PrintHelp>('h', "help", PrintHelp(), Help())  // print help
                                          );
    cl->parse(argc, argv);
    std::stringstream s;
    cl->report(s);
    if (s.str() != expected_report) {
        std::cerr << s.str() << std::endl;
        std::cerr << expected_report << std::endl;
        throw std::runtime_error(std::string("test FAILED: ") + expected_report);
    }
}

// ----------------------------------------------------------------------
