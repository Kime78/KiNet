#include "KiNet.hpp"

int main() {
    kinet::client::Socket client;
    client.connect("localhost", "25565");
    int i = 0;
    while(true) {
        std::string test = "";
        std::cout << "Say something: ";
        std::cin >> test;
        client.send(test);
    }
    return 0;
}