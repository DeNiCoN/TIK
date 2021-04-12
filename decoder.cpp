#include <boost/program_options.hpp>
#include <iostream>
#include <filesystem>
#include "tik.hpp"
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
        ("fname", po::value<std::string>()->required(), "Input file to decode");
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

    fs::path inputFile(vm["fname"].as<std::string>());
    if (!fs::exists(inputFile))
    {
        std::cerr << "File " << inputFile.string() << " do not exists\n";
        return 1;
    }

    fs::path out_path(inputFile);

    tik::decode(inputFile, out_path.extension() == ".hc" ?
                         out_path.stem() : out_path.concat(".decoded"));

    return 0;
}
