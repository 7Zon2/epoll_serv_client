#include "../include/epoll_client.hpp"


int main()
{
    try
    {
        epoll_client s;
        s.connect();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}