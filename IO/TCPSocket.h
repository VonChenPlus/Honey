#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "IO/Socket.h"

namespace IO
{
    class TCPSocket final: public Socket
    {
    public:
        TCPSocket(int sock, bool closeSock);
        TCPSocket(const char *host, int port);
        ~TCPSocket();

        virtual std::string getAddress() override;
        virtual int getPort() override;
        virtual std::string getEndpoint() override;

        virtual std::string getPeerAddress() override;
        virtual int getPeerPort() override;
        virtual std::string getPeerEndpoint() override;

        virtual bool sameMachine() override;

        virtual void shutdown() override;

        static void enableNagles(int sock, bool enable);
        static bool isSocket(int sock);
        static bool isConnected(int sock);
        static int getSockPort(int sock);

    private:
        bool closeSock_;
    };
}

#endif // TCPSOCKET_H
