#include <iostream> // ввод-вывод
#include <string> // строки
#include <thread> // потоки
#include <vector> // векторы
#include <algorithm> // алгоритмы
#include <winsock2.h> // для работы с сокетами в Windows
#include <ws2tcpip.h> // для работы с TCP/IP в Windows
#include <shared_mutex> // работа с shared_mutex
#include "logger.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 12345

std::vector<SOCKET> clients;
std::shared_mutex clients_mutex;

void broadcast(const std::string& message, SOCKET sender) { //отправка сообщения всем клиентам кроме отправителя
    clients_mutex.lock_shared();
    for (SOCKET client : clients) {
        if (client != sender) {
            send(client, message.c_str(), message.length(), 0);
        }
    }
    clients_mutex.unlock_shared();
}

void handle_client(SOCKET client_socket, Logger& logger) { // обработка соединения с клиентом 
    char buffer[1024];
    while (true) {
        int bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0); //получение данных от клиента
        if (bytes <= 0) break;

        buffer[bytes] = '\0';
        std::string msg = "Client " + std::to_string((int)client_socket) + ": " + buffer; //формирование сообщения
        std::cout << msg << std::endl;
        logger.write_Log(msg);
        broadcast(msg, client_socket);
    }

    closesocket(client_socket);
    logger.write_Log("Client disconnected: " + std::to_string((int)client_socket));

    clients_mutex.lock(); 
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end()); // удаление клиента 
    clients_mutex.unlock(); 
}

void run_server() {
    Logger logger("server.log");

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, SOMAXCONN);
    logger.write_Log("Server started");

    while (true) {
        SOCKET client_socket = accept(server_fd, nullptr, nullptr);
            
            clients_mutex.lock(); 
            clients.push_back(client_socket); //добавление клиента в вектор
            clients_mutex.unlock();

        logger.write_Log("New client connected.");
        std::thread(handle_client, client_socket, std::ref(logger)).detach();
    }

    closesocket(server_fd);
    WSACleanup();
}

int main() {
    run_server();
    return 0;
}