#include "TCPSocket.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/select.h>
#define errorNumber errno
#else
#include <io.h>
#include <winsock2.h>
#define errorNumber WSAGetLastError()
#endif

#include "UTILS/STRING/String.h"
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
            throw _NException_(StringFromFormat("unable to initialise Winsock2, errorNumber = %d", errorNumber), NException::IO);
        #else
        signal(SIGPIPE, SIG_IGN);
        #endif
        socketsInitialised = true;
    }

    TCPSocket::TCPSocket(int sock, bool closeSock)
        : Socket(sock)
        , closeSock_(closeSock) {
    }

    TCPSocket::TCPSocket(const char *host, int port)
        : closeSock_(true) {
        initSockets();

        int sock;
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            throw _NException_(StringFromFormat("unable to create socket, errorNumber = %d", errorNumber), NException::IO);

        #ifndef WIN32
          // - By default, close the socket on exec()
            fcntl(sock, F_SETFD, FD_CLOEXEC);
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
                throw _NException_(StringFromFormat("unable to resolve host by name, errorNumber = %d", errorNumber), NException::IO);
            }
        }

        // Attempt to connect to the remote host
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
            throw _NException_(StringFromFormat("unable to connect to host, errorNumber = %d", errorNumber), NException::IO);
        }

        // Disable Nagle's algorithm, to reduce latency
        enableNagles(sock, false);
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
        int info_size = sizeof(info);

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
        std::string endpoint = getAddress();
        int port = getPort();
        endpoint.append("::");
        char buffer[100] = {0};
        itoa(port, buffer, 10);
        endpoint.append(buffer);
        return endpoint;
    }

    std::string TCPSocket::getPeerAddress() {
        struct sockaddr_in  info;
        struct in_addr    addr;
        int info_size = sizeof(info);

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
        int info_size = sizeof(info);

        getpeername(getSock(), (struct sockaddr *)&info, &info_size);
        return ntohs(info.sin_port);
    }

    std::string TCPSocket::getPeerEndpoint() {
        std::string endpoint = getPeerAddress();
        int port = getPeerPort();
        endpoint.append("::");
        char buffer[100] = {0};
        itoa(port, buffer, 10);
        endpoint.append(buffer);
        return endpoint;
    }

    bool TCPSocket::sameMachine() {
        struct sockaddr_in peeraddr, myaddr;
        int addrlen = sizeof(struct sockaddr_in);

        getpeername(getSock(), (struct sockaddr *)&peeraddr, &addrlen);
        getsockname(getSock(), (struct sockaddr *)&myaddr, &addrlen);

        return (peeraddr.sin_addr.s_addr == myaddr.sin_addr.s_addr);
    }

    void TCPSocket::shutdown()
    {
        Socket::shutdown();
        ::shutdown(getSock(), 2);
    }

    void TCPSocket::enableNagles(int sock, bool enable) {
        int one = enable ? 0 : 1;
        if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one)) < 0) {
            throw _NException_(StringFromFormat("unable to setsockopt TCP_NODELAY, errorNumber = %d", errorNumber), NException::IO);
        }
    }

    bool TCPSocket::isSocket(int sock)
    {
        struct sockaddr_in info;
        int info_size = sizeof(info);
        return getsockname(sock, (struct sockaddr *)&info, &info_size) >= 0;
    }

    bool TCPSocket::isConnected(int sock)
    {
        struct sockaddr_in info;
        int info_size = sizeof(info);
        return getpeername(sock, (struct sockaddr *)&info, &info_size) >= 0;
    }


    int TCPSocket::getSockPort(int sock) {
        struct sockaddr_in info;
        int info_size = sizeof(info);
        if (getsockname(sock, (struct sockaddr *)&info, &info_size) < 0)
            return 0;
        return ntohs(info.sin_port);
    }
}
