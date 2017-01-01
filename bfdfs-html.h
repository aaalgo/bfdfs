#include <stdexcept>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <boost/boostache/boostache.hpp>
#include <boost/boostache/frontend/stache/grammar_def.hpp> // need to work out header only syntax
#include <boost/boostache/stache.hpp>
#include <boost/boostache/model/helper.hpp>
//#include <served/served.hpp>
#include "bfdfs.h"

extern char _binary_html_static_start;
extern char _binary_html_template_start;

namespace bfdfs {

    namespace boostache = boost::boostache;

    static std::unordered_map<string, string> const EXT_MIME{
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
    };
    static std::string const DEFAULT_MIME("application/octet-stream");

    class HTML {
        Loader statics;
        Loader templates;
        string dynamic_root;


        string read_file (string const &prefix, string const &path) const {
            string p = dynamic_root + prefix + path;
            std::ifstream is(p.c_str());
            if (!is) throw std::runtime_error("cannot open " + p);
            is.seekg(0, std::ios::end);
            string x;
            x.resize(is.tellg());
            is.seekg(0);
            is.read(&x[0], x.size());
            if (!is) throw std::runtime_error("failed to read " + p);
            return x;
        }

        static string const &path2mime (string const &path) {
            do {
                auto p = path.rfind('.');
                if (p == path.npos) break;
                string ext = path.substr(p);
                auto it = EXT_MIME.find(ext);
                if (it == EXT_MIME.end()) break;
                return it->second;
            } while (false);
            return DEFAULT_MIME;
        }
    public:
        HTML (string const &droot = ""):
            statics(&_binary_html_static_start),
            templates(&_binary_html_template_start),
            dynamic_root(droot)
        {
        }

        ~HTML () {
        }

        std::pair<char const *, char const *> getStatic (string const &path) const {
            return statics.at(path);
        }

#ifdef SERVED_HPP   // served
        template <typename Context>
        void render_to_response (served::response &res,
                                 string const &path,
                                 Context const &context) const {
            try {
                string buf;
                std::pair<char const *, char const *> p;
                if (dynamic_root.size()) {
                    buf = read_file("/template",  path);
                    p.first = &buf[0];
                    p.second = p.first + buf.size();
                }
                else {
                    p = templates.at(path);
                }
                res.set_header("Content-Type", path2mime(path));
                auto tmpl = boostache::load_template<boostache::format::stache>(p.first, p.second);
                std::stringstream ss;
                boostache::generate(ss, tmpl, context);
                res << ss.str();
            }
            catch (std::out_of_range const &) {
                served::response::stock_reply(404, res);
            }
        }

        void send_to_response (served::response &res,
                                string const &path) const {
            try {
                string buf;
                if (dynamic_root.size()) {
                    buf = read_file("/static",  path);
                }
                else {
                    buf = statics.get(path);
                }
                res.set_header("Content-Type", path2mime(path));
                res.set_body(buf);
            }
            catch (std::out_of_range const &) {
                served::response::stock_reply(404, res);
            }
        }
#endif
#ifdef SERVER_HTTP_HPP //Simple-Web-Server
        typedef SimpleWeb::Server<SimpleWeb::HTTP>::Response Response;
        template <typename Context>
        void render_to_response (std::shared_ptr<Response> res,
                                 string const &path,
                                 Context const &context) const {
            string buf;
            std::pair<char const *, char const *> p;
            if (dynamic_root.size()) {
                buf = read_file("/template",  path);
                p.first = &buf[0];
                p.second = p.first + buf.size();
            }
            else {
                p = templates.at(path);
            }
            auto tmpl = boostache::load_template<boostache::format::stache>(p.first, p.second);
            std::stringstream ss;
            boostache::generate(ss, tmpl, context);
            string text = ss.str();
            *res << "HTTP/1.1 200 OK\r\n";
            *res << "Content-Type: " << path2mime(path) << "\r\n";
            *res << "Content-Length: " << text.size() << "\r\n\r\n";
            *res << text;
        }

        void send_to_response (std::shared_ptr<Response> res,
                                string const &path) const {
            string text;
            if (dynamic_root.size()) {
                text = read_file("/static",  path);
            }
            else {
                text = statics.get(path);
            }
            *res << "HTTP/1.1 200 OK\r\n";
            *res << "Content-Type: " << path2mime(path) << "\r\n";
            *res << "Content-Length: " << text.size() << "\r\n\r\n";
            *res << text;
        }
#endif
    };


}
