#include "network.h"
#include "player.h"
#include "board.h"
#include <iostream>
#include <cstring>

// Функції UI
void clearScreen();
void pause();
void printTitle();
void displayVictory(const std::string& winner);
void displayMatchStats(const Player& player1, const Player& player2);

// ==================== NetworkManager Implementation ====================

NetworkManager::NetworkManager() : socket(INVALID_SOCKET_VALUE), connected(false) {
    initializeNetwork();
}

NetworkManager::~NetworkManager() {
    disconnect();
    cleanupNetwork();
}

bool NetworkManager::initializeNetwork() {
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        lastError = "WSAStartup failed: " + std::to_string(result);
        return false;
    }
#endif
    return true;
}

void NetworkManager::cleanupNetwork() {
#ifdef _WIN32
    WSACleanup();
#endif
}

bool NetworkManager::setSocketTimeout(int seconds) {
#ifdef _WIN32
    DWORD timeout = seconds * 1000;
    return setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == 0;
#else
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    return setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0;
#endif
}

bool NetworkManager::sendMessage(const NetworkMessage& msg) {
    if (!connected) {
        lastError = "Not connected";
        return false;
    }
    
    int bytesSent = send(socket, (const char*)&msg, sizeof(NetworkMessage), 0);
    if (bytesSent == SOCKET_ERROR_VALUE) {
        lastError = "Send failed";
        connected = false;
        return false;
    }
    
    return true;
}

bool NetworkManager::receiveMessage(NetworkMessage& msg) {
    if (!connected) {
        lastError = "Not connected";
        return false;
    }
    
    int bytesReceived = recv(socket, (char*)&msg, sizeof(NetworkMessage), 0);
    if (bytesReceived == SOCKET_ERROR_VALUE || bytesReceived == 0) {
        lastError = "Receive failed or connection closed";
        connected = false;
        return false;
    }
    
    return true;
}

void NetworkManager::disconnect() {
    if (socket != INVALID_SOCKET_VALUE) {
        closesocket(socket);
        socket = INVALID_SOCKET_VALUE;
    }
    connected = false;
}

// ==================== GameServer Implementation ====================

GameServer::GameServer(int serverPort) 
    : NetworkManager(), listenSocket(INVALID_SOCKET_VALUE), 
      clientSocket(INVALID_SOCKET_VALUE), port(serverPort) {
}

GameServer::~GameServer() {
    shutdown();
}

bool GameServer::start() {
    // Створюємо сокет для прослуховування
    listenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET_VALUE) {
        lastError = "Failed to create listen socket";
        return false;
    }
    
    // Дозволяємо повторне використання адреси
    int reuse = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
    
    // Налаштовуємо адресу сервера
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    // Прив'язуємо сокет до адреси
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR_VALUE) {
        lastError = "Bind failed";
        closesocket(listenSocket);
        return false;
    }
    
    // Починаємо прослуховування
    if (listen(listenSocket, 1) == SOCKET_ERROR_VALUE) {
        lastError = "Listen failed";
        closesocket(listenSocket);
        return false;
    }
    
    std::cout << Color::GREEN << "Сервер запущено на порті " << port << "\n" << Color::RESET;
    std::cout << Color::CYAN << "IP адреса: " << getServerIP() << "\n" << Color::RESET;
    
    return true;
}

bool GameServer::waitForClient() {
    std::cout << Color::YELLOW << "Очікування підключення клієнта...\n" << Color::RESET;
    
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET_VALUE) {
        lastError = "Accept failed";
        return false;
    }
    
    socket = clientSocket;
    connected = true;
    
    // Отримуємо IP клієнта
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
    
    std::cout << Color::GREEN << "Клієнт підключився: " << clientIP << "\n" << Color::RESET;
    
    return true;
}

std::string GameServer::getServerIP() const {
    return NetworkUtils::getLocalIPAddress();
}

void GameServer::shutdown() {
    if (clientSocket != INVALID_SOCKET_VALUE) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET_VALUE;
    }
    
    if (listenSocket != INVALID_SOCKET_VALUE) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET_VALUE;
    }
    
    disconnect();
}

// ==================== NetworkUtils Implementation ====================

namespace NetworkUtils {
    NetworkMessage createShotMessage(const Coordinate& coord) {
        return NetworkMessage(MSG_SHOT, coord.row, coord.col);
    }
    
    NetworkMessage createResultMessage(ShotResult result) {
        return NetworkMessage(MSG_RESULT, static_cast<int>(result));
    }
    
    Coordinate getCoordinateFromMessage(const NetworkMessage& msg) {
        return Coordinate(msg.data1, msg.data2);
    }
    
    ShotResult getResultFromMessage(const NetworkMessage& msg) {
        return static_cast<ShotResult>(msg.data1);
    }
    
    std::string getLocalIPAddress() {
        char hostBuffer[256];
        if (gethostname(hostBuffer, sizeof(hostBuffer)) == SOCKET_ERROR_VALUE) {
            return "127.0.0.1";
        }
        
        struct hostent* host = gethostbyname(hostBuffer);
        if (host == nullptr) {
            return "127.0.0.1";
        }
        
        struct in_addr** addrList = (struct in_addr**)host->h_addr_list;
        for (int i = 0; addrList[i] != nullptr; i++) {
            char* ip = inet_ntoa(*addrList[i]);
            if (ip && strcmp(ip, "127.0.0.1") != 0) {
                return std::string(ip);
            }
        }
        
        return "127.0.0.1";
    }
    
    bool isValidIPAddress(const std::string& ip) {
        struct sockaddr_in sa;
        return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) == 1;
    }
}

// ==================== NetworkPlayer Implementation ====================

NetworkPlayer::NetworkPlayer(const std::string& playerName, NetworkManager* net, bool server)
    : name(playerName), network(net), isServer(server) {
}

bool NetworkPlayer::sendShot(const Coordinate& coord) {
    NetworkMessage msg = NetworkUtils::createShotMessage(coord);
    return network->sendMessage(msg);
}

bool NetworkPlayer::receiveShot(Coordinate& coord) {
    NetworkMessage msg;
    if (!network->receiveMessage(msg)) {
        return false;
    }
    
    if (msg.type != MSG_SHOT) {
        return false;
    }
    
    coord = NetworkUtils::getCoordinateFromMessage(msg);
    return true;
}

bool NetworkPlayer::sendResult(ShotResult result) {
    NetworkMessage msg = NetworkUtils::createResultMessage(result);
    return network->sendMessage(msg);
}

bool NetworkPlayer::receiveResult(ShotResult& result) {
    NetworkMessage msg;
    if (!network->receiveMessage(msg)) {
        return false;
    }
    
    if (msg.type != MSG_RESULT) {
        return false;
    }
    
    result = NetworkUtils::getResultFromMessage(msg);
    return true;
}

bool NetworkPlayer::sendReady() {
    NetworkMessage msg(MSG_READY, 0, 0, name);
    return network->sendMessage(msg);
}

bool NetworkPlayer::receiveReady() {
    NetworkMessage msg;
    return network->receiveMessage(msg) && msg.type == MSG_READY;
}

bool NetworkPlayer::sendChatMessage(const std::string& message) {
    NetworkMessage msg(MSG_CHAT, 0, 0, message);
    return network->sendMessage(msg);
}

bool NetworkPlayer::receiveChatMessage(std::string& message) {
    NetworkMessage msg;
    if (!network->receiveMessage(msg)) {
        return false;
    }
    
    if (msg.type != MSG_CHAT) {
        return false;
    }
    
    message = std::string(msg.text);
    return true;
}

// ==================== Main Server Program ====================

void playNetworkGameAsServer() {
    GameServer server(DEFAULT_PORT);
    
    if (!server.start()) {
        std::cout << Color::RED << "Помилка запуску сервера: " 
                  << server.getLastError() << "\n" << Color::RESET;
        return;
    }
    
    if (!server.waitForClient()) {
        std::cout << Color::RED << "Помилка очікування клієнта: " 
                  << server.getLastError() << "\n" << Color::RESET;
        return;
    }
    
    // Створюємо гравця
    Player human;
    
    clearScreen();
    printTitle();
    
    std::cout << Color::CYAN << "Введіть ваше ім'я: " << Color::RESET;
    std::string name;
    std::getline(std::cin, name);
    if (name.empty()) name = "Сервер";
    human.setName(name);
    
    NetworkPlayer netPlayer(name, &server, true);
    
    // Відправляємо готовність
    std::cout << Color::YELLOW << "Відправка інформації про готовність...\n" << Color::RESET;
    if (!netPlayer.sendReady()) {
        std::cout << Color::RED << "Помилка відправки готовності\n" << Color::RESET;
        return;
    }
    
    // Очікуємо готовності від клієнта
    std::cout << Color::YELLOW << "Очікування готовності клієнта...\n" << Color::RESET;
    if (!netPlayer.receiveReady()) {
        std::cout << Color::RED << "Помилка отримання готовності\n" << Color::RESET;
        return;
    }
    
    clearScreen();
    std::cout << Color::GREEN << "Обидва гравці готові! Починаємо гру...\n" << Color::RESET;
    pause();
    
    // Розміщуємо кораблі
    clearScreen();
    human.placeShips();
    
    // Основний ігровий цикл
    bool gameOver = false;
    bool myTurn = true; // Сервер ходить першим
    int turnNumber = 1;
    
    while (!gameOver && server.isConnected()) {
        clearScreen();
        
        if (myTurn) {
            // Наш хід
            std::cout << Color::CYAN << "=== ВАШ ХІД #" << turnNumber << " ===\n" << Color::RESET;
            human.displayBothBoards();
            
            Coordinate target = human.chooseTarget();
            
            // Відправляємо постріл
            if (!netPlayer.sendShot(target)) {
                std::cout << Color::RED << "Помилка відправки пострілу\n" << Color::RESET;
                break;
            }
            
            // Отримуємо результат
            ShotResult result;
            if (!netPlayer.receiveResult(result)) {
                std::cout << Color::RED << "Помилка отримання результату\n" << Color::RESET;
                break;
            }
            
            human.processShotResult(target, result);
            human.displayTrackingBoard();
            
            if (result == SHOT_WIN) {
                gameOver = true;
                displayVictory(human.getName());
            } else {
                pause();
                myTurn = false;
            }
            
        } else {
            // Хід противника
            std::cout << Color::YELLOW << "=== ХІД ПРОТИВНИКА #" << turnNumber << " ===\n" << Color::RESET;
            std::cout << "Очікування пострілу...\n";
            
            Coordinate target;
            if (!netPlayer.receiveShot(target)) {
                std::cout << Color::RED << "Помилка отримання пострілу\n" << Color::RESET;
                break;
            }
            
            std::cout << "Противник стріляє по " << char('A' + target.row) << target.col << "\n";
            
            ShotResult result = human.receiveShot(target);
            
            // Відправляємо результат
            if (!netPlayer.sendResult(result)) {
                std::cout << Color::RED << "Помилка відправки результату\n" << Color::RESET;
                break;
            }
            
            human.displayOwnBoard();
            
            if (result == SHOT_WIN) {
                gameOver = true;
                displayVictory("Противник");
            } else {
                pause();
                myTurn = true;
                turnNumber++;
            }
        }
    }
    
    if (!server.isConnected()) {
        std::cout << Color::RED << "З'єднання втрачено\n" << Color::RESET;
    }
    
    pause();
    server.shutdown();
}

int main() {
    #ifdef _WIN32
        system("chcp 65001 > nul");
    #endif
    
    clearScreen();
    printTitle();
    
    std::cout << Color::CYAN << "🌐 РЕЖИМ СЕРВЕРА 🌐\n" << Color::RESET;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";
    
    playNetworkGameAsServer();
    
    std::cout << Color::CYAN << "\nСервер завершив роботу. До побачення!\n" << Color::RESET;
    
    return 0;
}