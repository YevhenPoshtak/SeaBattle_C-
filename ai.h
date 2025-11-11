#ifndef AI_H
#define AI_H

#include "player.h"
#include <random>
#include <vector>
#include <queue>

// Базовий клас для AI гравців
class AIPlayer : public Player {
protected:
    std::mt19937 rng;
    
public:
    AIPlayer(const std::string& aiName = "AI");
    virtual ~AIPlayer() = default;
    
    // Автоматичне розміщення кораблів
    void placeShips() override;
    
    // Вибір цілі - чисто віртуальний метод
    virtual Coordinate chooseTarget() override = 0;
};

// AI з випадковими пострілами
class RandomAI : public AIPlayer {
private:
    std::vector<Coordinate> availableTargets;
    
    void initializeTargets();
    void removeTarget(const Coordinate& coord);
    
public:
    RandomAI(const std::string& aiName = "Random AI");
    
    Coordinate chooseTarget() override;
};

// Розумний AI з стратегією
class SmartAI : public AIPlayer {
private:
    enum Mode {
        HUNT,    // Пошук кораблів
        TARGET   // Добивання корабля
    };
    
    Mode currentMode;
    std::queue<Coordinate> targetQueue;  // Черга цілей для добивання
    Coordinate lastHit;                   // Останнє влучання
    std::vector<Coordinate> currentShipHits; // Влучання по поточному кораблю
    
    // Допоміжні методи
    std::vector<Coordinate> getAdjacentCells(const Coordinate& coord) const;
    void addAdjacentTargets(const Coordinate& coord);
    Coordinate getSmartHuntTarget();
    bool isValidTarget(const Coordinate& coord) const;
    void analyzeShipDirection();
    
public:
    SmartAI(const std::string& aiName = "Smart AI");
    
    Coordinate chooseTarget() override;
    
    // Оновлення стану AI після пострілу
    void updateAfterShot(const Coordinate& coord, ShotResult result);
};

#endif // AI_H