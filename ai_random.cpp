#include "ai.h"
#include <iostream>
#include <algorithm>

// ==================== AIPlayer (базовий клас) ====================

AIPlayer::AIPlayer(const std::string& aiName) 
    : Player(aiName) {
    std::random_device rd;
    rng.seed(rd());
}

void AIPlayer::placeShips() {
    std::cout << Color::CYAN << name << " розміщує кораблі...\n" << Color::RESET;
    ownBoard.placeShipsRandomly();
    std::cout << Color::GREEN << "Кораблі розміщено!\n" << Color::RESET;
}

// ==================== RandomAI ====================

RandomAI::RandomAI(const std::string& aiName) 
    : AIPlayer(aiName) {
    initializeTargets();
}

void RandomAI::initializeTargets() {
    availableTargets.clear();
    
    // Заповнюємо всі можливі координати
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            availableTargets.push_back(Coordinate(row, col));
        }
    }
    
    // Перемішуємо для випадковості
    std::shuffle(availableTargets.begin(), availableTargets.end(), rng);
}

void RandomAI::removeTarget(const Coordinate& coord) {
    // Видаляємо координату зі списку доступних
    availableTargets.erase(
        std::remove_if(availableTargets.begin(), availableTargets.end(),
            [&coord](const Coordinate& c) {
                return c.row == coord.row && c.col == coord.col;
            }),
        availableTargets.end()
    );
}

Coordinate RandomAI::chooseTarget() {
    std::cout << Color::CYAN << name << " обирає ціль...\n" << Color::RESET;
    
    // Невелика затримка для реалістичності
    #ifdef _WIN32
        Sleep(500);  // Windows
    #else
        usleep(500000);  // Unix/Linux
    #endif
    
    if (availableTargets.empty()) {
        // Якщо всі цілі вичерпані (не повинно статися в нормальній грі)
        std::cerr << "Error: No available targets!\n";
        return Coordinate(-1, -1);
    }
    
    // Беремо останню ціль і видаляємо її
    Coordinate target = availableTargets.back();
    availableTargets.pop_back();
    
    std::cout << Color::YELLOW << name << " стріляє по " 
              << char('A' + target.row) << target.col << "\n" << Color::RESET;
    
    return target;
}