#ifndef __AAALGO_BFDFS__
#define __AAALGO_BFDFS__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include <boost/uuid/sha1.hpp>

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
            if (!os) {
                std::cerr << "Failed to open and write " << path << std::endl;
                throw std::runtime_error("fail to create file: " + path);
            }
        }

        void append (std::string const &path, std::istream &str) {
            char zero = 0;
            DirEntry e;
            e.name_offset = os.tellp();
            e.name_length = path.size();
            os.write(&path[0], path.size());
            os.write(&zero, 1);
            e.content_offset = os.tellp();
            std::copy(std::istreambuf_iterator<char>(str),
                      std::istreambuf_iterator<char>(),
                      std::ostreambuf_iterator<char>(os));
            e.content_length = uint32_t(os.tellp()) - e.content_offset;
            os.write(&zero, 1);
            dir.push_back(e);
        }

        ~BlobWriter () {
            uint32_t p = os.tellp();
            if (p != os.tellp()) {
                throw std::runtime_error("file size not consistent");
            }
            dir_offset = (p + DIR_ALIGN -1) / DIR_ALIGN * DIR_ALIGN;
            file_count = dir.size();
            os.seekp(dir_offset, std::ios::beg);
            os.write(reinterpret_cast<char const *>(&dir[0]), dir.size() * sizeof(dir[0]));
            os.seekp(0, std::ios::beg);
            os.write(reinterpret_cast<char const *>(&file_count), sizeof(file_count));
            os.write(reinterpret_cast<char const *>(&dir_offset), sizeof(dir_offset));
        }
    };

    struct Page {
        char const *begin, *end;
        string checksum;
    };

    class Loader: public unordered_map<string, Page> {

		static void sha1sum (char const *data, unsigned length, std::string *checksum) {
			uint32_t digest[5];
			boost::uuids::detail::sha1 sha1;
			sha1.process_block(data, data+length);
			sha1.get_digest(digest);
			static char const digits[] = "0123456789abcdef";
			checksum->clear();
			for(uint32_t c: digest) {
				checksum->push_back(digits[(c >> 28) & 0xF]);
				checksum->push_back(digits[(c >> 24) & 0xF]);
				checksum->push_back(digits[(c >> 20) & 0xF]);
				checksum->push_back(digits[(c >> 16) & 0xF]);
				checksum->push_back(digits[(c >> 12) & 0xF]);
				checksum->push_back(digits[(c >> 8) & 0xF]);
				checksum->push_back(digits[(c >> 4) & 0xF]);
				checksum->push_back(digits[c & 0xF]);
			}
		}
        
    public:
        Loader (char const *base) {
            uint32_t count = *reinterpret_cast<uint32_t const *>(base);
            uint32_t dir_off = *reinterpret_cast<uint32_t const *>(base + sizeof(count));
            //std::cout << count << '\t' << dir_off << std::endl;
            DirEntry const *e = reinterpret_cast<DirEntry const *>(base + dir_off);
            for (uint32_t i = 0; i < count; ++i) {
                string name(base + e->name_offset, base + e->name_offset + e->name_length);
                //std::cout << name << std::endl;
                Page page;
                page.begin = base + e->content_offset;
                page.end = page.begin + e->content_length;
				sha1sum(page.begin, page.end - page.begin, &page.checksum);
                (*this)[name] = page;
                e++;
            }
        }

        string get (string const &key) const {
            auto const &p = at(key);
            return string(p.begin, p.end);
        }

    };
}

#endif
