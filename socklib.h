#pragma once
#include <winsock2.h>
#include <memory>
#include <string>

#define  sock_errno  WSAGetLastError
#define  soclose     closesocket

class Socket
{
public:
#pragma warning(suppress:4480)
    enum : unsigned long { wrongIP = 0xFFFFFFFF, localIP = 0x7F000001 };
    Socket(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0);
    Socket(SOCKET sockno);         // Get owned!!!
    Socket(const Socket&so)              = delete;
    Socket& operator = (const Socket&so) = delete;
    Socket(Socket&&so):s(so.s) { so.s = INVALID_SOCKET; }
    virtual ~Socket()
    {
        shutdown(s,2);
        soclose(s);
    }

    unsigned long peer() const;

    void soRcvTimeout(int seconds) const;
    void soSndTimeout(int seconds) const;

private:
    SOCKET s;

protected:
    SOCKET sock() const { return s; }
    int setsockopt(int level, int optname, const char * optval, int optlen) const;

public:
    // эти функции при ошибке НЕ генерируют исключения
    static std::string   host(unsigned long addr);  // Имя по адресу.  Ошибка - ""
    static unsigned long host(const char * name);   // Адрес по имени. Ошибка - wrongIP
    static unsigned long addr(const char * a);      // "127.0.0.1" -> 0x7F000001
    static std::string   addr(unsigned long a);     // 0x7F000001 -> "127.0.0.1"
    static unsigned long addr(const std::string& a);// "127.0.0.1" -> 0x7F000001
    static unsigned long hostid();                  // Наш адрес.  Ошибка - wrongIP
    static std::string   hostname();                // Наше имя.

    static int           port(const std::string& service);
    static int           error();

};


class StrSocket: public Socket
{
public:
    StrSocket(unsigned long ip = 0, unsigned short int port = 0);
    StrSocket(const StrSocket&s) = delete;
    StrSocket(StrSocket&&s):Socket(std::move(s)){};
    StrSocket(SOCKET s):Socket(s) {}        // Get owned!!
    StrSocket& operator = (const StrSocket&so) = delete;

    int call(unsigned long addr, unsigned short int port) const;
    int call(const std::string& addr, unsigned short int port) const;
    StrSocket answer(int Connections = 0) const;
    int read (char       * buf, size_t buflen) const;
    int write(const char * buf, size_t buflen) const;
    int read (void       * buf, size_t buflen) const { return read ((char *)buf, buflen); }
    int write(const void * buf, size_t buflen) const { return write((const char *)buf, buflen); }

    std::string gets() const;
    size_t puts(const std::string& ss)  const;
    size_t puts(const char *ss)         const;

    void soReuse() const;
    void bindInfo(unsigned long& addr, unsigned short& port) const;
    unsigned long  bindAddr() const {
        unsigned long a; unsigned short p; bindInfo(a,p); return a; }
    unsigned short bindPort() const {
        unsigned long a; unsigned short p; bindInfo(a,p); return p; }
};


