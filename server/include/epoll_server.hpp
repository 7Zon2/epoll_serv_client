#include "../../include/socket_header.hpp"
#include <memory_resource>
#include <unordered_map>

class epoll_server
{
    SOCKET listener;
    SOCKET epl;
    static constexpr size_t number_events=1024;
    addrinfo hints{};
    addrinfo *inf{};
    char buf[4096]{};

    struct client_info
    {
        std::string adr;
        epoll_event ev;
    };

    epoll_event events[number_events];    
    std::pmr::unordered_map<SOCKET,client_info> client_map;

    public:

        void prepare()
        {
                hints.ai_family=AF_INET;
                hints.ai_socktype=SOCK_STREAM;
                hints.ai_flags= AI_PASSIVE;

                int err;

                inf=get_addr("localhost","8080",hints);
                get_name(*inf);
                listener=create_sock(*inf,SOCK_NONBLOCK);
                to_bind(listener,*inf);

                epl=epoll_create1(0);

                epoll_event ev;
                ev.events = EPOLLET | EPOLLIN;
                ev.data.fd=listener;
                err=epoll_ctl(epl,EPOLL_CTL_ADD,listener,&ev);
                if(err==-1){
                    throw std::runtime_error{"epoll_ctl() failed:" + get_error() + "\n"};
                }

                if(listen(listener,number_events)<0){
                    throw std::runtime_error{"Listen() failed:" + get_error() + "\n"};
                }         
        }

    private:

        void close_connection(SOCKET sock) noexcept
        {
            CLOSESOCKET(sock);
            int err=epoll_ctl(epl,EPOLL_CTL_DEL,sock,0);
            client_map.erase(sock);
        }

    private:

        void send_message(SOCKET sock)
        {
            int p=0;
            for(;;)
            {
                int sent=send(sock,buf+p,sizeof(buf)-p,MSG_NOSIGNAL);
                p+=sent;
                if(sent<=0){
                    int err=errno;
                    if(err==EAGAIN || err==EWOULDBLOCK){
                        print(get_client_name(sock),">",buf,"\n");
                        break;
                    }
                    print(get_client_name(sock),get_error(),"\n");
                    close_connection(sock);
                    return;
                }
            }
            
            memset(buf,0,sizeof buf);
        }
    
    private:

            std::string get_client_name(SOCKET sock)
            {
                auto it=client_map.find(sock);
                if(it!=client_map.end()){
                    return it->second.adr + "[" + std::to_string(sock) + "]:\n";
                }
                return {};
            }

    private:

        void recieve_message(SOCKET sock)
        {
            int err;
            for(;;)
            {
                int received_bytes=recv(sock,buf,sizeof(buf),0);
                if(received_bytes<=0)
                {
                    err=errno;
                    if(errno==EAGAIN || errno==EWOULDBLOCK){
                        break;
                    }
                    else{
                        print(get_client_name(sock),get_error());
                        close_connection(sock);
                        return;
                    }
                }
            }

            print(get_client_name(sock),"<",buf,"\n");       
        }

    public:

        void connect()
        {
            int err;

            for(;;)
            {
                int connected=epoll_wait(epl,events,number_events,-1);
                if(connected<0){
                    print("epoll_wait() error",get_error(),"\n");
                    return;
                }

                for(size_t i=0;i<connected;i++)
                {
                    SOCKET sock=events[i].data.fd;
                    if(sock==listener)
                    {
                        sockaddr_storage stor;
                        socklen_t len;

                        sock=accept4(listener,reinterpret_cast<sockaddr*>(&stor),&len,SOCK_NONBLOCK);
                        if(sock<0){
                            print("client accept error\n");
                            continue;
                        }

                        std::string str=get_name(stor);

                        epoll_event ev;
                        ev.events  = EPOLLET | EPOLLIN;
                        ev.data.fd = sock; 
                        err=epoll_ctl(epl,EPOLL_CTL_ADD,sock,&ev);
                        if(err==-1){
                            throw std::runtime_error{"epoll_ctl() failed" + get_error() + "\n"};
                        }

                        client_map[sock] = { str, ev };
                    }
                    else
                    {
                        recieve_message(sock);
                        send_message(sock);
                    }
                }
            }
        }

    public:

    epoll_server()
    {
        client_map.reserve(1024);
    }    

    ~epoll_server()
    {
        CLOSESOCKET(listener);
        freeaddrinfo(inf);
        for(auto &i:client_map)
        {
            CLOSESOCKET(i.first);
        }
    }

};


