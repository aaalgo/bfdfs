#include <iostream>
#include <boost/filesystem.hpp> 
#include <boost/filesystem/fstream.hpp> 
#include <boost/program_options.hpp>
#include "bfdfs.h"

using namespace std;
namespace fs = boost::filesystem; 

int main(int argc, char const* argv[]) {
    fs::path root;
    string output;

    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message.")
        ("output", po::value(&output), "")
        ("root", po::value(&root), "")
        ("check", "")
        ;

    po::positional_options_description p;
    p.add("output", 1);
    p.add("root", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
                     options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help") || output.empty() || root.empty()) {
        cout << "Usage:" << endl;
        cout << "\tbfdfs-load [--check] <output> <dir>" << endl;
        cout << desc;
        cout << endl;
        return 0;
    }

    string prefix = root.native();
    bfdfs::BlobWriter writer(output);
    for (fs::recursive_directory_iterator dir_end, dir(root); dir != dir_end; ++dir) {
        fs::path path(*dir);
        if (!fs::is_regular_file(path)) continue;
        string full = path.native();
        if (full.find(prefix) != 0) throw 0;
        size_t o = prefix.size();
        if (full[o] == '/') o++; 
        string name = full.substr(o);
        fs::ifstream is(path, ios::binary);
        writer.append(name, is);
    }

    return 0;
}

