#include <boost/program_options.hpp>
#include <iostream>
#include <filesystem>
#include "tik.hpp"


int main(int argc, char** argv)
{
    namespace po = boost::program_options;
    namespace fs = std::filesystem;
    po::options_description visible;

    visible.add_options()
        ("help", "Produce help message")
        ("shannon-fano", po::bool_switch(), "Encode using Shannon-Fano Algorithm");

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

    fs::path inputFile(vm["fname"].as<std::string>());
    if (!fs::exists(inputFile))
    {
        std::cerr << "File " << inputFile.string() << " do not exists\n";
        return 1;
    }

    if (vm.count("shannon-fano"))
    {
        tik::encode_shannon_fano(inputFile, fs::path(inputFile).concat(".encoded"));
    }
    else
    {
        tik::encode_shannon_fano(inputFile, fs::path(inputFile).concat(".encoded"));
    }

    return 0;
}
