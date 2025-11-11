#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>

const int BOARD_SIZE = 10;

// Типи кораблів та їх розміри
enum ShipType {
    CARRIER = 5,      // Авіаносець
    BATTLESHIP = 4,   // Лінкор
    CRUISER = 3,      // Крейсер
    SUBMARINE = 3,    // Підводний човен
    DESTROYER = 2     // Есмінець
};

// Стан клітинки на дошці
enum CellState {
    EMPTY = 0,        // Порожня клітинка
    SHIP = 1,         // Частина корабля
    MISS = 2,         // Промах
    HIT = 3           // Влучання
};

// Орієнтація корабля
enum Orientation {
    HORIZONTAL = 0,
    VERTICAL = 1
};

// Структура для координат
struct Coordinate {
    int row;
    int col;
    
    Coordinate() : row(0), col(0) {}
    Coordinate(int r, int c) : row(r), col(c) {}
    
    bool isValid() const {
        return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
    }
    
    bool operator==(const Coordinate& other) const {
        return row == other.row && col == other.col;
    }
};

// Структура для опису корабля
struct Ship {
    ShipType type;
    int size;
    Coordinate start;
    Orientation orientation;
    int hits;  // Кількість влучань
    
    Ship() : type(DESTROYER), size(2), hits(0), orientation(HORIZONTAL) {}
    
    Ship(ShipType t, Coordinate s, Orientation o) 
        : type(t), size(static_cast<int>(t)), start(s), orientation(o), hits(0) {}
    
    bool isSunk() const {
        return hits >= size;
    }
    
    // Отримати всі координати, які займає корабель
    std::vector<Coordinate> getCoordinates() const {
        std::vector<Coordinate> coords;
        for (int i = 0; i < size; i++) {
            if (orientation == HORIZONTAL) {
                coords.push_back(Coordinate(start.row, start.col + i));
            } else {
                coords.push_back(Coordinate(start.row + i, start.col));
            }
        }
        return coords;
    }
};

// Результат пострілу
enum ShotResult {
    SHOT_MISS = 0,      // Промах
    SHOT_HIT = 1,       // Влучання
    SHOT_SUNK = 2,      // Корабель потоплено
    SHOT_INVALID = 3,   // Неправильний постріл (вже стріляли)
    SHOT_WIN = 4        // Перемога (всі кораблі потоплені)
};

// Кольори для консолі (опціонально)
namespace Color {
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string BLUE = "\033[34m";
    const std::string YELLOW = "\033[33m";
    const std::string CYAN = "\033[36m";
    const std::string GRAY = "\033[90m";
}

// Стандартна конфігурація флоту
const std::vector<ShipType> STANDARD_FLEET = {
    CARRIER,      // 1x5
    BATTLESHIP,   // 1x4
    CRUISER,      // 1x3
    SUBMARINE,    // 1x3
    DESTROYER     // 1x2
};

#endif // COMMON_H