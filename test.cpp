#include <iostream>
#include "bfdfs.h"

using namespace std;

extern char _binary_bfdfs_blob_start;

int main () {
    bfdfs::Loader loader(&_binary_bfdfs_blob_start);
    for (auto p: loader) {
        cout << p.first << '\t' << (p.second.second - p.second.first) << endl;
    }
}
