#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"
#include <string>
#include <vector>

// Платформо-залежні заголовки для сокетів
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET SocketType;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    typedef int SocketType;
    #define INVALID_SOCKET_VALUE -1
    #define SOCKET_ERROR_VALUE -1
    #define closesocket close
#endif

// Константи
const int DEFAULT_PORT = 8888;
const int MAX_BUFFER_SIZE = 1024;
const int CONNECTION_TIMEOUT = 30; // секунди

// Типи повідомлень
enum MessageType {
    MSG_CONNECT = 0,        // З'єднання
    MSG_READY = 1,          // Гравець готовий
    MSG_SHOT = 2,           // Постріл
    MSG_RESULT = 3,         // Результат пострілу
    MSG_CHAT = 4,           // Чат
    MSG_DISCONNECT = 5,     // Від'єднання
    MSG_GAME_OVER = 6,      // Гра закінчена
    MSG_PING = 7,           // Перевірка з'єднання
    MSG_ERROR = 8           // Помилка
};

// Структура повідомлення
struct NetworkMessage {
    MessageType type;
    int data1;  // Використовується для row, result, тощо
    int data2;  // Використовується для col
    char text[256];  // Для імені, повідомлень чату тощо
    
    NetworkMessage() : type(MSG_CONNECT), data1(0), data2(0) {
        text[0] = '\0';
    }
    
    NetworkMessage(MessageType t, int d1 = 0, int d2 = 0, const std::string& txt = "") 
        : type(t), data1(d1), data2(d2) {
        strncpy(text, txt.c_str(), sizeof(text) - 1);
        text[sizeof(text) - 1] = '\0';
    }
};

// Базовий клас для мережевої комунікації
class NetworkManager {
protected:
    SocketType socket;
    bool connected;
    std::string lastError;
    
    // Ініціалізація мережі (Windows specific)
    bool initializeNetwork();
    
    // Очистка мережі
    void cleanupNetwork();
    
    // Встановити таймаут для сокету
    bool setSocketTimeout(int seconds);
    
public:
    NetworkManager();
    virtual ~NetworkManager();
    
    // Відправка повідомлення
    bool sendMessage(const NetworkMessage& msg);
    
    // Отримання повідомлення
    bool receiveMessage(NetworkMessage& msg);
    
    // Перевірка з'єднання
    bool isConnected() const { return connected; }
    
    // Отримати останню помилку
    std::string getLastError() const { return lastError; }
    
    // Закрити з'єднання
    virtual void disconnect();
};

// Серверний клас
class GameServer : public NetworkManager {
private:
    SocketType listenSocket;
    SocketType clientSocket;
    int port;
    
public:
    GameServer(int serverPort = DEFAULT_PORT);
    ~GameServer();
    
    // Запустити сервер та очікувати на з'єднання
    bool start();
    
    // Очікування підключення клієнта
    bool waitForClient();
    
    // Отримати IP адресу сервера
    std::string getServerIP() const;
    
    // Закрити сервер
    void shutdown();
};

// Клієнтський клас
class GameClient : public NetworkManager {
private:
    std::string serverAddress;
    int port;
    
public:
    GameClient();
    ~GameClient();
    
    // Підключитися до сервера
    bool connect(const std::string& address, int serverPort = DEFAULT_PORT);
    
    // Відключитися від сервера
    void disconnect() override;
};

// Допоміжні функції
namespace NetworkUtils {
    // Конвертація координат в повідомлення
    NetworkMessage createShotMessage(const Coordinate& coord);
    
    // Конвертація результату в повідомлення
    NetworkMessage createResultMessage(ShotResult result);
    
    // Отримати координату з повідомлення
    Coordinate getCoordinateFromMessage(const NetworkMessage& msg);
    
    // Отримати результат з повідомлення
    ShotResult getResultFromMessage(const NetworkMessage& msg);
    
    // Отримати локальну IP адресу
    std::string getLocalIPAddress();
    
    // Перевірити валідність IP адреси
    bool isValidIPAddress(const std::string& ip);
}

// Клас для гравця в мережевій грі
class NetworkPlayer {
private:
    std::string name;
    NetworkManager* network;
    bool isServer;
    
public:
    NetworkPlayer(const std::string& playerName, NetworkManager* net, bool server);
    
    // Відправити постріл
    bool sendShot(const Coordinate& coord);
    
    // Отримати постріл від противника
    bool receiveShot(Coordinate& coord);
    
    // Відправити результат пострілу
    bool sendResult(ShotResult result);
    
    // Отримати результат пострілу
    bool receiveResult(ShotResult& result);
    
    // Відправити повідомлення готовності
    bool sendReady();
    
    // Отримати повідомлення готовності
    bool receiveReady();
    
    // Відправити повідомлення в чат
    bool sendChatMessage(const std::string& message);
    
    // Отримати повідомлення з чату
    bool receiveChatMessage(std::string& message);
    
    std::string getName() const { return name; }
};

#endif // NETWORK_H