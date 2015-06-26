#include "TCPSocket.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <errno.h>
#define SOCKLEN socklen_t
#define errorNumber errno
#else
#include <io.h>
#include <winsock2.h>
#define SOCKLEN int
#define errorNumber WSAGetLastError()
#endif

#include "UTILS/STRING/HString.h"
using UTILS::STRING::StringFromFormat;

namespace IO
{
    // Socket initialisation
    static bool socketsInitialised = false;
    static void initSockets() {
        if (socketsInitialised)
            return;
        #ifdef WIN32
        WORD requiredVersion = MAKEWORD(2,0);
        WSADATA initResult;

        if (WSAStartup(requiredVersion, &initResult) != 0)
            throw _HException_(StringFromFormat("unable to initialise Winsock2, errorNumber = %d", errorNumber), HException::IO);
        #else
        signal(SIGPIPE, SIG_IGN);
        #endif
        socketsInitialised = true;
    }

    TCPSocket::TCPSocket(int sock, bool closeSock)
        : sock_(sock)
        , closeSock_(closeSock) {
    }

    TCPSocket::TCPSocket(const char *host, int port)
        : closeSock_(true) {
        initSockets();

        if ((sock_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            throw _HException_(StringFromFormat("unable to create socket, errorNumber = %d", errorNumber), HException::IO);

        #ifndef WIN32
          // - By default, close the socket on exec()
            fcntl(sock_, F_SETFD, FD_CLOEXEC);
        #endif

        // Try processing the host as an IP address
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(host);
        addr.sin_port = htons(port);
        if ((int)addr.sin_addr.s_addr == -1) {
            // Host was not an IP address - try resolving as DNS name
            struct hostent *hostinfo;
            hostinfo = gethostbyname(host);
            if (hostinfo && hostinfo->h_addr) {
                addr.sin_addr.s_addr = ((struct in_addr *)hostinfo->h_addr)->s_addr;
            }
            else {
                throw _HException_(StringFromFormat("unable to resolve host by name, errorNumber = %d", errorNumber), HException::IO);
            }
        }

        // Attempt to connect to the remote host
        if (connect(sock_, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
            throw _HException_(StringFromFormat("unable to connect to host, errorNumber = %d", errorNumber), HException::IO);
        }

        // Disable Nagle's algorithm, to reduce latency
        enableNagles(sock_, false);
    }

    TCPSocket::~TCPSocket() {
        if (closeSock_) {
            #ifdef WIN32
            closesocket(getSock());
            #else
            close(getSock());
            #endif
        }
    }

    std::string TCPSocket::getAddress() {
        struct sockaddr_in  info;
        struct in_addr    addr;
        SOCKLEN info_size = sizeof(info);

        getsockname(getSock(), (struct sockaddr *)&info, &info_size);
        memcpy(&addr, &info.sin_addr, sizeof(addr));

        char* name = inet_ntoa(addr);
        if (name) {
            return std::string(name);
        }

        return std::string("");
    }

    int TCPSocket::getPort() {
        return getSockPort(getSock());
    }

    std::string TCPSocket::getEndpoint() {
        return StringFromFormat("%s::%d", getAddress().c_str(), getPort());
    }

    std::string TCPSocket::getPeerAddress() {
        struct sockaddr_in  info;
        struct in_addr    addr;
        SOCKLEN info_size = sizeof(info);

        getpeername(getSock(), (struct sockaddr *)&info, &info_size);
        memcpy(&addr, &info.sin_addr, sizeof(addr));

        char* name = inet_ntoa(addr);
        if (name) {
            return std::string(name);
        }

        return std::string("");
    }

    int TCPSocket::getPeerPort() {
        struct sockaddr_in  info;
        SOCKLEN info_size = sizeof(info);

        getpeername(getSock(), (struct sockaddr *)&info, &info_size);
        return ntohs(info.sin_port);
    }

    std::string TCPSocket::getPeerEndpoint() {
        return StringFromFormat("%s::%d", getPeerAddress().c_str(), getPeerPort());
    }

    bool TCPSocket::sameMachine() {
        struct sockaddr_in peeraddr, myaddr;
        SOCKLEN addrlen = sizeof(struct sockaddr_in);

        getpeername(getSock(), (struct sockaddr *)&peeraddr, &addrlen);
        getsockname(getSock(), (struct sockaddr *)&myaddr, &addrlen);

        return (peeraddr.sin_addr.s_addr == myaddr.sin_addr.s_addr);
    }

    void TCPSocket::shutdown()
    {
        shutdowned_ = true;
        ::shutdown(getSock(), 2);
    }

    void TCPSocket::enableNagles(int sock, bool enable) {
        int one = enable ? 0 : 1;
        if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one)) < 0) {
            throw _HException_(StringFromFormat("unable to setsockopt TCP_NODELAY, errorNumber = %d", errorNumber), HException::IO);
        }
    }

    bool TCPSocket::isSocket(int sock) {
        struct sockaddr_in info;
        SOCKLEN info_size = sizeof(info);
        return getsockname(sock, (struct sockaddr *)&info, &info_size) >= 0;
    }

    bool TCPSocket::isConnected(int sock) {
        struct sockaddr_in info;
        SOCKLEN info_size = sizeof(info);
        return getpeername(sock, (struct sockaddr *)&info, &info_size) >= 0;
    }

    int TCPSocket::getSockPort(int sock) {
        struct sockaddr_in info;
        SOCKLEN info_size = sizeof(info);
        if (getsockname(sock, (struct sockaddr *)&info, &info_size) < 0)
            return 0;
        return ntohs(info.sin_port);
    }
}
