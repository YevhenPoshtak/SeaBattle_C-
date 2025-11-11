#ifndef PLAYER_H
#define PLAYER_H

#include "common.h"
#include "board.h"
#include <string>

class Player {
protected:
    std::string name;
    Board ownBoard;      // Власна дошка з кораблями
    Board trackingBoard; // Дошка для відстеження пострілів по противнику
    int shotsCount;      // Кількість зроблених пострілів
    int hitsCount;       // Кількість влучань
    
public:
    // Конструктор
    Player(const std::string& playerName = "Player");
    
    // Віртуальний деструктор
    virtual ~Player() = default;
    
    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
    
    // Доступ до дошок
    Board& getOwnBoard() { return ownBoard; }
    const Board& getOwnBoard() const { return ownBoard; }
    
    Board& getTrackingBoard() { return trackingBoard; }
    const Board& getTrackingBoard() const { return trackingBoard; }
    
    // Розміщення кораблів
    virtual void placeShips();
    void placeShipsRandomly() { ownBoard.placeShipsRandomly(); }
    bool placeShip(ShipType type, Coordinate start, Orientation orientation);
    
    // Вибір координат для пострілу (віртуальний метод для AI)
    virtual Coordinate chooseTarget();
    
    // Отримання пострілу від противника
    ShotResult receiveShot(const Coordinate& coord);
    
    // Обробка результату власного пострілу
    void processShotResult(const Coordinate& coord, ShotResult result);
    
    // Перевірка програшу
    bool hasLost() const { return ownBoard.allShipsSunk(); }
    
    // Статистика
    int getShotsCount() const { return shotsCount; }
    int getHitsCount() const { return hitsCount; }
    int getMissesCount() const { return shotsCount - hitsCount; }
    double getAccuracy() const;
    
    // Відображення дошок
    void displayOwnBoard() const;
    void displayTrackingBoard() const;
    void displayBothBoards() const;
    
    // Скидання статистики
    void reset();
    
    // Вивід статистики
    void displayStats() const;
};

#endif // PLAYER_H