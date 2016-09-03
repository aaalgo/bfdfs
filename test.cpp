#include <iostream>
#include "bfdfs.h"

using namespace std;

extern char _binary_bfdfs_blob_start;

int main () {
    bfdfs::Root root(&_binary_bfdfs_blob_start);
    for (auto p: root) {
        cout << p.first << '\t' << (p.second.second - p.second.first) << endl;
    }
}
