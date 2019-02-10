# Data Structures and Algorithms Necessary

- <u>**Fast**</u> thread safe **set**. Only needs to hold integers. Used to transfer waiting and ready sockets and file descriptors between the HTTP Client and the epoll() handling thread. Start using std::set and STL threading/mutex templates and improve at a later date. 
- **Thread safe in-memory hash table** for storing state of active requests. Start by wrapping std::unordered_map and improve from there. State dies when HTTP response is fully downloaded and all Crawler processing on it is completed. 
- **Thread safe disk based hash table.** Disk based hash table for the DNS cache as well as the index, preventing repeat downloads, as well as storing domain specific info like robots.txt. Can also be used to store HTML files without the file system. Interoperable with the memory-based one. Use mmap(), can share between processes.
- **Hash Functions** - Integers and strings. 
- **URL Parser**. Using std::regex right now with the recommended regex string from RFC but should probably write our own?





