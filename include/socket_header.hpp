#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <concepts>
#include <string_view>
#include <ranges>
#include <functional>
#include <iterator>
#include <ranges>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>



#define SOCKET int
#define ISVALIDSOCKET(s) ((s)>=0)
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)



template<typename R>
concept is_range=std::ranges::viewable_range<R>;

template<typename  R>
concept is_string_container = 
std::ranges::viewable_range<R> 
&&
requires(R &&r)
{
    {*r.begin()} -> std::convertible_to<std::string_view>;
};


struct expander
{
    template<typename...Types>
    expander([[maybe_unused]] Types&&...args){}
};


template<typename...Types>
struct adapter : Types...
{
    using Types::operator()...;   
};



template<typename...Types>
adapter(Types...)->adapter<Types...>;



template<typename...Types>
inline void print(Types&&...args)
{

     adapter mycaller
    {

        [](std::string_view vw="\n")
        {
            std::cout<<vw;
        },

        [](std::wstring_view vw=L"\n")
        {
            std::wcout<<vw;
        },

        [](std::string_view vw,const size_t sz)
        {
            std::cout<<vw.substr(0,sz);
        },

        [](std::wstring vw,const size_t sz)
        {
            std::wcout<<vw.substr(0,sz);
        },

        []<typename T>
        requires requires (T&& t)
        {
            std::to_wstring(t);
        }
        (T&& t)
        {
            std::wcout<<std::to_wstring(t);
        },

        []<is_string_container R> 
        (R&& r)
        {
            for(auto it=r.begin();it!=r.end();it++)
            {
                std::string_view vw{*it};
                std::cout<<vw;
            }
        },

        []<std::ranges::viewable_range R>
        requires std::is_convertible_v<R,std::wstring>
        (R&& r)
        {
            std::ranges::ref_view Ran=r;
            std::wcout<<std::wstring{Ran.begin(),Ran.end()};
        },

        []<typename It>
        requires requires (It it)
        {
            requires  std::is_base_of_v<std::forward_iterator_tag,It>;
            requires  std::is_convertible_v<decltype(*it),wchar_t>;
        }
        (It beg,It end)
        {
            std::wcout<<std::wstring{beg,end};
        },

        []<typename T>
        requires requires(T&& t)
        {
            t.begin();
            t.end();

            std::cout<<*t.begin();
        }
        (T&& t)
        {
            std::string_view str{t.begin(),t.end()};
            std::cout<<str;
        }

    };


    auto call_print=[&mycaller]<typename Arg>
    (Arg&& arg)
    {
        std::invoke(mycaller,std::forward<Arg>(arg));
    };
 
    expander{(call_print(std::forward<Types>(args)),void(),0)...};
}



template<typename...Types>
requires requires(Types&&...args)
{
   print(std::forward<Types>(args)...);
}
auto all_to_string(Types&&...args)
{
    auto parse=[]<typename T>
    (std::string& str,T&& t)
    {
        std::string_view view=t;
        str.append(view);
    };

    std::string str;
    (parse(str,std::forward<Types>(args)),...);
    return str;
}



//=============================================================================================================================================//


[[nodiscard]]
SOCKET create_sock(const addrinfo& inf,auto...flags)
{
    print("Creating socket...\n");
    SOCKET sock=socket(inf.ai_family,(inf.ai_socktype |...| flags),inf.ai_protocol);
    if(!ISVALIDSOCKET(sock))
        std::runtime_error{"Socket is invalid"};

    return sock;
}



void to_bind(SOCKET& sock,const addrinfo& inf)
{
    print("Binding...\n");
    if(bind(sock,inf.ai_addr,inf.ai_addrlen))
        std::runtime_error{"Bind error:" + std::to_string(GETSOCKETERRNO())};
}



void to_connect(SOCKET& sock,addrinfo& inf)
{
    print("Connecting...\n");
    if(connect(sock,inf.ai_addr,inf.ai_addrlen)<0)
        std::runtime_error{"Connection socket failed:" + std::to_string(GETSOCKETERRNO())};
}



[[nodiscard]]
 addrinfo* get_addr(std::string_view adr,std::string_view port,addrinfo& inf)
{
    addrinfo* ptr{};
    if(getaddrinfo(adr.data(),port.data(),&inf,&ptr))
        throw std::runtime_error{"get_addr failed:"+std::to_string(GETSOCKETERRNO())};
    return ptr;
}



auto get_name(addrinfo& inf)
{
    char addr[100];
    char serv[100];

    getnameinfo(inf.ai_addr,inf.ai_addrlen,
    addr,100,serv,100,NI_NUMERICHOST | NI_NUMERICSERV);

    print("Remote address is:  ",addr,":",serv,"\n");

    std::string str{addr};
    str.append(serv);

    return str;
}


auto get_name(sockaddr_storage& address)
{
    char buf[100];
    socklen_t sz=sizeof(address);
    getnameinfo((sockaddr*)(&address),sz,buf,
        sizeof(buf),0,0,NI_NUMERICHOST | NI_NUMERICSERV
    );
    print("\nClient address:",buf,"\n");   
    return std::string{buf};
}


std::string get_error()
{
    return strerror(errno); 
}