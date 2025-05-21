#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <shared_mutex>
#include "logger.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 12345

std::shared_mutex socket_mutex;

void receive_thread(SOCKET sock, Logger& logger) { // прием сообщения
    char buffer[1024];
    while (true) {
        
        socket_mutex.lock_shared();
           int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        socket_mutex.unlock_shared();
        if (bytes <= 0) break;

        buffer[bytes] = '\0';
        std::string msg(buffer);
        std::cout << msg << std::endl;
        logger.write_Log("Received: " + msg);
    }
}

void run_client() {
    Logger logger("client.log");

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed.\n";
        return;
    }

    std::thread receiver(receive_thread, sock, std::ref(logger));

    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "/quit") break;
            socket_mutex.lock();
            send(sock, input.c_str(), input.length(), 0);
            socket_mutex.unlock(); 

        logger.write_Log("Sent: " + input);
    }

    closesocket(sock);
    receiver.detach(); // отсоединение потока
    WSACleanup();
}

int main() {
    run_client();
    return 0;
}
