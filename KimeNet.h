#ifndef KIMENET_H_INCLUDED
#define KIMENET_H_INCLUDED

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0503
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


namespace kinet
{
    namespace client {
        class Socket;
    }
    namespace server {
        class Socket;
    }
    class WSAData
    {
        private:
            static bool initialised;
        protected:
            WSAData() noexcept {
                if (!initialised) {
                    ::WSADATA data;
                    auto err = ::WSAStartup(MAKEWORD(2, 2), &data);
                    if (err) {
                    // handle failure
                    } else {
                        initialised = true;
                    }
                }
            }
            ~WSAData() noexcept {
                ::WSACleanup();
            }
    };
    bool WSAData::initialised = false;
    namespace client
    {
        class Socket  : private WSAData
        {
        public:
            void connect(std::string IP,std::string PORT)
            {
                ZeroMemory( &hints, sizeof(hints) );
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_protocol = IPPROTO_TCP;
                iResult = getaddrinfo(IP.c_str(), PORT.c_str(), &hints, &result);
                if ( iResult != 0 )
                {
                    printf("getaddrinfo failed with error: %d\n", iResult);
                    WSACleanup();
                    exit(0);
                }

                    // Attempt to connect to an address until one succeeds
                for(ptr=result; ptr != NULL ; ptr=ptr->ai_next)
                {

                    // Create a SOCKET for connecting to server
                    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                                           ptr->ai_protocol);
                    if (ConnectSocket == INVALID_SOCKET)
                    {
                        printf("socket failed with error: %d\n", WSAGetLastError());
                        WSACleanup();
                        exit(0);
                    }

                    // Connect to server.
                    iResult = ::connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
                    if (iResult == SOCKET_ERROR)
                    {
                        closesocket(ConnectSocket);
                        ConnectSocket = INVALID_SOCKET;
                        continue;
                    }
                    break;
                }

            freeaddrinfo(result);

            if (ConnectSocket == INVALID_SOCKET)
            {
                printf("Unable to connect to server!\n");
                WSACleanup();
                exit(0);
            }
        }
        void send(std::string msg)
        {
            iResult = ::send( ConnectSocket, msg.c_str(), static_cast<int>(msg.size() + 1), 0 );
        }
        void receive(std::string& msg)
        {
            msg.reserve(DEFAULT_BUFLEN); // msg.capacity() should == size
            msg.resize(DEFAULT_BUFLEN); // forces the string to allocate memory and makes sure it's usable
            iResult = recv(ClientSocket, &msg[0], static_cast<int>(msg.capacity()), 0);
        }
        private:
           // static kinet::WSAData wsa;
            WSADATA wsaData;
            SOCKET ConnectSocket = INVALID_SOCKET;
            struct addrinfo *result = NULL,
                                 *ptr = NULL,
                                  hints;
            char *sendbuf = new char[5000];
            char recvbuf[DEFAULT_BUFLEN];
            int iResult;
            int recvbuflen = DEFAULT_BUFLEN;
            SOCKET ListenSocket = INVALID_SOCKET;
            SOCKET ClientSocket = INVALID_SOCKET;
        };
    }
    namespace server
    {
        class Socket : private WSAData
        {
        public:
            struct client
            {
                friend class Socket;
                SOCKET ss;
                bool connected;
            private:
                client() = default;


            };
            client clients[10000];
            u_int clients_connected = 0;
            void start(std::string IP, std::string PORT)
            {
                ZeroMemory(&hints, sizeof(hints));
                hints.ai_family = AF_INET;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_protocol = IPPROTO_TCP;
                hints.ai_flags = AI_PASSIVE;

                // Resolve the server address and port
                iResult = getaddrinfo(IP.c_str(), PORT.c_str(), &hints, &result);
                if ( iResult != 0 ) {
                    printf("getaddrinfo failed with error: %d\n", iResult);
                    WSACleanup();
                    return;
                }

                // Create a SOCKET for connecting to server
                ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
                if (ListenSocket == INVALID_SOCKET) {
                    printf("socket failed with error: %d\n", WSAGetLastError());
                    freeaddrinfo(result);
                    WSACleanup();
                    return;
                }

                // Setup the TCP listening socket
                iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
                if (iResult == SOCKET_ERROR) {
                    printf("bind failed with error: %d\n", WSAGetLastError());
                    freeaddrinfo(result);
                    closesocket(ListenSocket);
                    WSACleanup();
                    return;}
                iResult = listen(ListenSocket, SOMAXCONN);
                if (iResult == SOCKET_ERROR) {
                    printf("listen failed with error: %d\n", WSAGetLastError());
                    closesocket(ListenSocket);
                    WSACleanup();
                    return;
                    }
                freeaddrinfo(result);
            }
            void accept()
            {
                ClientSocket = ::accept(ListenSocket, NULL, NULL);
                if (ClientSocket == INVALID_SOCKET) {
                    printf("accept failed with error: %d\n", WSAGetLastError());
                    closesocket(ListenSocket);
                    WSACleanup();
                    return;
                }
                clients[clients_connected++].ss = ClientSocket;
                clients[clients_connected].connected = true;
            }

            void disconnect(int id)
            {
                clients[id].connected = false;
            }

            void send_all(std::string msg)
            {
                for(unsigned i = 0; i< clients_connected;i++)
                    iResult = ::send(clients[i].ss, msg.c_str(), static_cast<int>(msg.size() + 1), 0 );
            }
            void send_to_id(std::string msg,int id)
            {
                iResult = ::send(clients[id].ss, msg.c_str(), static_cast<int>(msg.size() + 1), 0 );
            }
            void receive_from_id(std::string& msg, int id)
            {
                msg.reserve(DEFAULT_BUFLEN); // msg.capacity() should == size
                msg.resize(DEFAULT_BUFLEN); // forces the string to allocate memory and makes sure it's usable
                iResult = recv(clients[id].ss, &msg[0], static_cast<int>(msg.capacity()), 0);
            }
        private:
            //static kinet::WSAData wsa;
            int iResult;

            SOCKET ListenSocket = INVALID_SOCKET;
            SOCKET ClientSocket = INVALID_SOCKET;

            struct addrinfo *result = NULL;
            struct addrinfo hints;

            int iSendResult;
        };
    }


}


#endif // KIMENET_H_INCLUDED
