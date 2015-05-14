#ifndef SOCKET_H
#define SOCKET_H

#include <string>

#include "BASE/Buffer.h"

namespace IO
{
    class Socket
    {
    public:
        Socket(int sock): sock_(sock) {
        }
        virtual ~Socket() {}

        int getSock() {return sock_;}

        // if shutdown() is overridden then the override MUST call on to here
        virtual void shutdown() {shutdowned_ = true;}
        bool isShutdown() const {return shutdowned_;}

        // information about this end of the socket
        virtual std::string getAddress() = 0; // a string e.g. "192.168.0.1"
        virtual int getPort() = 0;
        virtual std::string getEndpoint() = 0; // <address>::<port>

        // information about the remote end of the socket
        virtual std::string getPeerAddress() = 0; // a string e.g. "192.168.0.1"
        virtual int getPeerPort() = 0;
        virtual std::string getPeerEndpoint() = 0; // <address>::<port>

        // Is the remote end on the same machine?
        virtual bool sameMachine() = 0;

    protected:
        Socket() {}
        void setSock(int sock) { sock_ = sock; }

        int sock_;
        bool shutdowned_;
    };
}

#endif // SOCKET_H
