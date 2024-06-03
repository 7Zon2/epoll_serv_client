#include "../include/epoll_server.hpp"

int main()
{
    try
    {
        epoll_server s{};
        s.prepare();
        s.connect();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}