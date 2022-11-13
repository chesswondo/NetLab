#include "socklib.h"

#pragma comment(lib,"ws2_32.lib")

#define  sock_errno  WSAGetLastError
#define  soclose     closesocket

#include <vector>
#include <stdexcept>
#include <format>
#include <iostream>

#pragma warning(disable: 4996 4127)

#define  DEBUG 0

using namespace std;

namespace {   // WinSock Initialization
    struct initWinSock
    {
        initWinSock();
        ~initWinSock();
    } init;

    initWinSock::initWinSock()
    {
        WORD wVersionRequested = MAKEWORD(1,1);;
        WSADATA wsaData;
        WSAStartup(wVersionRequested, &wsaData);
    }
    initWinSock::~initWinSock()
    {
        WSACleanup();
    }
}

Socket::Socket(int domain, int type, int protocol)
:s(socket(domain,type,protocol))
{
    if (sock() == INVALID_SOCKET) throw runtime_error("bad socket creation");
}

Socket::Socket(SOCKET sockno)
:s(sockno)
{}

unsigned long Socket::peer() const
{
    struct sockaddr_in name;
    int namelen = sizeof(name);
    int res = getpeername(sock(),reinterpret_cast<sockaddr*>(&name),&namelen);
    if (res) return wrongIP;
    return ntohl(name.sin_addr.s_addr);
};

int Socket::setsockopt(int level, int optname,
                                 const char * optval, int optlen) const
{
    return ::setsockopt(sock(),level,optname,optval,optlen);
}

void Socket::soRcvTimeout(int seconds) const
{
    struct timeval T;
    T.tv_sec  = seconds*1000;
    T.tv_usec = 0;
    setsockopt(SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<char*>(&T),sizeof(T));
}
void Socket::soSndTimeout(int seconds) const
{
    struct timeval T;
    T.tv_sec  = seconds*1000;
    T.tv_usec = 0;
    setsockopt(SOL_SOCKET,SO_SNDTIMEO,reinterpret_cast<char*>(&T),sizeof(T));
}

string Socket::addr(unsigned long a)
{
    a = htonl(a);
    return inet_ntoa(*reinterpret_cast<in_addr*>(&a));
}

unsigned long Socket::addr(const char * a)
{
    if (a == 0) return wrongIP;
    unsigned long v = inet_addr(a);
    return ntohl(v);
}

unsigned long Socket::host(const char* name)
{
    struct hostent * H;
    H = gethostbyname(name);
    if (H == 0) {
        if (false)
        {
            char buf[20];
            _snprintf(buf,20,". Code %d",(h_errno == -1) ? sock_errno() : h_errno);
            string err("gethostbyname ");
            err = err + name + ": " +
                ((h_errno == -1)             ? "generic error, see code" :
                 (h_errno == HOST_NOT_FOUND) ? "host not found" :
                 (h_errno == TRY_AGAIN)      ? "try again later" :
                 (h_errno == NO_RECOVERY)    ? "unrecoverable error" :
                 (h_errno == NO_DATA)        ? "valid address does not have address" :
                 (h_errno == NO_ADDRESS)     ? "valid address does not have address" :
                 "???") + buf;
            throw runtime_error(err.c_str());
        }
        else
        {
            return wrongIP;
        }
    }
    return ntohl(*reinterpret_cast<unsigned long *>(&H->h_addr[0]));
}

string Socket::host(unsigned long a)
{
    unsigned long b = htonl(a);
    struct hostent * H;
    H = gethostbyaddr(reinterpret_cast<char*>(&b),sizeof(b),AF_INET);
    if (H == 0)
    {
        if (false)
        {
            char buf[20];
            _snprintf(buf,20,". Code %d",(h_errno == -1) ? sock_errno() : h_errno);
            string err("gethostbyaddr ");
            err = err + addr(a) + ": " +
                ((h_errno == -1)             ? "generic error, see code" :
                 (h_errno == HOST_NOT_FOUND) ? "host not found" :
                 (h_errno == TRY_AGAIN)      ? "try again later" :
                 (h_errno == NO_RECOVERY)    ? "unrecoverable error" :
                 (h_errno == NO_DATA)        ? "valid address does not have address" :
                 (h_errno == NO_ADDRESS)     ? "valid address does not have address" :
                 "???") + buf;
            throw runtime_error(err.c_str());
        }
        else
        {
            return "";
        }
    }
    return string(H->h_name);
}


unsigned long Socket::hostid()
{
    unsigned long x = host(hostname().c_str());
    return x;
}

string Socket::hostname()
{
    const int strLen = 512;
    char buf[strLen];
    if (gethostname(buf,strLen) == 0) return buf;
    return "localhost";
}

int Socket::error()
{
    return sock_errno();
}

int Socket::port(const string& service)
{
    servent * se = getservbyname(service.c_str(),"tcp");
    if (se != 0)
    {
        return htons(se->s_port);
    }
    else
    {
        struct protoent * pe = getprotobyname(service.c_str());
        if (pe != 0)
        {
            return htons(pe->p_proto);
        }
    }
    return 0;
}


StrSocket::StrSocket(unsigned long ip, unsigned short int port)
:Socket(AF_INET,SOCK_STREAM,0)
{
    soReuse();
    struct sockaddr_in name;
    memset(&name,0,sizeof(name));
    if ((ip == 0)||(ip == wrongIP)) {
        name.sin_addr.s_addr = INADDR_ANY;
    } else {
        name.sin_addr.s_addr = htonl(ip);
    };
    name.sin_family = AF_INET;
    name.sin_port   = htons(port);
    int rc = bind(sock(),reinterpret_cast<sockaddr*>(&name),sizeof(name));
    if (rc) throw runtime_error(format("bind socket error {}",rc));
}

int StrSocket::call(unsigned long addr, unsigned short int port) const
{
    struct sockaddr_in name;
    memset(&name,0,sizeof(name));
    name.sin_family      = AF_INET;
    name.sin_addr.s_addr = htonl(addr);
    name.sin_port        = htons(port);

    int rc = connect(sock(),reinterpret_cast<sockaddr*>(&name),sizeof(name));
    if (rc) {
        rc = sock_errno();
        throw runtime_error(format("connect socket error {}",rc));
    }
    return rc;
}

int StrSocket::call(const string& addr, unsigned short int port) const
{
    unsigned long a;
    a = host(addr.c_str());
    if (a == wrongIP)
    {
        throw runtime_error(format("error resolve {}: {}",addr,sock_errno()));
        return sock_errno();
    }
    return call(a,port);
}

int StrSocket::read(char * buf, size_t buflen) const
{
    int rc = recv(static_cast<int>(sock()),buf,
                  static_cast<int>(buflen),0);
    if (rc == SOCKET_ERROR)
        throw runtime_error("recv socket error");
    return rc;
}

string StrSocket::gets() const
{
    char c = 0;
    string v;
    do {
        int rc = recv(sock(),&c,1,0);
#if DEBUG
        if (rc == 0 ) cout << "sock_errno() == " << sock_errno() << endl;
#endif
        if (rc == 0) throw runtime_error("recv connection closed");
        if (rc == SOCKET_ERROR)
            throw runtime_error("recv socket error");
        if (rc == 1) v.push_back(c);
    } while (c != '\n');
    v.pop_back();
    return v;
}

int StrSocket::write(const char * buf, size_t buflen) const
{
    if (buflen == 0) return 0;
    int rc = send(static_cast<int>(sock()),buf,
                  static_cast<int>(buflen),0);
    if (rc == SOCKET_ERROR)
        throw runtime_error("send socket error");
    return rc;
}

StrSocket StrSocket::answer(int Connections) const
{
    struct sockaddr client;
    int clilen = sizeof(client);
    if (listen(sock(),(Connections <= 0) ? SOMAXCONN : Connections))
        throw runtime_error(format("listen error {}",sock_errno()));
    SOCKET rc = accept(/*static_cast<int>*/(sock()),&client,&clilen);
    if (rc == INVALID_SOCKET)
        throw runtime_error(format("accept error {}",sock_errno()));
    return StrSocket(rc);
};

void StrSocket::soReuse() const
{
    int on = 1;
    setsockopt(SOL_SOCKET,SO_REUSEADDR,(char*)&on,sizeof(on));
}

void StrSocket::bindInfo(unsigned long& a, unsigned short& p) const
{
    sockaddr_in sa;
    int sl = sizeof(sa);
    if (getsockname(sock(),
                    reinterpret_cast<sockaddr*>(&sa),&sl) != 0)
    {
        a = 0; p = 0;
    }
    else
    {
        p = sa.sin_port;
        a = ntohl(*reinterpret_cast<unsigned long *>(&sa.sin_addr));
    }
}

size_t StrSocket::puts(const char *ss) const
{
    int rc = write(ss,strlen(ss));
    rc += write("\n",1);
    return rc;
}
size_t StrSocket::puts(const std::string& ss) const
{
    return puts(ss.c_str());
}


