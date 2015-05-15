#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <string>

namespace IO
{
    class TCPSocket
    {
    public:
        TCPSocket(int sock, bool closeSock);
        TCPSocket(const char *host, int port);
        ~TCPSocket();

        int getSock() {return sock_;}

        virtual std::string getAddress();
        virtual int getPort();
        virtual std::string getEndpoint();

        virtual std::string getPeerAddress();
        virtual int getPeerPort();
        virtual std::string getPeerEndpoint();

        virtual bool sameMachine();

        virtual void shutdown();

        static void enableNagles(int sock, bool enable);
        static bool isSocket(int sock);
        static bool isConnected(int sock);
        static int getSockPort(int sock);

    private:
        int sock_;
        bool shutdowned_;
        bool closeSock_;
    };
}

#endif // TCPSOCKET_H
