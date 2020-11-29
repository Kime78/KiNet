#pragma once
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0503
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <string>
#define KiNet_DEFAULT_BUFLEN 512
#define KiNet_DEFAULT_PORT "27015"


namespace kinet
{
    enum class Status
    {
        no_error = 0,
        ip_error = 1,
        binding_error = 2,
        port_error = 3,
        send_error = 4,
        receive_error = 5,
        connect_error = 6,


    };
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
                    std::string error = "WSAData couldn't start! Error Code: " + std::to_string(err);
                    throw std::runtime_error(error.c_str());
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
                    //printf("getaddrinfo failed with error: %d\n", iResult);
                    std::string error = "getaddrinfo failed with error:" + std::to_string(iResult);
                    throw std::runtime_error(error.c_str());
                    WSACleanup();

                }

                    // Attempt to connect to an address until one succeeds
                for(ptr=result; ptr != NULL ; ptr=ptr->ai_next)
                {

                    // Create a SOCKET for connecting to server
                    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                                           ptr->ai_protocol);
                    if (ConnectSocket == INVALID_SOCKET)
                    {
                        //printf("socket failed with error: %ld\n", WSAGetLastError());
                        std::string error = "getaddrinfo failed with error:" + WSAGetLastError();
                        throw std::runtime_error(error.c_str());
                        WSACleanup();

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
                std::string error = "Couldn't connect to server!";
                throw std::runtime_error(error.c_str());
                
                WSACleanup();

            }
        }
        void send(std::string msg) //bit buffer
        {
            iResult = ::send( ConnectSocket, msg.c_str(), static_cast<int>(msg.size() + 1), 0 );
        }
        void receive(std::string& msg) //bit buffer
        {
            msg.reserve(KiNet_DEFAULT_BUFLEN); // msg.capacity() should == size
            msg.resize(KiNet_DEFAULT_BUFLEN); // forces the string to allocate memory and makes sure it's usable
            iResult = recv(ClientSocket, &msg[0], static_cast<int>(msg.capacity()), 0);
        }
        private:
           // static kinet::WSAData wsa;
            //WSADATA wsaData;
            SOCKET ConnectSocket = INVALID_SOCKET;
            struct addrinfo *result = NULL,
                                 *ptr = NULL,
                                  hints;
            char *sendbuf = new char[5000];
            char recvbuf[KiNet_DEFAULT_BUFLEN];
            int iResult;
            int recvbuflen = KiNet_DEFAULT_BUFLEN;
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
            client clients[10000]; //list
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
                    //printf("getaddrinfo failed with error: %d\n", iResult);
                    std::string error = "getaddrinfo failed with error:" + iResult;
                    throw std::runtime_error(error.c_str());
                    WSACleanup();

                }

                // Create a SOCKET for connecting to server
                ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
                if (ListenSocket == INVALID_SOCKET) {
                    //printf("socket failed with error: %ld\n", WSAGetLastError());
                    std::string error = "socket failed with error:" + WSAGetLastError();
                    throw std::runtime_error(error.c_str());
                    freeaddrinfo(result);
                    WSACleanup();

                }

                // Setup the TCP listening socket
                iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
                if (iResult == SOCKET_ERROR) {
                    //printf("bind failed with error: %d\n", WSAGetLastError());
                    std::string error = "bind failed with error:" + WSAGetLastError();
                    throw std::runtime_error(error.c_str());
                    freeaddrinfo(result);
                    closesocket(ListenSocket);
                    WSACleanup();
                    }
                iResult = listen(ListenSocket, SOMAXCONN);
                if (iResult == SOCKET_ERROR) {
                    //printf("listen failed with error: %d\n", WSAGetLastError());
                    std::string error = "listen failed with error:" + WSAGetLastError();
                    throw std::runtime_error(error.c_str());
                    closesocket(ListenSocket);
                    WSACleanup();

                    }
                freeaddrinfo(result);
            }
            void accept()
            {
                ClientSocket = ::accept(ListenSocket, NULL, NULL);
                if (ClientSocket == INVALID_SOCKET) {
                    //printf("accept failed with error: %d\n", WSAGetLastError());
                    std::string error = "accept failed with error:" + WSAGetLastError();
                    throw std::runtime_error(error.c_str());
                    closesocket(ListenSocket);
                    WSACleanup();

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
                for(int i = 0; i< clients_connected;i++)
                    iResult = ::send(clients[i].ss, msg.c_str(), static_cast<int>(msg.size() + 1), 0 );
            }
            void send_to_id(std::string msg,int id)
            {
                iResult = ::send(clients[id].ss, msg.c_str(), static_cast<int>(msg.size() + 1), 0 );
            }
            void receive_from_id(std::string& msg, int id)
            {
                msg.reserve(KiNet_DEFAULT_BUFLEN); // msg.capacity() should == size
                msg.resize(KiNet_DEFAULT_BUFLEN); // forces the string to allocate memory and makes sure it's usable
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

