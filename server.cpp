#include "KiNet.hpp"

int main() {
    kinet::server::Socket server;
    server.start("localhost", "25565");
    server.accept();
    while (true)
    {
        std::string test = "";
        server.receive_from_id(test, 0);
        if(test != "")  {
            std::cout << test;
        }
    }
    
    
    return 0;
}