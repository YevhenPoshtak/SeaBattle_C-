#include "ai.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// ==================== SmartAI ====================

SmartAI::SmartAI(const std::string& aiName) 
    : AIPlayer(aiName), currentMode(HUNT), lastHit(-1, -1) {
}

std::vector<Coordinate> SmartAI::getAdjacentCells(const Coordinate& coord) const {
    std::vector<Coordinate> adjacent;
    
    // Перевіряємо 4 сусідні клітинки (вгору, вниз, ліворуч, праворуч)
    const int directions[][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (const auto& dir : directions) {
        Coordinate newCoord(coord.row + dir[0], coord.col + dir[1]);
        if (newCoord.isValid()) {
            adjacent.push_back(newCoord);
        }
    }
    
    return adjacent;
}

void SmartAI::addAdjacentTargets(const Coordinate& coord) {
    std::vector<Coordinate> adjacent = getAdjacentCells(coord);
    
    for (const auto& target : adjacent) {
        // Додаємо тільки якщо ще не атакували
        if (isValidTarget(target)) {
            targetQueue.push(target);
        }
    }
}

bool SmartAI::isValidTarget(const Coordinate& coord) const {
    if (!coord.isValid()) {
        return false;
    }
    
    // Перевіряємо чи ця клітинка вже атакована
    return !trackingBoard.isAttacked(coord);
}

void SmartAI::analyzeShipDirection() {
    if (currentShipHits.size() < 2) {
        return;
    }
    
    // Визначаємо напрямок корабля
    bool horizontal = (currentShipHits[0].row == currentShipHits[1].row);
    
    // Очищаємо чергу
    while (!targetQueue.empty()) {
        targetQueue.pop();
    }
    
    // Знаходимо крайні точки
    if (horizontal) {
        int minCol = currentShipHits[0].col;
        int maxCol = currentShipHits[0].col;
        int row = currentShipHits[0].row;
        
        for (const auto& hit : currentShipHits) {
            minCol = std::min(minCol, hit.col);
            maxCol = std::max(maxCol, hit.col);
        }
        
        // Додаємо цілі на кінцях
        Coordinate left(row, minCol - 1);
        Coordinate right(row, maxCol + 1);
        
        if (isValidTarget(left)) targetQueue.push(left);
        if (isValidTarget(right)) targetQueue.push(right);
    } else {
        int minRow = currentShipHits[0].row;
        int maxRow = currentShipHits[0].row;
        int col = currentShipHits[0].col;
        
        for (const auto& hit : currentShipHits) {
            minRow = std::min(minRow, hit.row);
            maxRow = std::max(maxRow, hit.row);
        }
        
        // Додаємо цілі на кінцях
        Coordinate top(minRow - 1, col);
        Coordinate bottom(maxRow + 1, col);
        
        if (isValidTarget(top)) targetQueue.push(top);
        if (isValidTarget(bottom)) targetQueue.push(bottom);
    }
}

Coordinate SmartAI::getSmartHuntTarget() {
    // Стратегія "шахової дошки" - стріляємо тільки по чорних клітинках
    // Це оптимально, оскільки найменший корабель має розмір 2
    
    std::vector<Coordinate> checkerboardTargets;
    
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            // Вибираємо клітинки де (row + col) парне
            if ((row + col) % 2 == 0) {
                Coordinate coord(row, col);
                if (isValidTarget(coord)) {
                    checkerboardTargets.push_back(coord);
                }
            }
        }
    }
    
    // Якщо шахові клітинки закінчились, беремо будь-яку доступну
    if (checkerboardTargets.empty()) {
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                Coordinate coord(row, col);
                if (isValidTarget(coord)) {
                    checkerboardTargets.push_back(coord);
                }
            }
        }
    }
    
    if (checkerboardTargets.empty()) {
        return Coordinate(-1, -1);
    }
    
    // Вибираємо випадкову ціль з доступних
    std::uniform_int_distribution<> dist(0, checkerboardTargets.size() - 1);
    return checkerboardTargets[dist(rng)];
}

Coordinate SmartAI::chooseTarget() {
    std::cout << Color::CYAN << name << " аналізує ситуацію...\n" << Color::RESET;
    
    // Невелика затримка для реалістичності
    #ifdef _WIN32
        Sleep(700);  // Windows
    #else
        usleep(700000);  // Unix/Linux
    #endif
    
    Coordinate target;
    
    if (currentMode == TARGET && !targetQueue.empty()) {
        // Режим добивання - беремо ціль з черги
        target = targetQueue.front();
        targetQueue.pop();
        
        std::cout << Color::YELLOW << name << " продовжує добивати корабель -> " 
                  << char('A' + target.row) << target.col << "\n" << Color::RESET;
    } else {
        // Режим пошуку - використовуємо розумну стратегію
        currentMode = HUNT;
        target = getSmartHuntTarget();
        
        if (!target.isValid()) {
            std::cerr << "Error: No valid targets available!\n";
            return Coordinate(-1, -1);
        }
        
        std::cout << Color::YELLOW << name << " шукає кораблі -> " 
                  << char('A' + target.row) << target.col << "\n" << Color::RESET;
    }
    
    return target;
}

void SmartAI::updateAfterShot(const Coordinate& coord, ShotResult result) {
    switch (result) {
        case SHOT_HIT:
            // Влучання - переходимо в режим добивання
            currentMode = TARGET;
            lastHit = coord;
            currentShipHits.push_back(coord);
            
            // Аналізуємо напрямок якщо є >= 2 влучань
            if (currentShipHits.size() >= 2) {
                analyzeShipDirection();
            } else {
                // Перше влучання - додаємо всі сусідні клітинки
                addAdjacentTargets(coord);
            }
            
            std::cout << Color::GREEN << "AI: Влучання! Продовжую атаку...\n" << Color::RESET;
            break;
            
        case SHOT_SUNK:
            // Корабель потоплено - повертаємось до пошуку
            currentMode = HUNT;
            currentShipHits.clear();
            
            // Очищаємо чергу цілей
            while (!targetQueue.empty()) {
                targetQueue.pop();
            }
            
            std::cout << Color::RED << "AI: Корабель знищено! Шукаю наступну ціль...\n" << Color::RESET;
            break;
            
        case SHOT_MISS:
            std::cout << Color::GRAY << "AI: Промах...\n" << Color::RESET;
            
            // Якщо в режимі добивання і черга порожня, повертаємось до пошуку
            if (currentMode == TARGET && targetQueue.empty()) {
                currentMode = HUNT;
            }
            break;
            
        case SHOT_WIN:
            std::cout << Color::GREEN << "AI переміг!\n" << Color::RESET;
            break;
            
        case SHOT_INVALID:
            std::cerr << "AI Error: Invalid shot!\n";
            break;
    }
}