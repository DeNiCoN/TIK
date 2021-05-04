#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iterator>
#include "tik.hpp"
#include "utils.hpp"
#include "hamming.hpp"

enum class ExecutionMode
{
    ENCODE,
    DECODE,
    FLIP,
    CHECK
};

std::istream& operator>>(std::istream& in, ExecutionMode& mode)
{
    std::string str;
    in >> str;
    if (str == "encode")
        mode = ExecutionMode::ENCODE;
    else if (str == "decode")
        mode = ExecutionMode::DECODE;
    else if (str == "flip")
        mode = ExecutionMode::FLIP;
    else if (str == "check")
        mode = ExecutionMode::CHECK;
    else
        in.setstate(in.rdstate() | std::ios::failbit);
    return in;
}

int main(int argc, char** argv)
{
    namespace po = boost::program_options;
    namespace fs = std::filesystem;

    ExecutionMode mode;

    po::options_description visible;
    visible.add_options()
        ("help", "Produce help message")
        ("fname", po::value<std::string>()->required(), "Input file to encode")
        ("mode", po::value(&mode)->required(),
         "Mode of executable [encode, decode, flip, check]")
        ("subargs", po::value<std::vector<std::string>>(), "Arguments for command");

    po::positional_options_description positional;
    positional.add("fname", 1);
    positional.add("mode", 1);
    positional.add("subargs", -1);

    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(visible)
        .positional(positional).allow_unregistered().run();
    po::store(parsed, vm);

    if (vm.count("help")) {
        std::cout << visible << "\n";
        return 1;
    }
    po::notify(vm);

    fs::path input_file(vm["fname"].as<std::string>());

    if (!fs::exists(input_file))
    {
        std::cerr << "File " << input_file.string() << " do not exists\n";
        return 1;
    }

    std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
    opts.erase(opts.begin(), opts.begin() + 2);

    switch (mode)
    {
    case ExecutionMode::ENCODE:
    {
        po::options_description encode_options("encode options");
        encode_options.add_options()
            ("suffix", po::value<std::string>()->default_value(".hamming"));
        po::store(po::command_line_parser(opts).options(encode_options).run(), vm);

        if (vm.count("help"))
        {
            std::cout << encode_options << "\n";
            return 1;
        }
        vm.notify();

        fs::path out_path(input_file);
        const std::string suffix = vm["suffix"].as<std::string>();
        if (vm.count("no-extended"))
            tik::hamming::encode_not_extended(input_file, out_path.concat(suffix));
        else
            tik::hamming::encode(input_file, out_path.concat(suffix));

        break;
    }
    case ExecutionMode::DECODE:
    {
        po::options_description decode_options("decode options");
        decode_options.add_options()
            ("suffix", po::value<std::string>()->default_value(".decoded"));
        po::store(po::command_line_parser(opts).options(decode_options).run(), vm);

        if (vm.count("help"))
        {
            std::cout << decode_options << "\n";
            return 1;
        }
        vm.notify();

        fs::path out_path(input_file);
        const std::string suffix = vm["suffix"].as<std::string>();
        tik::hamming::decode(input_file, out_path.concat(suffix));
    }
    case ExecutionMode::FLIP:
    {
        po::options_description flip_options("flip options");
        std::vector<size_t> positions;
        flip_options.add_options()
            ("out", po::value<std::string>(), "Output file")
            ("pos", po::value(&positions), "Positions to flip bit");
        po::positional_options_description positional;
        positional.add("pos", -1);
        po::store(po::command_line_parser(opts).options(flip_options).run(), vm);

        if (vm.count("help"))
        {
            std::cout << flip_options << "\n";
            return 1;
        }
        vm.notify();

        fs::path out_path = vm.count("out") ? fs::path(vm["out"].as<std::string>()) : input_file;

        std::ifstream stream(input_file, std::ios::binary);
        std::istreambuf_iterator<char> begin(stream);
        std::istreambuf_iterator<char> end;
        std::vector<char> buffer(begin, end);

        for (const std::size_t pos : positions)
        {
            tik::utils::flip_bit(buffer.begin(), pos);
        }

        break;
    }
    case ExecutionMode::CHECK:
    {
        break;
    }
    }

    return 0;
}
