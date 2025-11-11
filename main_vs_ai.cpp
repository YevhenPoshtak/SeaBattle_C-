#include "common.h"
#include "board.h"
#include "player.h"
#include "ai.h"
#include <iostream>
#include <string>

// Функції UI (декларації)
void clearScreen();
void pause();
void printTitle();
void displayRules();
void displayVictory(const std::string& winner);
bool askPlayAgain();
void displayMatchStats(const Player& player1, const Player& player2);
void displayTurnInfo(const std::string& playerName, int turnNumber);
void showLoadingAnimation(const std::string& message, int duration);

// Вибір складності AI
int selectAIDifficulty() {
    clearScreen();
    printTitle();
    
    std::cout << Color::YELLOW << "Виберіть складність AI:\n" << Color::RESET;
    std::cout << "  1. " << Color::GREEN << "Простий AI" << Color::RESET << " - випадкові постріли\n";
    std::cout << "  2. " << Color::RED << "Розумний AI" << Color::RESET << " - стратегічні постріли\n";
    std::cout << "\nВаш вибір: ";
    
    int choice;
    std::cin >> choice;
    std::cin.ignore();
    
    return choice;
}

void playVsAI() {
    // Вибір складності
    int difficulty = selectAIDifficulty();
    
    // Створюємо гравця
    Player human;
    
    clearScreen();
    printTitle();
    
    std::cout << Color::CYAN << "Введіть ваше ім'я: " << Color::RESET;
    std::string name;
    std::getline(std::cin, name);
    if (name.empty()) name = "Гравець";
    human.setName(name);
    
    // Створюємо AI відповідної складності
    AIPlayer* ai;
    if (difficulty == 2) {
        ai = new SmartAI("🤖 Розумний AI");
    } else {
        ai = new RandomAI("🤖 Простий AI");
    }
    
    clearScreen();
    
    // Людина розміщує кораблі
    std::cout << Color::YELLOW << "═══════════════════════════════════════\n";
    std::cout << "  " << human.getName() << ", підготуйтесь!\n";
    std::cout << "═══════════════════════════════════════\n" << Color::RESET;
    pause();
    clearScreen();
    
    human.placeShips();
    
    std::cout << "\n" << Color::GREEN << "Ваші кораблі розміщено!\n" << Color::RESET;
    pause();
    
    // AI розміщує кораблі
    clearScreen();
    showLoadingAnimation(ai->getName() + " розміщує свої кораблі", 3);
    ai->placeShips();
    
    // Початок гри
    clearScreen();
    std::cout << Color::GREEN;
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                            ║\n";
    std::cout << "║                    ⚓ ГРА ПОЧИНАЄТЬСЯ! ⚓                  ║\n";
    std::cout << "║                                                            ║\n";
    std::cout << "║         " << human.getName() << "  VS  " << ai->getName() << "                   ║\n";
    std::cout << "║                                                            ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    std::cout << Color::RESET;
    pause();
    
    // Основний ігровий цикл
    int turnNumber = 1;
    bool gameOver = false;
    bool humanTurn = true;  // Людина ходить першою
    
    while (!gameOver) {
        clearScreen();
        
        if (humanTurn) {
            // ХІД ЛЮДИНИ
            displayTurnInfo(human.getName(), turnNumber);
            
            // Показуємо дошки гравця
            human.displayBothBoards();
            
            // Гравець робить постріл
            bool validShot = false;
            Coordinate target;
            ShotResult result;
            
            while (!validShot) {
                target = human.chooseTarget();
                
                if (!target.isValid()) {
                    std::cout << Color::RED << "Неправильні координати! Спробуйте ще раз.\n" << Color::RESET;
                    continue;
                }
                
                // Робимо постріл по AI
                result = ai->receiveShot(target);
                
                if (result == SHOT_INVALID) {
                    continue;
                }
                
                validShot = true;
            }
            
            // Обробляємо результат пострілу
            human.processShotResult(target, result);
            
            // Показуємо оновлену дошку
            std::cout << "\n";
            human.displayTrackingBoard();
            
            // Перевіряємо чи гра закінчена
            if (result == SHOT_WIN) {
                gameOver = true;
                
                pause();
                displayVictory(human.getName());
                
                // Показуємо статистику
                displayMatchStats(human, *ai);
                
                // Показуємо фінальні дошки
                std::cout << Color::CYAN << "\nФінальний стан:\n" << Color::RESET;
                std::cout << "\nВаші кораблі:\n";
                human.displayOwnBoard();
                
                std::cout << "\nКораблі AI:\n";
                ai->getOwnBoard().display(false);
                
            } else {
                pause();
                humanTurn = false;
            }
            
        } else {
            // ХІД AI
            displayTurnInfo(ai->getName(), turnNumber);
            
            std::cout << Color::YELLOW << ai->getName() << " думає...\n" << Color::RESET;
            
            // AI вибирає ціль
            Coordinate target = ai->chooseTarget();
            
            // AI робить постріл
            ShotResult result = human.receiveShot(target);
            
            // AI обробляє результат (тільки для SmartAI)
            SmartAI* smartAI = dynamic_cast<SmartAI*>(ai);
            if (smartAI) {
                smartAI->updateAfterShot(target, result);
            }
            
            // Оновлюємо tracking board AI
            ai->processShotResult(target, result);
            
            // Показуємо вашу дошку після пострілу AI
            std::cout << "\nВаша дошка після пострілу AI:\n";
            human.displayOwnBoard();
            
            // Перевіряємо чи гра закінчена
            if (result == SHOT_WIN) {
                gameOver = true;
                
                pause();
                displayVictory(ai->getName());
                
                // Показуємо статистику
                displayMatchStats(human, *ai);
                
                // Показуємо фінальні дошки
                std::cout << Color::CYAN << "\nФінальний стан:\n" << Color::RESET;
                std::cout << "\nВаші кораблі:\n";
                human.displayOwnBoard();
                
                std::cout << "\nКораблі AI:\n";
                ai->getOwnBoard().display(false);
                
            } else {
                pause();
                humanTurn = true;
                turnNumber++;
            }
        }
    }
    
    delete ai;
}

int main() {
    // Встановлюємо UTF-8 для Windows
    #ifdef _WIN32
        system("chcp 65001 > nul");
    #endif
    
    bool keepPlaying = true;
    
    while (keepPlaying) {
        clearScreen();
        printTitle();
        
        std::cout << Color::YELLOW << "Показати правила гри? (y/n): " << Color::RESET;
        std::string showRules;
        std::getline(std::cin, showRules);
        
        if (showRules == "y" || showRules == "Y" || showRules == "т" || showRules == "Т") {
            displayRules();
        }
        
        // Запускаємо гру
        playVsAI();
        
        // Питаємо чи хочуть грати ще раз
        keepPlaying = askPlayAgain();
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