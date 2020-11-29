#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <strings.h>
#include <string>
#include <exception>
#include <cstring>

#define KINET_DEFAULT_PORT 42069
#define KINET_DEFAULT_BUFLEN 512
#define KINET_LOCALHOST "127.0.0.1"

namespace kinet
{
    namespace client 
    {
        class Socket
        {
        public:
            void connect(std::string address, int port)
            {
                client = socket(AF_INET, SOCK_STREAM, 0);
                if(client < 0)
                {
                    throw std::runtime_error("Error creating socket");
                }

                server = gethostbyname(address.c_str());
                if(server == NULL)
                {
                    throw std::runtime_error("IP address is invalid");
                }

                bzero((char *) &server_addr, sizeof(server_addr));
                server_addr.sin_family = AF_INET;
                bcopy((char *)server->h_addr, 
                    (char *)&server_addr.sin_addr.s_addr,
                    server->h_length);
                server_addr.sin_port = htons(port);

                if (::connect(client, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
                    throw std::runtime_error("Failed to connect to server!");

            }

            void send(std::string message)
            {
                result = write(client, message.c_str(), message.length());
                if (result < 0) 
                    throw std::runtime_error("ERROR writing to socket");
            }

            void receive(std::string& message)
            {
                result = read(client, buffer, KINET_DEFAULT_BUFLEN);
                if (result < 0) 
                {
                    throw std::runtime_error("ERROR reading from socket");
                }
                else
                {
                    //std::cout << result << '\n';     
                    message.assign(buffer, strlen(buffer));   
                }
            }
        private:
            int client, result;
            struct sockaddr_in server_addr;
            struct hostent *server;
            char buffer[KINET_DEFAULT_BUFLEN];
        };
    }

    namespace server
    {
        class Socket
        {
        public:    
            void start(std::string address, int port)
            {
                server = socket(AF_INET, SOCK_STREAM, 0);

                if(server < 0)
                {
                    throw std::runtime_error("Error opening socket");
                }

                bzero((char*) &server_addr, sizeof(server_addr));//set the address to 0?

                server_addr.sin_family = AF_INET;
                server_addr.sin_addr.s_addr = htonl(INADDR_ANY);;
                server_addr.sin_port = htons(port);    
                //inet_pton()

                if(bind(server, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
                {
                    throw std::runtime_error("Error Binding Port");
                }
            }

            void accept()
            {
                listen(server, 10);
                client_len = sizeof(client_addr);

                new_socket = ::accept(server, (struct sockaddr *) &client_addr, &client_len);
                if(new_socket < 0)
                {
                    throw std::runtime_error("Error Accepting a client");
                }
            }

            void disconnect();

            void send_all(std::string message)
            {
                result = write(new_socket, message.c_str(), KINET_DEFAULT_BUFLEN);
                //std::cout << result;
                if(result < 0)
                {
                    throw std::runtime_error("Error sending a message!");
                }
            }

            //void send_privately();

            void receive(std::string& message)
            {
                bzero(buffer, KINET_DEFAULT_BUFLEN);
                result = read(new_socket, buffer, 511);
                if(result < 0)
                {
                    //std::cout << buffer;
                    throw std::runtime_error("Error Recieving a message!");
                }
                else
                {
                    //std::cout << buffer << '\n';   
                    message.assign(buffer, 512);
                    //message = buffer;
                }
                
            }
        private:
            int server, new_socket;
            socklen_t client_len;
            char buffer[KINET_DEFAULT_BUFLEN];
            struct sockaddr_in server_addr, client_addr;
            int result;
        };
    }
} // namespace kinet

int dummy_thicc_client(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        perror("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);


    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        perror("ERROR connecting");

    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);

    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         perror("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         perror("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}

int dummy_thicc_server(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     sockfd = socket(AF_INET, SOCK_STREAM, 0); //start the server
     if (sockfd < 0) 
        perror("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));

     portno = atoi(argv[1]); //get port
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) //bind port
              perror("ERROR on binding");


     listen(sockfd,5); //listen for connections?
     clilen = sizeof(cli_addr); 
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen); //accept client
     if (newsockfd < 0) 
          perror("ERROR on accept");


     bzero(buffer,256); //reads messages
     n = read(newsockfd,buffer,255);
     if (n < 0) perror("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);
     n = write(newsockfd,"I got your message",18);
     if (n < 0) perror("ERROR writing to socket");
     close(newsockfd);
     close(sockfd);
     return 0; 
}