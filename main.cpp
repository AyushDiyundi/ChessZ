#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include "ChessEngineConnector.hpp"

using namespace sf;

int size = 56;
Vector2f offset(28, 28);
const int EMPTY = 0;

Sprite f[32]; // Figures
std::string position = "";

int board[8][8] =
    {-1, -2, -3, -4, -5, -3, -2, -1,
     -6, -6, -6, -6, -6, -6, -6, -6,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      6,  6,  6,  6,  6,  6,  6,  6,
      1,  2,  3,  4,  5,  3,  2,  1};
bool whiteToMove = true;

std::string toChessNote(Vector2f p) {
    std::string s = "";
    s += char(p.x / size + 97);
    s += char(7 - p.y / size + 49);
    return s;
}

Vector2f toCoord(char a, char b) {
    int x = int(a) - 97;
    int y = 7 - int(b) + 49;
    return Vector2f(x * size, y * size);
}

bool isValidMove(int fromX, int fromY, int toX, int toY) {
    if (toX < 0 || toX >= 8 || toY < 0 || toY >= 8) return false;

    if (board[toY][toX] != EMPTY && (whiteToMove ? board[toY][toX] > 0 : board[toY][toX] < 0)) return false;

    return true;
}

void movePiece(Vector2f oldPos, Vector2f newPos) {
    int oldX = oldPos.x / size;
    int oldY = oldPos.y / size;
    int newX = newPos.x / size;
    int newY = newPos.y / size;

    if (isValidMove(oldX, oldY, newX, newY)) {
        board[newY][newX] = board[oldY][oldX];
        board[oldY][oldX] = EMPTY;

        for (int i = 0; i < 32; i++) {
            if (f[i].getPosition() == oldPos) {
                f[i].setPosition(newPos);
                break;
            }
        }

        position += toChessNote(oldPos) + toChessNote(newPos) + " ";
        whiteToMove = !whiteToMove;
    }
}

void applyMove(RenderWindow& window, Sprite& sBoard, const std::string& move_str) {
    Vector2f oldPos = toCoord(move_str[0], move_str[1]);
    Vector2f newPos = toCoord(move_str[2], move_str[3]);

    int n = -1;
    for (int i = 0; i < 32; i++) {
        if (f[i].getPosition() == oldPos) {
            n = i;
            break;
        }
    }

    if (n != -1) {
        for (int k = 0; k < 50; k++) {
            Vector2f p = newPos - oldPos;
            f[n].move(p.x / 50, p.y / 50);

            window.clear();
            window.draw(sBoard);
            for (int i = 0; i < 32; i++) window.draw(f[i]);
            window.display();
        }

        movePiece(oldPos, newPos);
        position += move_str + " ";
        f[n].setPosition(newPos);
    }
}

void handleAIMove(RenderWindow& window, Sprite& sBoard) {
    std::string move_str = getNextMove(position);

    applyMove(window, sBoard, move_str);
}
void loadPosition() {
    int k = 0; // Index for the pieces array

    // Clear any existing piece positions
    for (int i = 0; i < 32; ++i) {
        f[i].setPosition(-100, -100); // Move pieces off-screen
    }

    // Load pieces from the board array
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int n = board[i][j];
            if (n == EMPTY) continue; // Skip empty squares

            int x = abs(n) - 1; // Piece type index (0 for pawns, 1 for knights, etc.)
            int y = (n > 0) ? 1 : 0; // Piece color (1 for white, 0 for black)

            // Set the texture rect for the piece
            f[k].setTextureRect(IntRect(size * x, size * y, size, size));
            // Set the position for the piece
            f[k].setPosition(size * j, size * i);

            // Log the position of each piece
            std::cout << "Piece " << k << " at position: " << f[k].getPosition().x
                      << ", " << f[k].getPosition().y << std::endl;

            k++;
        }
    }

    // Apply all moves from the position string
    size_t pos_length = position.length();
    for (size_t i = 0; i < pos_length; i += 5) {
        if (i + 4 < pos_length) { // Ensure valid substring bounds
            std::string move_str = position.substr(i, 4);
            applyMove(move_str); // Apply the move without expecting a return value
        }
    }
}

int main() {
    RenderWindow window(VideoMode(504, 504), "The Chess! (press SPACE)");
    ConnectToEngine("D:/ChessZ/stockfish.exe");

    Texture t1, t2;
    t1.loadFromFile("D:/ChessZ/images/figures.png");
    t2.loadFromFile("D:/ChessZ/images/board.png");

    for (int i = 0; i < 32; i++) f[i].setTexture(t1);
    Sprite sBoard(t2);

    loadPosition(window, sBoard);

    bool isMove = false;
    float dx = 0, dy = 0;
    Vector2f oldPos, newPos;
    std::string str;
    int n = 0;

    while (window.isOpen()) {
        Vector2i pos = Mouse::getPosition(window) - Vector2i(offset);

        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();

            if (e.type == Event::KeyPressed && e.key.code == Keyboard::BackSpace) {
                if (position.length() > 6) position.erase(position.length() - 6, 5);
                loadPosition(window, sBoard);
            }

            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                for (int i = 0; i < 32; i++) {
                    if (f[i].getGlobalBounds().contains(pos.x, pos.y)) {
                        isMove = true;
                        n = i;
                        dx = pos.x - f[i].getPosition().x;
                        dy = pos.y - f[i].getPosition().y;
                        oldPos = f[i].getPosition();
                        break;
                    }
                }
            }

            if (e.type == Event::MouseButtonReleased && e.mouseButton.button == Mouse::Left) {
                if (isMove) {
                    isMove = false;
                    Vector2f p = f[n].getPosition() + Vector2f(size / 2, size / 2);
                    newPos = Vector2f(size * int(p.x / size), size * int(p.y / size));
                    str = toChessNote(oldPos) + toChessNote(newPos);
                    movePiece(oldPos, newPos);
                    if (oldPos != newPos) position += str + " ";
                    f[n].setPosition(newPos);
                    whiteToMove = !whiteToMove;

                    handleAIMove(window, sBoard);  // AI makes its move after the player
                }
            }
        }

        if (isMove) f[n].setPosition(pos.x - dx, pos.y - dy);

        window.clear();
        window.draw(sBoard);
        for (int i = 0; i < 32; i++) window.draw(f[i]);
        window.display();
    }
    CloseConnection();
    return 0;
}

