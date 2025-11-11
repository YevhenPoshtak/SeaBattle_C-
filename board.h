#ifndef BOARD_H
#define BOARD_H

#include "common.h"
#include <vector>
#include <string>

class Board {
private:
    // Ігрове поле
    std::vector<std::vector<CellState>> grid;
    
    // Кораблі на дошці
    std::vector<Ship> ships;
    
    // Перевірка чи можна розмістити корабель
    bool canPlaceShip(const Ship& ship) const;
    
    // Перевірка чи клітинки навколо вільні (для правила сусідства)
    bool hasAdjacentShips(const Coordinate& coord) const;

public:
    // Конструктор
    Board();
    
    // Очистити дошку
    void clear();
    
    // Розмістити корабель
    bool placeShip(const Ship& ship);
    bool placeShip(ShipType type, Coordinate start, Orientation orientation);
    
    // Автоматичне розміщення всіх кораблів
    void placeShipsRandomly();
    
    // Постріл по координатам
    ShotResult shoot(const Coordinate& coord);
    
    // Отримати стан клітинки
    CellState getCell(const Coordinate& coord) const;
    CellState getCell(int row, int col) const;
    
    // Перевірка чи всі кораблі потоплені
    bool allShipsSunk() const;
    
    // Отримати кількість живих кораблів
    int getRemainingShips() const;
    
    // Отримати всі кораблі
    const std::vector<Ship>& getShips() const { return ships; }
    
    // Вивести дошку в консоль
    void display(bool hideShips = false) const;
    
    // Перевірка чи координата вже атакована
    bool isAttacked(const Coordinate& coord) const;
    
    // Отримати статистику
    int getTotalHits() const;
    int getTotalMisses() const;
    
    // Для дебагу - вивести дошку з кораблями
    void debugDisplay() const;
};

#endif // BOARD_H