#include "../../include/socket_header.hpp"



class epoll_client 
{
        int epl;
        SOCKET sock;
        epoll_event event;
        addrinfo hints{};
        addrinfo *inf{};
        char buf[4096]{};

    private:

        void prepare()
        {
            hints.ai_socktype=SOCK_STREAM;
            hints.ai_family=AF_UNSPEC;
            inf=get_addr("localhost","8080",hints);

            print("Connected to host:",get_name(*inf),"\n");
            sock=create_sock(*inf,SOCK_NONBLOCK);
            
            epl=epoll_create1(0);
            epoll_event ev;
            ev.data.fd=sock;
            ev.events=EPOLLIN | EPOLLET;
            int err=epoll_ctl(epl,EPOLL_CTL_ADD,sock,&ev);
            if(err<0){
                throw std::runtime_error{"epoll_ctl() failed:" + get_error() + "\n"};
            }

            to_connect(sock,*inf);
        }

    public:

        void connect()
        {
            prepare();
            for(;;)
            {
                send_message();
                receive_message();
            }
        }

    private:

        void send_message()
        {
            print(">");
            std::fgets(buf,sizeof buf,stdin);
            int p=0;

            for(;;)
            {
                int sent=send(sock,buf+p,sizeof(buf)-p,MSG_NOSIGNAL);
                p+=sent;
                if(sent<0)
                {
                    int err=errno;
                    if(err==EAGAIN || err==EWOULDBLOCK){
                        break;
                    }   
                    else{
                        throw std::runtime_error{"Host error:" + get_error() + "\n"};
                    }
                }
                else if(p==sizeof(buf)){
                    break;
                }
            }
            
            memset(buf,0,sizeof buf);
        }


        void receive_message()
        {
              int connected=epoll_wait(epl,&event,1,100000);
                if(connected<0){
                        throw std::runtime_error{"Host wait error:" + get_error() + "\n"};
                }

            for(;;)
            {
                int rec=recv(sock,buf, sizeof buf,0);
                if(rec==-1)
                {
                    int err=errno;
                    if(err==EAGAIN || err==EWOULDBLOCK){
                        break;
                    }
                    else{
                        throw std::runtime_error{"Host recv error:" + get_error() + "\n"};
                    }
                }
            }
            print("<",buf,"\n");
            memset(buf,0,sizeof buf);
        }

    public:

        ~epoll_client()
        {
            CLOSESOCKET(sock);
            freeaddrinfo(inf);
        }
};  