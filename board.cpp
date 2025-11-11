#include "board.h"
#include <iostream>
#include <random>
#include <algorithm>

Board::Board() {
    clear();
}

void Board::clear() {
    // Ініціалізуємо порожнє поле
    grid.clear();
    grid.resize(BOARD_SIZE, std::vector<CellState>(BOARD_SIZE, EMPTY));
    ships.clear();
}

bool Board::hasAdjacentShips(const Coordinate& coord) const {
    // Перевіряємо всі 8 сусідніх клітинок
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            
            int newRow = coord.row + dr;
            int newCol = coord.col + dc;
            
            if (newRow >= 0 && newRow < BOARD_SIZE && 
                newCol >= 0 && newCol < BOARD_SIZE) {
                if (grid[newRow][newCol] == SHIP) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Board::canPlaceShip(const Ship& ship) const {
    std::vector<Coordinate> coords = ship.getCoordinates();
    
    // Перевіряємо всі координати корабля
    for (const auto& coord : coords) {
        // Перевірка меж
        if (!coord.isValid()) {
            return false;
        }
        
        // Перевірка чи клітинка вільна
        if (grid[coord.row][coord.col] != EMPTY) {
            return false;
        }
        
        // Перевірка сусідів (кораблі не можуть торкатися)
        if (hasAdjacentShips(coord)) {
            return false;
        }
    }
    
    return true;
}

bool Board::placeShip(const Ship& ship) {
    if (!canPlaceShip(ship)) {
        return false;
    }
    
    // Розміщуємо корабель
    std::vector<Coordinate> coords = ship.getCoordinates();
    for (const auto& coord : coords) {
        grid[coord.row][coord.col] = SHIP;
    }
    
    ships.push_back(ship);
    return true;
}

bool Board::placeShip(ShipType type, Coordinate start, Orientation orientation) {
    Ship ship(type, start, orientation);
    return placeShip(ship);
}

void Board::placeShipsRandomly() {
    clear();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> posDist(0, BOARD_SIZE - 1);
    std::uniform_int_distribution<> orientDist(0, 1);
    
    // Розміщуємо кожен тип корабля зі стандартного флоту
    for (ShipType type : STANDARD_FLEET) {
        bool placed = false;
        int attempts = 0;
        const int maxAttempts = 1000;
        
        while (!placed && attempts < maxAttempts) {
            int row = posDist(gen);
            int col = posDist(gen);
            Orientation orient = static_cast<Orientation>(orientDist(gen));
            
            Ship ship(type, Coordinate(row, col), orient);
            placed = placeShip(ship);
            attempts++;
        }
        
        if (!placed) {
            // Якщо не вдалося розмістити, починаємо заново
            placeShipsRandomly();
            return;
        }
    }
}

ShotResult Board::shoot(const Coordinate& coord) {
    if (!coord.isValid()) {
        return SHOT_INVALID;
    }
    
    CellState& cell = grid[coord.row][coord.col];
    
    // Перевіряємо чи вже стріляли тут
    if (cell == MISS || cell == HIT) {
        return SHOT_INVALID;
    }
    
    // Промах
    if (cell == EMPTY) {
        cell = MISS;
        return SHOT_MISS;
    }
    
    // Влучання
    if (cell == SHIP) {
        cell = HIT;
        
        // Знаходимо корабель, в який влучили
        for (auto& ship : ships) {
            std::vector<Coordinate> shipCoords = ship.getCoordinates();
            for (const auto& shipCoord : shipCoords) {
                if (shipCoord == coord) {
                    ship.hits++;
                    
                    // Перевіряємо чи корабель потоплено
                    if (ship.isSunk()) {
                        // Перевіряємо чи всі кораблі потоплені
                        if (allShipsSunk()) {
                            return SHOT_WIN;
                        }
                        return SHOT_SUNK;
                    }
                    return SHOT_HIT;
                }
            }
        }
    }
    
    return SHOT_MISS;
}

CellState Board::getCell(const Coordinate& coord) const {
    if (!coord.isValid()) {
        return EMPTY;
    }
    return grid[coord.row][coord.col];
}

CellState Board::getCell(int row, int col) const {
    return getCell(Coordinate(row, col));
}

bool Board::allShipsSunk() const {
    for (const auto& ship : ships) {
        if (!ship.isSunk()) {
            return false;
        }
    }
    return true;
}

int Board::getRemainingShips() const {
    int count = 0;
    for (const auto& ship : ships) {
        if (!ship.isSunk()) {
            count++;
        }
    }
    return count;
}

void Board::display(bool hideShips) const {
    std::cout << "  ";
    for (int i = 0; i < BOARD_SIZE; i++) {
        std::cout << " " << i;
    }
    std::cout << "\n";
    
    for (int row = 0; row < BOARD_SIZE; row++) {
        std::cout << char('A' + row) << " ";
        
        for (int col = 0; col < BOARD_SIZE; col++) {
            CellState cell = grid[row][col];
            
            if (hideShips && cell == SHIP) {
                std::cout << " ~";  // Ховаємо кораблі противника
            } else {
                switch (cell) {
                    case EMPTY:
                        std::cout << " ~";
                        break;
                    case SHIP:
                        std::cout << Color::BLUE << " S" << Color::RESET;
                        break;
                    case MISS:
                        std::cout << Color::GRAY << " o" << Color::RESET;
                        break;
                    case HIT:
                        std::cout << Color::RED << " X" << Color::RESET;
                        break;
                }
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

bool Board::isAttacked(const Coordinate& coord) const {
    if (!coord.isValid()) {
        return false;
    }
    
    CellState cell = grid[coord.row][coord.col];
    return (cell == MISS || cell == HIT);
}

int Board::getTotalHits() const {
    int count = 0;
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            if (cell == HIT) {
                count++;
            }
        }
    }
    return count;
}

int Board::getTotalMisses() const {
    int count = 0;
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            if (cell == MISS) {
                count++;
            }
        }
    }
    return count;
}

void Board::debugDisplay() const {
    std::cout << "=== DEBUG: Board State ===\n";
    display(false);
    std::cout << "Ships: " << ships.size() << "\n";
    std::cout << "Remaining: " << getRemainingShips() << "\n";
    std::cout << "Hits: " << getTotalHits() << "\n";
    std::cout << "Misses: " << getTotalMisses() << "\n";
}