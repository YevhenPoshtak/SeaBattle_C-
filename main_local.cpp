#include "common.h"
#include "board.h"
#include "player.h"
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
void showPlayerSwitchScreen(const std::string& nextPlayer);

void playLocalGame() {
    // Створюємо двох гравців
    Player player1, player2;
    
    clearScreen();
    printTitle();
    
    // Введення імен гравців
    std::cout << Color::CYAN << "Введіть ім'я першого гравця: " << Color::RESET;
    std::string name1;
    std::getline(std::cin, name1);
    if (name1.empty()) name1 = "Гравець 1";
    player1.setName(name1);
    
    std::cout << Color::CYAN << "Введіть ім'я другого гравця: " << Color::RESET;
    std::string name2;
    std::getline(std::cin, name2);
    if (name2.empty()) name2 = "Гравець 2";
    player2.setName(name2);
    
    clearScreen();
    
    // Гравець 1 розміщує кораблі
    std::cout << Color::YELLOW << "═══════════════════════════════════════\n";
    std::cout << "  " << player1.getName() << ", підготуйтесь!\n";
    std::cout << "═══════════════════════════════════════\n" << Color::RESET;
    pause();
    clearScreen();
    
    player1.placeShips();
    
    std::cout << "\n" << Color::GREEN << "Кораблі гравця " << player1.getName() 
              << " розміщено!\n" << Color::RESET;
    pause();
    
    // Зміна гравця
    showPlayerSwitchScreen(player2.getName());
    
    // Гравець 2 розміщує кораблі
    std::cout << Color::YELLOW << "═══════════════════════════════════════\n";
    std::cout << "  " << player2.getName() << ", підготуйтесь!\n";
    std::cout << "═══════════════════════════════════════\n" << Color::RESET;
    pause();
    clearScreen();
    
    player2.placeShips();
    
    std::cout << "\n" << Color::GREEN << "Кораблі гравця " << player2.getName() 
              << " розміщено!\n" << Color::RESET;
    pause();
    
    // Початок гри
    clearScreen();
    std::cout << Color::GREEN;
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                            ║\n";
    std::cout << "║                    ⚓ ГРА ПОЧИНАЄТЬСЯ! ⚓                  ║\n";
    std::cout << "║                                                            ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    std::cout << Color::RESET;
    pause();
    
    // Основний ігровий цикл
    Player* currentPlayer = &player1;
    Player* opponent = &player2;
    int turnNumber = 1;
    bool gameOver = false;
    
    while (!gameOver) {
        // Зміна гравця
        showPlayerSwitchScreen(currentPlayer->getName());
        
        // Відображення інформації про хід
        displayTurnInfo(currentPlayer->getName(), turnNumber);
        
        // Показуємо дошки гравця
        currentPlayer->displayBothBoards();
        
        // Гравець робить постріл
        bool validShot = false;
        Coordinate target;
        ShotResult result;
        
        while (!validShot) {
            target = currentPlayer->chooseTarget();
            
            if (!target.isValid()) {
                std::cout << Color::RED << "Неправильні координати! Спробуйте ще раз.\n" << Color::RESET;
                continue;
            }
            
            // Робимо постріл
            result = opponent->receiveShot(target);
            
            if (result == SHOT_INVALID) {
                // Невалідний постріл - просимо спробувати знову
                continue;
            }
            
            validShot = true;
        }
        
        // Обробляємо результат пострілу
        currentPlayer->processShotResult(target, result);
        
        // Показуємо оновлену дошку після пострілу
        std::cout << "\n";
        currentPlayer->displayTrackingBoard();
        
        // Перевіряємо чи гра закінчена
        if (result == SHOT_WIN) {
            gameOver = true;
            
            pause();
            displayVictory(currentPlayer->getName());
            
            // Показуємо статистику
            displayMatchStats(*currentPlayer, *opponent);
            
            // Показуємо фінальні дошки обох гравців
            std::cout << Color::CYAN << "Фінальний стан дошок:\n" << Color::RESET;
            std::cout << "\n" << currentPlayer->getName() << ":\n";
            currentPlayer->displayOwnBoard();
            
            std::cout << "\n" << opponent->getName() << ":\n";
            opponent->displayOwnBoard();
            
        } else {
            // Продовжуємо гру
            pause();
            
            // Міняємо гравців
            if (currentPlayer == &player1) {
                currentPlayer = &player2;
                opponent = &player1;
            } else {
                currentPlayer = &player1;
                opponent = &player2;
                turnNumber++; // Збільшуємо номер ходу після повного раунду
            }
        }
    }
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
        playLocalGame();
        
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