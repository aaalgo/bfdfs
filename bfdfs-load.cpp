#include <fnmatch.h>
#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp> 
#include <boost/filesystem/fstream.hpp> 
#include <boost/program_options.hpp>
#include "bfdfs.h"

using namespace std;
namespace fs = boost::filesystem; 

int main(int argc, char const* argv[]) {
    vector<string> excludes;
    fs::path root;
    string output;
    string target_O;
    string target_B;
    string name;
    string objcopy;

    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message.")
        ("output", po::value(&output), "")
        ("root", po::value(&root), "")
        (",O", po::value(&target_O)->default_value("elf64-x86-64"), "")
        (",B", po::value(&target_B)->default_value("i386:x86-64"), "")
        ("name,n", po::value(&name)->default_value("bfdfs_blob"), "")
        ("check", "")
        ("exclude,x", po::value(&excludes), "")
        ("objcopy", po::value(&objcopy)->default_value("objcopy"), "")
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
    fs::path temp(name);
    {
        bfdfs::BlobWriter writer(temp.native());
        for (fs::recursive_directory_iterator dir_end, dir(root); dir != dir_end; ++dir) {
            fs::path path(*dir);
            if (!fs::is_regular_file(path)) continue;
            string bn = path.filename().native();
            bool good = true;
            for (auto const &v: excludes) {
                if (fnmatch(v.c_str(), bn.c_str(), 0) == 0) {
                    good = false;
                    break;
                }
            }
            if (!good) continue;
            string full = path.native();
            if (full.find(prefix) != 0) throw 0;
            size_t o = prefix.size();
            //if (full[o] == '/') o++; 
            string name = full.substr(o);
            fs::ifstream is(path, ios::binary);
            writer.append(name, is);
        }
    }
    ostringstream oss;
    oss << objcopy << " -I binary -O " << target_O << " -B " << target_B << " " << temp << " " << output;
    std::system(oss.str().c_str());
    //fs::remove(temp);
    return 0;
}

