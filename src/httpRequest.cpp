#include "httpRequest.h"
#include <string>
#include <sstream>

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
    // run a bunch of std::string constructors


    static std::string urlDecoder(const std::string& url) {
        std::ostringstream decoded;
        std::string hexBuffer = "00"; 
        for (size_t i = 0; i < url.size(); ++i) {
            if (url[i] == '%') {
                if (url.size() < i + 3) {
                    return url; // fail safeish
                } else {
                    try { // std::stoi can throw
                        hexBuffer[0] = url[i + 1];
                        hexBuffer[1] = url[i + 2];
                        size_t decodedNum;     // convert the hex string to an ascii int then cast it to a char
                        decoded << static_cast<char>(std::stoi(hexBuffer, &decodedNum, 16));
                        // if we only decoded 1 number then there was an illegal sequence
                        if (decodedNum < hexBuffer.size()) {
                            return url;
                        }
                        // move out pointer forward
                        i += 2;
                    } catch (...) {
                        return url;
                    }
                }
            } else if (url[i] == '+') {
                decoded << ' ';
            } else {
                decoded << url[i];
            }
        }
        return decoded.str();
    }


    HTTPRequest::HTTPRequest(std::string url) {
        url.erase(remove_if(url.begin(), url.end(), isspace), url.end());
        // decode the url before we do anything else
        url = urlDecoder(url);
        // check mailto
        if (url.size() >= 6 && 
            url[0] == 'm' && url[1] == 'a' && url[2] == 'i' &&
            url[3] == 'l' && url[4] == 't' && url[5] == 'o') {
                return;
        }
        if (url.size() >= 5 && url.substr(0, 5) == "data:") {
            return;
        }
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

    // return value will either get optimized out or the compiler
    // will use the move constructor
    std::string HTTPRequest::filename() const {
        std::string rv;
        if (robots()) {
            return "robots/" + host;
        } else {
            std::string slashesRemoved = host + path;
            for (size_t i = 0; i < slashesRemoved.size(); ++i) {
                if (slashesRemoved[i] == '/' || slashesRemoved[i] == '\0') {
                    slashesRemoved[i] = '_';
                }
            }
            return "pages/" + slashesRemoved;
        }
    }

    std::string HTTPRequest::requestString() const {
        std::stringstream ss;
        ss << constants::getMethod << ' ' << path << ' ' << constants::httpVersion << endl;
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
        return path == constants::robotsTxtString || path == constants::robotsTxtString2;
    }

    // returns whether the extension of the file is one we're looking for
    // we're specifically trying to avoid javascript, css, and images
    // everything else is fair game
    bool HTTPRequest::goodExtension() const {
        for (size_t i = 0; i < NUM_BAD_EXTENSIONS; ++i) {
            const std::string& badExtension = BAD_EXTENSIONS[i];
            if (path.size() > badExtension.size()) {
                // check the last badExtension.size() characters and see if they match, if any of them match completely
                // then return false, otherwise go onto the next one, if none of them match completely, we can return true
                bool fullMatch = true;
                for (size_t i = 0; i < badExtension.size(); ++i) {
                        // align any matching extensions at the end of the up
                    if (path[path.size() - badExtension.size() + i] != badExtension[i]) {
                        fullMatch = false;
                        break;
                    }
                }
                if (fullMatch) {
                    return false;
                }
            }
        } 
        return true;
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
            result += "://";
        }
        result += host;
        if (path != "") {
            result += path;
        } else {
            result += "/";
        }
        if (query != "") {
            result += "?";
            result += "query";
        }
        return result;
    }
}
