#include <exception>
#include <iostream>
#include <list>
#include <ncurses.h>
#include <random>

constexpr int FRAME_TIME = 100;
constexpr int DEFAULT_HEIGHT = 12;
constexpr int DEFAULT_WIDTH = 40;

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

	void start_game() const {
		if (!initscr()) {
			throw std::runtime_error("Failed to initialize ncurses");
		}
		try {
			curs_set(0);
			nodelay(stdscr, TRUE);
			noecho();
			keypad(stdscr, TRUE);
		} catch (...) {
			endwin();
			throw;
		}
	}

	void end_game() const { endwin(); }

	int get_height() const { return height; }

	int get_width() const { return width; }
};

class Snake {
  private:
	Game &game;
	Direction direction = Direction::RIGHT;
	std::list<Point> snake = {Point(0, 0)};

  public:
	Snake(Game &g) : game(g){};

	void move() {
		switch (direction) {
		case Direction::UP:
			snake.push_front(Point(snake.front().x,
								   (snake.front().y - 1 + game.get_height()) %
									   game.get_height()));
			break;
		case Direction::DOWN:
			snake.push_front(Point(snake.front().x,
								   (snake.front().y + 1) % game.get_height()));
			break;
		case Direction::LEFT:
			snake.push_front(Point(
				((snake.front().x - 1 + game.get_width()) % game.get_width()),
				snake.front().y));
			break;
		case Direction::RIGHT:
			snake.push_front(Point(((snake.front().x + 1) % game.get_width()),
								   snake.front().y));
			break;
		}
		snake.pop_back();
	}

	void grow() { snake.push_back(snake.back()); }

	void change_direction(const Direction &d) { direction = d; }

	bool is_snake_part(const Point &p) const {
		for (auto iter = snake.begin(); iter != snake.end(); iter++) {
			if (*iter == p)
				return true;
		}
		return false;
	}

	bool eat_itself() const {
		for (auto iter1 = snake.begin(); iter1 != snake.end(); iter1++) {
			for (auto iter2 = std::next(iter1); iter2 != snake.end(); iter2++) {
				if ((*iter1) == (*iter2))
					return true;
			}
		}
		return false;
	}

	size_t size() const { return snake.size(); }
};

class Food {
  private:
	Game &game;
	Point p;

  public:
	void respawn(const Snake &snake) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distX(0, game.get_width() - 1);
		std::uniform_int_distribution<> distY(0, game.get_height() - 1);
		do {
			p.x = distX(gen);
			p.y = distY(gen);
		} while (snake.is_snake_part(p));
	}

	Food(Game &g) : game(g){};

	Point get_position() const { return p; };
};

class Console_Renderer {
  private:
	Game &game;
	Snake &snake;
	Food &food;

  public:
	Console_Renderer(Game &g, Snake &s, Food &f) : game(g), snake(s), food(f){};

	void render_game() const {
		for (int y = 0; y < game.get_height(); y++) {
			for (int x = 0; x < game.get_width(); x++) {
				if (snake.is_snake_part(Point(x, y))) {
					mvprintw(y, x, "0");
				} else if (Point(x, y) == food.get_position()) {
					mvprintw(y, x, "b");
				} else {
					mvprintw(y, x, "-");
				}
			}
			printw("\n");
		}
		refresh();
	}

	void render_game_over() const {
		clear();
		mvprintw(game.get_height() / 2, game.get_width() / 2 - 5, "GAME OVER");
		mvprintw(game.get_height() / 2 + 1, game.get_width() / 2 - 5,
				 "Score: %zu", snake.size() - 1);
		refresh();
	}

	void render_score() const {
		mvprintw(game.get_height(), game.get_width() / 2 - 5, "Score: %zu",
				 snake.size() - 1);
		refresh();
	}
};

bool check_collision(Food &food, Snake &snake) {
	return snake.is_snake_part(food.get_position());
}

int main() {
	Game game(DEFAULT_HEIGHT, DEFAULT_WIDTH);
	Snake snake(game);
	Food food(game);
	game.start_game();
	food.respawn(snake);
	Console_Renderer renderer(game, snake, food);
	while (!snake.eat_itself()) {
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
		if (check_collision(food, snake)) {
			food.respawn(snake);
			snake.grow();
		}
		snake.move();
		renderer.render_game();
		renderer.render_score();
		napms(FRAME_TIME);
		clear();
	}
	renderer.render_game_over();
	napms(3000);
	game.end_game();
}