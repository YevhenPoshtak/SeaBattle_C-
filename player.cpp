#include "player.h"
#include <iostream>
#include <sstream>
#include <cctype>

Player::Player(const std::string& playerName) 
    : name(playerName), shotsCount(0), hitsCount(0) {
}

bool Player::placeShip(ShipType type, Coordinate start, Orientation orientation) {
    return ownBoard.placeShip(type, start, orientation);
}

void Player::placeShips() {
    std::cout << "\n" << Color::CYAN << name << ", розміщуємо кораблі!" << Color::RESET << "\n\n";
    
    // Показуємо які кораблі треба розмістити
    std::cout << "Кораблі для розміщення:\n";
    std::cout << "  1. Авіаносець (5 клітинок)\n";
    std::cout << "  2. Лінкор (4 клітинки)\n";
    std::cout << "  3. Крейсер (3 клітинки)\n";
    std::cout << "  4. Підводний човен (3 клітинки)\n";
    std::cout << "  5. Есмінець (2 клітинки)\n\n";
    
    std::cout << "Введіть 'auto' для автоматичного розміщення\n\n";
    
    std::string input;
    std::getline(std::cin, input);
    
    // Перевіряємо чи користувач хоче автоматичне розміщення
    if (input == "auto" || input == "AUTO" || input == "а" || input == "Auto") {
        ownBoard.placeShipsRandomly();
        std::cout << Color::GREEN << "Кораблі розміщено автоматично!\n" << Color::RESET;
        displayOwnBoard();
        return;
    }
    
    // Ручне розміщення
    for (ShipType type : STANDARD_FLEET) {
        bool placed = false;
        
        while (!placed) {
            ownBoard.display(false);
            
            std::string shipName;
            switch (type) {
                case CARRIER: shipName = "Авіаносець (5)"; break;
                case BATTLESHIP: shipName = "Лінкор (4)"; break;
                case CRUISER: shipName = "Крейсер (3)"; break;
                case SUBMARINE: shipName = "Підводний човен (3)"; break;
                case DESTROYER: shipName = "Есмінець (2)"; break;
            }
            
            std::cout << "\nРозміщуємо: " << Color::YELLOW << shipName << Color::RESET << "\n";
            std::cout << "Введіть координати (наприклад, A5) та орієнтацію (h/v): ";
            
            std::string coordStr, orientStr;
            std::cin >> coordStr >> orientStr;
            std::cin.ignore();
            
            // Парсимо координати
            if (coordStr.length() < 2) {
                std::cout << Color::RED << "Неправильний формат! Спробуйте ще раз.\n" << Color::RESET;
                continue;
            }
            
            char rowChar = std::toupper(coordStr[0]);
            int row = rowChar - 'A';
            int col;
            
            try {
                col = std::stoi(coordStr.substr(1));
            } catch (...) {
                std::cout << Color::RED << "Неправильна колонка! Спробуйте ще раз.\n" << Color::RESET;
                continue;
            }
            
            Coordinate start(row, col);
            
            if (!start.isValid()) {
                std::cout << Color::RED << "Координати поза межами дошки!\n" << Color::RESET;
                continue;
            }
            
            // Парсимо орієнтацію
            Orientation orient;
            if (orientStr == "h" || orientStr == "H" || orientStr == "г" || orientStr == "Г") {
                orient = HORIZONTAL;
            } else if (orientStr == "v" || orientStr == "V" || orientStr == "в" || orientStr == "В") {
                orient = VERTICAL;
            } else {
                std::cout << Color::RED << "Неправильна орієнтація! Використовуйте h (горизонтально) або v (вертикально).\n" << Color::RESET;
                continue;
            }
            
            // Пробуємо розмістити корабель
            Ship ship(type, start, orient);
            if (ownBoard.placeShip(ship)) {
                placed = true;
                std::cout << Color::GREEN << "Корабель розміщено!\n" << Color::RESET;
            } else {
                std::cout << Color::RED << "Неможливо розмістити корабель тут! Перевірте:\n" << Color::RESET;
                std::cout << "  - Чи корабель не виходить за межі дошки\n";
                std::cout << "  - Чи немає інших кораблів поруч\n";
            }
        }
    }
    
    std::cout << Color::GREEN << "\nВсі кораблі розміщено!\n" << Color::RESET;
    displayOwnBoard();
}

Coordinate Player::chooseTarget() {
    trackingBoard.display(true);
    
    std::cout << "\n" << Color::CYAN << name << ", ваш хід!" << Color::RESET << "\n";
    std::cout << "Введіть координати для пострілу (наприклад, A5): ";
    
    std::string input;
    std::cin >> input;
    std::cin.ignore();
    
    if (input.length() < 2) {
        std::cout << Color::RED << "Неправильний формат!\n" << Color::RESET;
        return Coordinate(-1, -1);
    }
    
    char rowChar = std::toupper(input[0]);
    int row = rowChar - 'A';
    int col;
    
    try {
        col = std::stoi(input.substr(1));
    } catch (...) {
        std::cout << Color::RED << "Неправильна колонка!\n" << Color::RESET;
        return Coordinate(-1, -1);
    }
    
    return Coordinate(row, col);
}

ShotResult Player::receiveShot(const Coordinate& coord) {
    return ownBoard.shoot(coord);
}

void Player::processShotResult(const Coordinate& coord, ShotResult result) {
    shotsCount++;
    
    // Оновлюємо tracking board
    switch (result) {
        case SHOT_MISS:
            // Маркуємо як промах на tracking board
            trackingBoard.shoot(coord);
            std::cout << Color::GRAY << "Промах!\n" << Color::RESET;
            break;
            
        case SHOT_HIT:
            hitsCount++;
            // Маркуємо як влучання на tracking board
            trackingBoard.shoot(coord);
            std::cout << Color::YELLOW << "Влучання!\n" << Color::RESET;
            break;
            
        case SHOT_SUNK:
            hitsCount++;
            trackingBoard.shoot(coord);
            std::cout << Color::RED << "Корабель потоплено!\n" << Color::RESET;
            break;
            
        case SHOT_WIN:
            hitsCount++;
            trackingBoard.shoot(coord);
            std::cout << Color::GREEN << "Всі кораблі противника знищено! Перемога!\n" << Color::RESET;
            break;
            
        case SHOT_INVALID:
            shotsCount--; // Не рахуємо невалідний постріл
            std::cout << Color::RED << "Ви вже стріляли сюди! Спробуйте інші координати.\n" << Color::RESET;
            break;
    }
}

double Player::getAccuracy() const {
    if (shotsCount == 0) return 0.0;
    return (static_cast<double>(hitsCount) / shotsCount) * 100.0;
}

void Player::displayOwnBoard() const {
    std::cout << Color::CYAN << name << " - Ваша дошка:\n" << Color::RESET;
    ownBoard.display(false);
}

void Player::displayTrackingBoard() const {
    std::cout << Color::CYAN << name << " - Постріли по противнику:\n" << Color::RESET;
    trackingBoard.display(true);
}

void Player::displayBothBoards() const {
    std::cout << "\n╔═══════════════════════════════════════════════════╗\n";
    std::cout << "║  " << Color::CYAN << name << Color::RESET << "\n";
    std::cout << "╠═══════════════════════════════════════════════════╣\n";
    
    std::cout << "\n  Ваші кораблі:\n";
    ownBoard.display(false);
    
    std::cout << "\n  Ваші постріли:\n";
    trackingBoard.display(true);
    
    std::cout << "╚═══════════════════════════════════════════════════╝\n\n";
}

void Player::reset() {
    ownBoard.clear();
    trackingBoard.clear();
    shotsCount = 0;
    hitsCount = 0;
}

void Player::displayStats() const {
    std::cout << "\n╔═══════════════════════════════════════════════════╗\n";
    std::cout << "║  " << Color::CYAN << "Статистика: " << name << Color::RESET << "\n";
    std::cout << "╠═══════════════════════════════════════════════════╣\n";
    std::cout << "║  Зроблено пострілів: " << shotsCount << "\n";
    std::cout << "║  Влучань: " << Color::GREEN << hitsCount << Color::RESET << "\n";
    std::cout << "║  Промахів: " << Color::GRAY << getMissesCount() << Color::RESET << "\n";
    std::cout << "║  Точність: " << Color::YELLOW << getAccuracy() << "%" << Color::RESET << "\n";
    std::cout << "║  Залишилось кораблів противника: " << (5 - ownBoard.getRemainingShips()) << "/5\n";
    std::cout << "╚═══════════════════════════════════════════════════╝\n\n";
}