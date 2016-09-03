#ifndef __AAALGO_BFDFS__
#define __AAALGO_BFDFS__

#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>

// BFDFS blob format
// uint32_t file_count  # number of files in the blob
// uint32_t dir_offset  # offset of file directory
// ...
// ...
// ...
// ------- dir -------
// name_offset, name_length, content_offset, content_length
// name_offset, name_length, content_offset, content_length
// ...

namespace bfdfs {
    using std::vector;
    using std::ofstream;
    using std::string;
    using std::unordered_map;

    static unsigned constexpr DIR_ALIGN = 32;

    struct __attribute__((__packed__)) DirEntry {
        uint32_t name_offset;
        uint32_t name_length;
        uint32_t content_offset;
        uint32_t content_length;
    };

    class BlobWriter {
        ofstream os;
        vector<DirEntry> dir;
        uint32_t file_count;
        uint32_t dir_offset;
    public:
        BlobWriter (string const &path)
            : os(path, std::ios::binary),
            file_count(0),
            dir_offset(0)
        {
            os.write(reinterpret_cast<char const *>(&file_count), sizeof(file_count));
            os.write(reinterpret_cast<char const *>(&dir_offset), sizeof(dir_offset));
        }

        void append (std::string const &path, std::istream &str) {
            DirEntry e;
            e.name_offset = os.tellp();
            e.name_length = path.size();
            os.write(&path[0], path.size());
            e.content_offset = os.tellp();
            os << str.rdbuf();
            e.content_length = uint32_t(os.tellp()) - e.content_offset;
            dir.push_back(e);
        }

        BlobWriter () {
            uint32_t p = os.tellp();
            if (p != os.tellp()) throw "Data too big";
            dir_offset = (p + DIR_ALIGN -1) / DIR_ALIGN * DIR_ALIGN;
            file_count = dir.size();
            os.seekp(dir_offset, std::ios::beg);
            os.write(reinterpret_cast<char const *>(&dir[0]), dir.size() * sizeof(dir[0]));
            os.seekp(0, std::ios::beg);
            os.write(reinterpret_cast<char const *>(&file_count), sizeof(file_count));
            os.write(reinterpret_cast<char const *>(&dir_offset), sizeof(dir_offset));
        }
    };

    class Root: unordered_map<string, std::pair<char const *, char const *>> {
    public:
        Root (char const *base) {
            uint32_t count = *reinterpret_cast<uint32_t const *>(base);
            uint32_t dir_off = *reinterpret_cast<uint32_t const *>(base + sizeof(count));
            DirEntry const *e = reinterpret_cast<DirEntry const *>(base + dir_off);
            for (uint32_t i = 0; i < count; ++i) {
                string name(base + e->name_offset, base + e->name_offset + e->name_length);
                char const *first = base + e->content_offset;
                char const *second = first + e->content_length;
                (*this)[name] = std::make_pair(first, second);
                e++;
            }
        }
    };
}

#endif
