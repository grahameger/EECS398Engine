#include "constants.h"

namespace constants {
    const std::string getMethod = "GET";
    const std::string endl = "\r\n";
    const std::string httpVersion = "HTTP/1.1";
    const std::string hostString = "Host: ";
    const std::string connClose = "Connection: close" + endl;
    const std::string userAgents = "User-Agent: Ceatles/1.0 (Linux)";
    const std::string encoding = "Accept-Encoding: identity";
    const std::string httpStr = "http";
    const std::string httpsStr = "https";
    const std::string port80 = "80";
    const std::string port443 = "443";
    const std::string robotsTxtString = "/robots.txt";

    const size_t MAX_CONNECTIONS = 1000;
    const size_t RECV_SIZE = 8192;
    const size_t BUFFER_SIZE = RECV_SIZE;
    const size_t NUM_THREADS = 4;
    const uint32_t SLEEP_US = 10000;
    const size_t DEFAULT_FILE_SIZE = 1024000; // 1MiB or 256 pages
    const long int TIMEOUTSECONDS = 5;
    const long int TIMEOUTUSECONDS = 0;
}