#include "httpRequest.h"

namespace search {

    using constants::httpVersion;
    using constants::endl;
    using constants::encoding;
    using constants::hostString;
    using constants::connClose;
    using constants::userAgents;
    using constants::robotsTxtString;

    // parse a well formed url and get the stuff within
    // URI = scheme:[//authority]path[?query][#fragment]
    // authority = [userinfo@]host[:port]
    // uses the RFC 3986 regex suggestion for URL parsing
    // Using GCC 8.2.0 on Linux the return value is moved
    // not copied. Therefore it is faster than the heap
    // based version. 
    // Bad urls will copy the empty request but will not
    // run a bunch of std::string constructors.
    HTTPRequest::HTTPRequest(std::string url) {
        static std::regex r(
                R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
                std::regex::extended
        );
        std::smatch result;
        if (std::regex_match(url, result, r)) {
            scheme = result[2];
            host = result[4];
            path = result[5];
            query = result[7];
            fragment = result[9];
        }
    }

    // TODO: do all the string manipulation on the stack
    // maximum url length apache supports is 8KB. Our stacks are 2MB.
    std::string HTTPRequest::filename() const {
        std::string slashesRemoved;
        std::string rv;
        if (path == robotsTxtString) {
            rv = "robots/" + host;
        } else {
            slashesRemoved = host + path;
            for (char &ch : slashesRemoved) {
                if (ch == '/') {
                    ch = '_';
                }
            }
            rv = "pages/" + slashesRemoved;
        }
        return rv;
    }

    std::string HTTPRequest::requestString() const {
        std::stringstream ss;
        ss << method << ' ' << '/' << path << ' ' << httpVersion << endl;
        ss << hostString << ' ' << host << endl;
        ss << userAgents << endl;
        ss << encoding << endl;
        ss << connClose << endl;
        return ss.str();
    }

    void HTTPRequest::print() const {
        std::stringstream ss;
        ss << "{\n";
        ss << "\t" << "method: " << method << '\n';
        ss << "\t" << "host: " << host << '\n';
        ss << "\t" << "path: " << path << '\n';
        ss << "\t" << "query: " << query << '\n';
        ss << "\t" << "fragment: " << fragment << '\n';
        ss << "\t" << "headers: " << headers << '\n';
        ss << "\t" << "scheme: " << scheme << '\n';
        ss << "\t" << "port: " << port << '\n';
        ss << "}\n";
        std::cout << ss.str() << std::flush;
    }

    bool HTTPRequest::robots() const {
        return path == robotsTxtString;
    }

    std::string HTTPRequest::uri() const {
        // Pseudocode from RFC 2396
        // result = ""
        //  if scheme is defined then
        //      append scheme to result
        //      append ":" to result
        //  if authority is defined then
        //      append "//" to result
        //      append authority to result
        //  append path to result
        //  if query is defined then
        //      append "?" to result
        //      append query to result
        //  if fragment is defined then
        //      append "#" to result
        //      append fragment to result
        //  return result
        std::string result = "";
        if (scheme != "") {
            result += scheme;
            result += ":";
        }
        result += host;
        result += path;
        if (query != "") {
            result += "?";
            result += "query";
        }
        if (fragment != "") {
            result += "#";
            result += fragment;
        }
        return result;
    }
}
