#include <exception>
#include <iostream>
#include <ncurses.h>

constexpr int FRAME_TIME = 600;
constexpr int DEFAULT_HEIGHT = 25;
constexpr int DEFAULT_WIDTH = 80;

enum class Direction { UP, DOWN, LEFT, RIGHT };

class Point {
public:
  int x;
  int y;

  Point(int x = 0, int y = 0) : x(x), y(y) {}

  bool operator==(const Point &other) const {
    return x == other.x && y == other.y;
  }

  bool operator!=(const Point &other) const { return !(*this == other); }
};

class Game {
private:
  int height;
  int width;

public:
  Game(int h = DEFAULT_HEIGHT, int w = DEFAULT_WIDTH) : height(h), width(w){};

  void start_game() {
    if (!initscr()) {
      throw std::invalid_argument("Name must be less than 10 chars");
    }
    curs_set(0);
    nodelay(stdscr, true);
    noecho();
    keypad(stdscr, true);
  }

  void end_game() { endwin(); }

  int get_height() { return height; }

  int get_width() { return width; }
};

class Snake {
private:
  Game &game;
  Point pos;
  Direction direction = Direction::RIGHT;

public:
  Snake(Game &g, int x, int y) : game(g), pos(x, y){};

  void move() {
    switch (direction) {
    case Direction::UP:
      pos.y = (pos.y - 1 + game.get_height()) % game.get_height();
      break;
    case Direction::DOWN:
      pos.y = (pos.y + 1) % game.get_height();
      break;
    case Direction::LEFT:
      pos.x = (pos.x - 1 + game.get_width()) % game.get_width();
      break;
    case Direction::RIGHT:
      pos.x = (pos.x + 1) % game.get_width();
      break;
    }
  }

  void change_direction(Direction d) { direction = d; }

  Point get_position() { return pos; }
};

class Food {
private:
  Game &game;
  Point p;

public:
  Food(Game &g, int x, int y) : game(g), p(x, y){};

  Point get_position() { return p; };
};

class Console_Renderer {
private:
  Game &game;
  Snake &snake;
  Food &food;

public:
  Console_Renderer(Game &g, Snake &s, Food &f) : game(g), snake(s), food(f){};

  void render() {
    for (size_t y = 0; y < game.get_height(); y++) {
      for (size_t x = 0; x < game.get_width(); x++) {
        if (snake.get_position().x == x && snake.get_position().y == y) {
          printw("0");
        } else if (x == food.get_position().x && y == food.get_position().y) {
          printw("b");
        } else {
          printw("-");
        }
      }
      printw("\n");
    }
    refresh();
  }
};

bool check_collision(Food &food, Snake &snake) {
  return (snake.get_position().x == food.get_position().x &&
          snake.get_position().y == food.get_position().y);
}

int main() {
  Game game(DEFAULT_HEIGHT, DEFAULT_WIDTH);
  Snake snake(game, 1, 1);
  Food food(game, 10, 10);
  game.start_game();
  Console_Renderer renderer(game, snake, food);
  while (!check_collision(food, snake)) {
    int ch = getch();
    switch (ch) {
    case KEY_UP:
      snake.change_direction(Direction::UP);
      break;
    case KEY_DOWN:
      snake.change_direction(Direction::DOWN);
      break;
    case KEY_LEFT:
      snake.change_direction(Direction::LEFT);
      break;
    case KEY_RIGHT:
      snake.change_direction(Direction::RIGHT);
      break;
    }
    snake.move();
    renderer.render();
    napms(FRAME_TIME);
    clear();
  }
  game.end_game();
}