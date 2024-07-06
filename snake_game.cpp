#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <ctime>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int GRID_SIZE = 20;
const int INITIAL_SNAKE_SIZE = 4;
const int MOVE_INTERVAL = 150;

SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
SDL_Texture* gBackgroundTexture = nullptr;
SDL_Texture* gSnakeHeadTexture = nullptr;
SDL_Texture* gSnakeBodyTexture = nullptr;
SDL_Texture* gSnakeTailTexture = nullptr;
SDL_Texture* gFruitTexture = nullptr; // Texture for the fruit

SDL_Texture* gSnakeTailTextureUp = nullptr;
SDL_Texture* gSnakeTailTextureDown = nullptr;
SDL_Texture* gSnakeTailTextureLeft = nullptr;
SDL_Texture* gSnakeTailTextureRight = nullptr;

TTF_Font* gFont = nullptr;
int score = 0;

struct Point {
    int x, y;
    Point() : x(0), y(0) {}
    Point(int _x, int _y) : x(_x), y(_y) {}
};

enum class Direction { UP, DOWN, LEFT, RIGHT };

enum class GameState { WELCOME_SCREEN, PLAYING, GAME_OVER };

class Snake {
public:
    Snake(int x, int y) : direction(Direction::RIGHT) {
        for (int i = 0; i < INITIAL_SNAKE_SIZE; ++i) {
            body.push_back(Point(x - i, y));
        }
    }

    void move() {
        Point new_head = body.front();
        switch (direction) {
            case Direction::UP:
                new_head.y--;
                break;
            case Direction::DOWN:
                new_head.y++;
                break;
            case Direction::LEFT:
                new_head.x--;
                break;
            case Direction::RIGHT:
                new_head.x++;
                break;
        }
        body.pop_back();
        body.insert(body.begin(), new_head);
    }

    void changeDirection(Direction new_direction) {
        if ((new_direction == Direction::UP && direction != Direction::DOWN) ||
            (new_direction == Direction::DOWN && direction != Direction::UP) ||
            (new_direction == Direction::LEFT && direction != Direction::RIGHT) ||
            (new_direction == Direction::RIGHT && direction != Direction::LEFT)) {
            direction = new_direction;
        }
    }

    bool collidesWithItself() const {
        Point head = body.front();
        for (size_t i = 1; i < body.size(); ++i) {
            if (body[i].x == head.x && body[i].y == head.y) {
                return true;
            }
        }
        return false;
    }

    void grow() {
        Point tail = body.back();
        body.push_back(tail);
    }

    const std::vector<Point>& getBody() const {
        return body;
    }

private:
    std::vector<Point> body;
    Direction direction;
};

class Food {
public:
    Food() {
        respawn();
    }

    void respawn() {
        srand(time(nullptr));
        location.x = rand() % (SCREEN_WIDTH / GRID_SIZE);
        location.y = rand() % (SCREEN_HEIGHT / GRID_SIZE);
    }

    Point getLocation() const {
        return location;
    }

private:
    Point location;
};

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    gWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Load background image
    SDL_Surface* surface = IMG_Load("background.jpg");
    if (surface == nullptr) {
        std::cerr << "Failed to load background image! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    gBackgroundTexture = SDL_CreateTextureFromSurface(gRenderer, surface);
    SDL_FreeSurface(surface);
    if (gBackgroundTexture == nullptr) {
        std::cerr << "Failed to create texture from background image! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Load snake textures
    surface = IMG_Load("snake_head.png");
    if (surface == nullptr) {
        std::cerr << "Failed to load snake head image! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    gSnakeHeadTexture = SDL_CreateTextureFromSurface(gRenderer, surface);
    SDL_FreeSurface(surface);
    if (gSnakeHeadTexture == nullptr) {
        std::cerr << "Failed to create texture from snake head image! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    surface = IMG_Load("snake_body.png");
    if (surface == nullptr) {
        std::cerr << "Failed to load snake body image! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    gSnakeBodyTexture = SDL_CreateTextureFromSurface(gRenderer, surface);
    SDL_FreeSurface(surface);
    if (gSnakeBodyTexture == nullptr) {
        std::cerr << "Failed to create texture from snake body image! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Load snake tail textures
    gSnakeTailTextureUp = IMG_LoadTexture(gRenderer, "snake_tail_up.png");
    gSnakeTailTextureDown = IMG_LoadTexture(gRenderer, "snake_tail_down.png");
    gSnakeTailTextureLeft = IMG_LoadTexture(gRenderer, "snake_tail_left.png");
    gSnakeTailTextureRight = IMG_LoadTexture(gRenderer, "snake_tail_right.png");

    // Check for errors in loading tail textures
    if (gSnakeTailTextureUp == nullptr || gSnakeTailTextureDown == nullptr ||
        gSnakeTailTextureLeft == nullptr || gSnakeTailTextureRight == nullptr) {
        std::cerr << "Failed to load snake tail textures! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Load fruit texture
    gFruitTexture = IMG_LoadTexture(gRenderer, "fruit.png");
    if (gFruitTexture == nullptr) {
        std::cerr << "Failed to load fruit texture! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Load font
    gFont = TTF_OpenFont("Roboto-Medium.ttf", 28);
    if (gFont == nullptr) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

void close() {
    SDL_DestroyTexture(gBackgroundTexture);
    gBackgroundTexture = nullptr;

    SDL_DestroyTexture(gSnakeHeadTexture);
    gSnakeHeadTexture = nullptr;

    SDL_DestroyTexture(gSnakeBodyTexture);
    gSnakeBodyTexture = nullptr;

    SDL_DestroyTexture(gSnakeTailTexture);
    gSnakeTailTexture = nullptr;

    SDL_DestroyTexture(gSnakeTailTextureUp);
    SDL_DestroyTexture(gSnakeTailTextureDown);
    SDL_DestroyTexture(gSnakeTailTextureLeft);
    SDL_DestroyTexture(gSnakeTailTextureRight);

    SDL_DestroyTexture(gFruitTexture); // Destroy fruit texture

    TTF_CloseFont(gFont);
    gFont = nullptr;

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    gRenderer = nullptr;

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void drawBackground() {
    SDL_RenderCopy(gRenderer, gBackgroundTexture, nullptr, nullptr);
}

Direction snakeDirection(const std::vector<Point>& snake) {
    if (snake.size() < 2) {
        return Direction::RIGHT; // Default direction if snake has only head
    }
    const Point& head = snake.front();
    const Point& neck = snake[1];
    if (head.x == neck.x && head.y > neck.y) {
        return Direction::UP;
    } else if (head.x == neck.x && head.y < neck.y) {
        return Direction::DOWN;
    } else if (head.y == neck.y && head.x > neck.x) {
        return Direction::LEFT;
    } else {
        return Direction::RIGHT;
    }
}

Direction snakeTailDirection(const std::vector<Point>& snake) {
    if (snake.size() < 2) {
        return Direction::RIGHT; // Default direction if snake has only head
    }
    const Point& tail = snake.back();
    const Point& neck = snake[snake.size() - 2];
    if (tail.x == neck.x && tail.y > neck.y) {
        return Direction::UP;
    } else if (tail.x == neck.x && tail.y < neck.y) {
        return Direction::DOWN;
    } else if (tail.y == neck.y && tail.x > neck.x) {
        return Direction::LEFT;
    } else {
        return Direction::RIGHT;
    }
}

void drawSnake(const std::vector<Point>& snake) {
    if (snake.empty()) return; // Ensure there's at least one segment (head)

    // Render the tail first
    SDL_Rect tailRect = { snake.back().x * GRID_SIZE, snake.back().y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
    switch (snakeTailDirection(snake)) {
        case Direction::UP:
            SDL_RenderCopy(gRenderer, gSnakeTailTextureUp, nullptr, &tailRect);
            break;
        case Direction::DOWN:
            SDL_RenderCopy(gRenderer, gSnakeTailTextureDown, nullptr, &tailRect);
            break;
        case Direction::LEFT:
            SDL_RenderCopy(gRenderer, gSnakeTailTextureLeft, nullptr, &tailRect);
            break;
        case Direction::RIGHT:
            SDL_RenderCopy(gRenderer, gSnakeTailTextureRight, nullptr, &tailRect);
            break;
    }

    // Render the body segments
    for (size_t i = 1; i < snake.size() - 1; ++i) {
        SDL_Rect bodyRect = { snake[i].x * GRID_SIZE, snake[i].y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
        SDL_RenderCopy(gRenderer, gSnakeBodyTexture, nullptr, &bodyRect);
    }

    // Render the head
    SDL_Rect headRect = { snake.front().x * GRID_SIZE, snake.front().y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
    switch (snakeDirection(snake)) {
        case Direction::UP:
            SDL_RenderCopy(gRenderer, gSnakeHeadTexture, nullptr, &headRect);
            break;
        case Direction::DOWN:
            SDL_RenderCopyEx(gRenderer, gSnakeHeadTexture, nullptr, &headRect, 180.0, nullptr, SDL_FLIP_NONE);
            break;
        case Direction::LEFT:
            SDL_RenderCopyEx(gRenderer, gSnakeHeadTexture, nullptr, &headRect, -90.0, nullptr, SDL_FLIP_NONE);
            break;
        case Direction::RIGHT:
            SDL_RenderCopyEx(gRenderer, gSnakeHeadTexture, nullptr, &headRect, 90.0, nullptr, SDL_FLIP_NONE);
            break;
    }
}

void drawFood(const Food& food) {
    SDL_Rect rect = { food.getLocation().x * GRID_SIZE, food.getLocation().y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
    SDL_RenderCopy(gRenderer, gFruitTexture, nullptr, &rect);
}

void renderText(const std::string& message, int x, int y) {
    SDL_Color color = {0, 0, 0};
    SDL_Surface* surface = TTF_RenderText_Solid(gFont, message.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(gRenderer, surface);
    SDL_Rect destRect = { x, y, surface->w, surface->h };

    SDL_RenderCopy(gRenderer, texture, nullptr, &destRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderWelcomeScreen() {
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
    SDL_RenderClear(gRenderer);

    // Render background image
    SDL_Rect backgroundRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderCopy(gRenderer, gBackgroundTexture, nullptr, &backgroundRect);

    renderText("Snake Game", SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 4);

    // Render Start button
    SDL_Rect startButtonRect = { SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2, 100, 50 };
    SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255); // Green color
    SDL_RenderFillRect(gRenderer, &startButtonRect);
    renderText("Start", SCREEN_WIDTH / 2 - 30, SCREEN_HEIGHT / 2 + 10);

    // Render Exit button
    SDL_Rect exitButtonRect = { SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 + 70, 100, 50 };
    SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255); // Red color
    SDL_RenderFillRect(gRenderer, &exitButtonRect);
    renderText("Exit", SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT / 2 + 80);

    SDL_RenderPresent(gRenderer);
}

int main(int argc, char* args[]) {
    if (!init()) {
        std::cerr << "Failed to initialize!" << std::endl;
        return -1;
    }

    bool quit = false;
    SDL_Event e;

    GameState gameState = GameState::WELCOME_SCREEN;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                if (gameState == GameState::WELCOME_SCREEN) {
                    // Check if Start button clicked
                    if (mouseX >= SCREEN_WIDTH / 2 - 50 && mouseX <= SCREEN_WIDTH / 2 + 50 &&
                        mouseY >= SCREEN_HEIGHT / 2 && mouseY <= SCREEN_HEIGHT / 2 + 50) {
                        gameState = GameState::PLAYING;
                        Snake snake(SCREEN_WIDTH / (2 * GRID_SIZE), SCREEN_HEIGHT / (2 * GRID_SIZE));
                        Food food;
                        Uint32 lastMove = SDL_GetTicks();
                        score = 0;
                        while (!quit && gameState == GameState::PLAYING) {
                            while (SDL_PollEvent(&e) != 0) {
                                if (e.type == SDL_QUIT) {
                                    quit = true;
                                    gameState = GameState::WELCOME_SCREEN;
                                } else if (e.type == SDL_KEYDOWN) {
                                    switch (e.key.keysym.sym) {
                                        case SDLK_UP:
                                            snake.changeDirection(Direction::UP);
                                            break;
                                        case SDLK_DOWN:
                                            snake.changeDirection(Direction::DOWN);
                                            break;
                                        case SDLK_LEFT:
                                            snake.changeDirection(Direction::LEFT);
                                            break;
                                        case SDLK_RIGHT:
                                            snake.changeDirection(Direction::RIGHT);
                                            break;
                                    }
                                }
                            }

                            if (SDL_GetTicks() - lastMove >= MOVE_INTERVAL) {
                                snake.move();
                                if (snake.collidesWithItself()) {
                                    std::cout << "Game Over!" << std::endl;
                                    gameState = GameState::GAME_OVER;
                                    quit = true;
                                    continue;
                                }

                                Point head = snake.getBody().front();
                                if (head.x < 0 || head.x >= SCREEN_WIDTH / GRID_SIZE || head.y < 0 || head.y >= SCREEN_HEIGHT / GRID_SIZE) {
                                    std::cout << "Game Over!" << std::endl;
                                    gameState = GameState::GAME_OVER;
                                    quit = true;
                                    continue;
                                }

                                if (head.x == food.getLocation().x && head.y == food.getLocation().y) {
                                    snake.grow();
                                    food.respawn();
                                    score++;
                                }

                                lastMove = SDL_GetTicks();
                            }

                            SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                            SDL_RenderClear(gRenderer);

                            drawBackground(); // Draw the background image first
                            drawSnake(snake.getBody());
                            drawFood(food);
                            renderText("Score: " + std::to_string(score), 10, 10);

                            SDL_RenderPresent(gRenderer);

                            SDL_Delay(10);
                        }
                    }
                    // Check if Exit button clicked
                    else if (mouseX >= SCREEN_WIDTH / 2 - 50 && mouseX <= SCREEN_WIDTH / 2 + 50 &&
                             mouseY >= SCREEN_HEIGHT / 2 + 70 && mouseY <= SCREEN_HEIGHT / 2 + 120) {
                        quit = true;
                    }
                }
            }
        }

        if (gameState == GameState::WELCOME_SCREEN) {
            renderWelcomeScreen();
        }
    }

    close();

    return 0;
}
