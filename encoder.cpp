#include <boost/program_options.hpp>
#include <iostream>
#include <filesystem>


int main(int argc, char** argv)
{
    namespace po = boost::program_options;
    namespace fs = std::filesystem;
    po::options_description visible;

    visible.add_options()
        ("help", "Produce help message");

    po::options_description hidden;
    hidden.add_options()
        ("fname", po::value<std::string>()->required(), "Input file to encode");
    po::positional_options_description positional;
    positional.add("fname", 1);

    po::options_description all;
    po::variables_map vm;
    all.add(visible).add(hidden);
    po::store(po::command_line_parser(argc, argv).options(all)
              .positional(positional).run(),
              vm);

    if (vm.count("help")) {
        std::cout << visible << "\n";
        return 1;
    }
    po::notify(vm);

    fs::path inputFile(vm.as<std::string>("fname"));
    if (!inputFile.exists())
    {
        std::cerr << "File " << inputFile.str() << " do not exists\n";
        return 1;
    }

    return 0;
}
