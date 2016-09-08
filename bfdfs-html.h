#include <stdexcept>
#include <sstream>
#include <boost/boostache/boostache.hpp>
#include <boost/boostache/frontend/stache/grammar_def.hpp> // need to work out header only syntax
#include <boost/boostache/stache.hpp>
#include <boost/boostache/model/helper.hpp>
#include <served/served.hpp>
#include <magic.h>

#include "bfdfs.h"

extern char _binary_html_static_start;
extern char _binary_html_template_start;

namespace bfdfs {

    namespace boostache = boost::boostache;

    class HTML {
        Loader statics;
        Loader templates;
        magic_t cookie;
    public:
        HTML ():
            statics(&_binary_html_static_start),
            templates(&_binary_html_template_start) {
            cookie = magic_open(MAGIC_MIME_TYPE);
            magic_load(cookie, NULL);
        }

        ~HTML () {
            magic_close(cookie);
        }

        template <typename Context>
        void render_to_response (served::response &res,
                                 string const &path,
                                 Context const &context) const {
            try {
                auto p = templates.at(path);
                char const *mime = magic_buffer(cookie, p.first, p.second - p.first);
                res.set_header("Content-Type", mime);
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
                string buf = statics.get(path);
                char const *mime = magic_buffer(cookie, &buf[0], buf.size());
                res.set_header("Content-Type", mime);
                res.set_body(buf);
            }
            catch (std::out_of_range const &) {
                served::response::stock_reply(404, res);
            }
        }
    };


}