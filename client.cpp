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

// ==================== GameClient Implementation ====================

GameClient::GameClient() 
    : NetworkManager(), serverAddress(""), port(DEFAULT_PORT) {
}

GameClient::~GameClient() {
    disconnect();
}

bool GameClient::connect(const std::string& address, int serverPort) {
    serverAddress = address;
    port = serverPort;
    
    // Створюємо сокет
    socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket == INVALID_SOCKET_VALUE) {
        lastError = "Failed to create socket";
        return false;
    }
    
    // Налаштовуємо адресу сервера
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    // Конвертуємо IP адресу
    if (inet_pton(AF_INET, serverAddress.c_str(), &serverAddr.sin_addr) <= 0) {
        lastError = "Invalid address";
        closesocket(socket);
        socket = INVALID_SOCKET_VALUE;
        return false;
    }
    
    std::cout << Color::YELLOW << "Підключення до " << serverAddress 
              << ":" << port << "...\n" << Color::RESET;
    
    // Підключаємося до сервера
    if (::connect(socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR_VALUE) {
        lastError = "Connection failed";
        closesocket(socket);
        socket = INVALID_SOCKET_VALUE;
        return false;
    }
    
    connected = true;
    std::cout << Color::GREEN << "Успішно підключено до сервера!\n" << Color::RESET;
    
    return true;
}

void GameClient::disconnect() {
    if (connected) {
        NetworkMessage msg(MSG_DISCONNECT);
        sendMessage(msg);
    }
    NetworkManager::disconnect();
    serverAddress = "";
}

// ==================== Main Client Program ====================

void playNetworkGameAsClient() {
    GameClient client;
    
    clearScreen();
    printTitle();
    
    std::cout << Color::CYAN << "🌐 ПІДКЛЮЧЕННЯ ДО СЕРВЕРА 🌐\n" << Color::RESET;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";
    
    // Введення IP адреси сервера
    std::string serverIP;
    std::cout << Color::YELLOW << "Введіть IP адресу сервера: " << Color::RESET;
    std::getline(std::cin, serverIP);
    
    if (serverIP.empty()) {
        serverIP = "127.0.0.1"; // localhost за замовчуванням
        std::cout << Color::CYAN << "Використовується localhost (127.0.0.1)\n" << Color::RESET;
    }
    
    // Валідація IP
    if (!NetworkUtils::isValidIPAddress(serverIP)) {
        std::cout << Color::RED << "Неправильний формат IP адреси!\n" << Color::RESET;
        pause();
        return;
    }
    
    // Введення порту (опціонально)
    int port = DEFAULT_PORT;
    std::cout << Color::YELLOW << "Введіть порт (Enter для " << DEFAULT_PORT << "): " << Color::RESET;
    std::string portStr;
    std::getline(std::cin, portStr);
    
    if (!portStr.empty()) {
        try {
            port = std::stoi(portStr);
        } catch (...) {
            std::cout << Color::RED << "Неправильний порт, використовується " 
                      << DEFAULT_PORT << "\n" << Color::RESET;
            port = DEFAULT_PORT;
        }
    }
    
    // Підключення до сервера
    if (!client.connect(serverIP, port)) {
        std::cout << Color::RED << "Помилка підключення: " 
                  << client.getLastError() << "\n" << Color::RESET;
        std::cout << "\nПереконайтесь що:\n";
        std::cout << "  • Сервер запущений\n";
        std::cout << "  • IP адреса правильна\n";
        std::cout << "  • Порт не заблокований фаєрволом\n";
        pause();
        return;
    }
    
    // Створюємо гравця
    Player human;
    
    clearScreen();
    printTitle();
    
    std::cout << Color::CYAN << "Введіть ваше ім'я: " << Color::RESET;
    std::string name;
    std::getline(std::cin, name);
    if (name.empty()) name = "Клієнт";
    human.setName(name);
    
    NetworkPlayer netPlayer(name, &client, false);
    
    // Очікуємо готовності від сервера
    std::cout << Color::YELLOW << "Очікування готовності сервера...\n" << Color::RESET;
    if (!netPlayer.receiveReady()) {
        std::cout << Color::RED << "Помилка отримання готовності\n" << Color::RESET;
        return;
    }
    
    // Відправляємо свою готовність
    std::cout << Color::YELLOW << "Відправка інформації про готовність...\n" << Color::RESET;
    if (!netPlayer.sendReady()) {
        std::cout << Color::RED << "Помилка відправки готовності\n" << Color::RESET;
        return;
    }
    
    clearScreen();
    std::cout << Color::GREEN << "Обидва гравці готові! Починаємо гру...\n" << Color::RESET;
    pause();
    
    // Розміщуємо кораблі
    clearScreen();
    human.placeShips();
    
    std::cout << Color::YELLOW << "\nОчікування початку гри...\n" << Color::RESET;
    
    // Основний ігровий цикл
    bool gameOver = false;
    bool myTurn = false; // Клієнт ходить другим
    int turnNumber = 1;
    
    while (!gameOver && client.isConnected()) {
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
                turnNumber++;
            }
            
        } else {
            // Хід противника
            std::cout << Color::YELLOW << "=== ХІД ПРОТИВНИКА #" << turnNumber << " ===\n" << Color::RESET;
            std::cout << "Очікування пострілу від сервера...\n";
            
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
            }
        }
    }
    
    if (!client.isConnected()) {
        std::cout << Color::RED << "З'єднання втрачено\n" << Color::RESET;
    }
    
    pause();
    client.disconnect();
}

int main() {
    #ifdef _WIN32
        system("chcp 65001 > nul");
    #endif
    
    bool keepPlaying = true;
    
    while (keepPlaying) {
        playNetworkGameAsClient();
        
        std::cout << "\n" << Color::YELLOW << "Спробувати підключитися знову? (y/n): " << Color::RESET;
        std::string answer;
        std::getline(std::cin, answer);
        
        keepPlaying = (answer == "y" || answer == "Y" || answer == "yes" || 
                      answer == "т" || answer == "Т" || answer == "так");
    }
    
    clearScreen();
    std::cout << Color::CYAN;
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                            ║\n";
    std::cout << "║              Дякуємо за гру! До побачення! 👋             ║\n";
    std::cout << "║                                                            ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    std::cout << Color::RESET << "\n";
    
    return 0;
}