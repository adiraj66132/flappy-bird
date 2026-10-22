#include "raylib.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>

namespace {

constexpr int kScreenW = 288;
constexpr int kScreenH = 512;
constexpr float kGroundH = 112.0f;
constexpr float kGroundY = kScreenH - kGroundH;
constexpr float kGravity = 1000.0f;
constexpr float kFlapVel = -320.0f;
constexpr float kMaxFall = 620.0f;
constexpr float kPipeSpeed = 120.0f;
constexpr float kPipeW = 52.0f;
constexpr float kPipeH = 320.0f;
constexpr float kPipeGap = 110.0f;
constexpr float kSpawnInterval = 1.5f;
constexpr float kBirdX = 64.0f;
constexpr float kBirdW = 34.0f;
constexpr float kBirdH = 24.0f;

class Sprite {
public:
    Sprite() = default;
    explicit Sprite(const std::string& path) : tex_(LoadTexture(path.c_str())) {}
    ~Sprite()
    {
        if (tex_.id != 0) UnloadTexture(tex_);
    }

    Sprite(const Sprite&) = delete;
    Sprite& operator=(const Sprite&) = delete;

    Sprite(Sprite&& other) noexcept : tex_(other.tex_) { other.tex_ = {}; }
    Sprite& operator=(Sprite&& other) noexcept
    {
        if (this != &other) {
            if (tex_.id != 0) UnloadTexture(tex_);
            tex_ = other.tex_;
            other.tex_ = {};
        }
        return *this;
    }

    const ::Texture2D& get() const { return tex_; }
    bool loaded() const { return tex_.id != 0; }

private:
    ::Texture2D tex_{};
};

struct Assets {
    Sprite background{"assets/sprites/background-day.png"};
    Sprite base{"assets/sprites/base.png"};
    Sprite pipe{"assets/sprites/pipe-green.png"};
    Sprite birdUp{"assets/sprites/yellowbird-upflap.png"};
    Sprite birdMid{"assets/sprites/yellowbird-midflap.png"};
    Sprite birdDown{"assets/sprites/yellowbird-downflap.png"};
    Sprite message{"assets/sprites/message.png"};
    Sprite gameover{"assets/sprites/gameover.png"};
    std::array<Sprite, 10> digits;

    Assets()
    {
        for (int i = 0; i < 10; i++)
            digits[i] = Sprite("assets/sprites/" + std::to_string(i) + ".png");
    }

    bool loaded() const
    {
        if (!background.loaded() || !base.loaded() || !pipe.loaded() ||
            !birdUp.loaded() || !birdMid.loaded() || !birdDown.loaded() ||
            !message.loaded() || !gameover.loaded())
            return false;
        for (const auto& d : digits)
            if (!d.loaded()) return false;
        return true;
    }
};

struct Bird {
    float x = kBirdX;
    float y = kScreenH * 0.45f;
    float vy = 0.0f;
    float angle = 0.0f;
    float animTimer = 0.0f;
    int frame = 1;
};

struct Pipe {
    float x;
    float gapCenter;
    bool scored = false;
};

class Game {
public:
    Game()
    {
        if (!assets_.loaded()) {
            TraceLog(LOG_ERROR, "Missing sprites in assets/sprites/");
            ok_ = false;
        }
        reset();
    }

    bool ok() const { return ok_; }

    void run()
    {
        while (!WindowShouldClose()) {
            float dt = std::min(GetFrameTime(), 0.05f);
            update(dt);
            draw();
        }
    }

private:
    enum class State { Ready, Playing, GameOver };

    Assets assets_;
    Bird bird_;
    std::vector<Pipe> pipes_;
    State state_ = State::Ready;
    int score_ = 0;
    float spawnTimer_ = 0.0f;
    float baseScroll_ = 0.0f;
    float readyTime_ = 0.0f;
    bool ok_ = true;

    void reset()
    {
        bird_ = Bird{};
        pipes_.clear();
        score_ = 0;
        spawnTimer_ = 0.0f;
        readyTime_ = 0.0f;
        state_ = State::Ready;
    }

    void flap() { bird_.vy = kFlapVel; }

    bool hitsPipe() const
    {
        Rectangle birdRect = { bird_.x - kBirdW * 0.35f, bird_.y - kBirdH * 0.35f,
                               kBirdW * 0.7f, kBirdH * 0.7f };
        for (const Pipe& p : pipes_) {
            float gapTop = p.gapCenter - kPipeGap * 0.5f;
            float gapBottom = p.gapCenter + kPipeGap * 0.5f;
            Rectangle top = { p.x, gapTop - kPipeH, kPipeW, kPipeH };
            Rectangle bottom = { p.x, gapBottom, kPipeW, kPipeH };
            if (CheckCollisionRecs(birdRect, top) || CheckCollisionRecs(birdRect, bottom))
                return true;
        }
        return false;
    }

    void update(float dt)
    {
        if (state_ != State::GameOver) {
            baseScroll_ = std::fmod(baseScroll_ + kPipeSpeed * dt,
                                    (float)assets_.base.get().width);
            bird_.animTimer += dt;
            if (bird_.animTimer > 0.1f) {
                bird_.animTimer = 0.0f;
                bird_.frame = (bird_.frame + 1) % 3;
            }
        }

        switch (state_) {
        case State::Ready:
            readyTime_ += dt;
            bird_.y = kScreenH * 0.45f + std::sin(readyTime_ * 5.0f) * 6.0f;
            bird_.angle = 0.0f;
            if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                state_ = State::Playing;
                bird_.vy = 0.0f;
                flap();
            }
            break;

        case State::Playing:
            if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                flap();

            bird_.vy = std::min(bird_.vy + kGravity * dt, kMaxFall);
            bird_.y += bird_.vy * dt;
            if (bird_.y < kBirdH * 0.5f) {
                bird_.y = kBirdH * 0.5f;
                bird_.vy = 0.0f;
            }
            bird_.angle = std::clamp(bird_.vy * 0.12f, -25.0f, 90.0f);

            spawnTimer_ -= dt;
            if (spawnTimer_ <= 0.0f) {
                pipes_.push_back({ (float)kScreenW, (float)GetRandomValue(130, 300) });
                spawnTimer_ = kSpawnInterval;
            }

            for (Pipe& p : pipes_) {
                p.x -= kPipeSpeed * dt;
                if (!p.scored && p.x + kPipeW < bird_.x - kBirdW * 0.4f) {
                    p.scored = true;
                    score_++;
                }
            }
            pipes_.erase(std::remove_if(pipes_.begin(), pipes_.end(),
                                        [](const Pipe& p) { return p.x + kPipeW < -10.0f; }),
                         pipes_.end());

            if (hitsPipe() || bird_.y + kBirdH * 0.5f >= kGroundY) {
                if (bird_.y + kBirdH * 0.5f >= kGroundY) {
                    bird_.y = kGroundY - kBirdH * 0.5f;
                    bird_.vy = 0.0f;
                }
                state_ = State::GameOver;
            }
            break;

        case State::GameOver:
            bird_.vy = std::min(bird_.vy + kGravity * dt, kMaxFall);
            bird_.y += bird_.vy * dt;
            if (bird_.y + kBirdH * 0.5f >= kGroundY) {
                bird_.y = kGroundY - kBirdH * 0.5f;
                bird_.vy = 0.0f;
            }
            bird_.angle = std::min(bird_.angle + 300.0f * dt, 90.0f);
            if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                reset();
            break;
        }
    }

    void drawScore(int value, int y) const
    {
        std::string text = std::to_string(value);
        int digitW = assets_.digits[0].get().width;
        int x = (kScreenW - (int)text.size() * digitW) / 2;
        for (char c : text) {
            DrawTexture(assets_.digits[c - '0'].get(), x, y, WHITE);
            x += digitW;
        }
    }

    void draw() const
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTexture(assets_.background.get(), 0, 0, WHITE);

        for (const Pipe& p : pipes_) {
            float gapTop = p.gapCenter - kPipeGap * 0.5f;
            float gapBottom = p.gapCenter + kPipeGap * 0.5f;

            DrawTexture(assets_.pipe.get(), (int)p.x, (int)gapBottom, WHITE);

            Rectangle src = { 0.0f, 0.0f, kPipeW, kPipeH };
            Rectangle dst = { p.x + kPipeW * 0.5f, gapTop - kPipeH * 0.5f, kPipeW, kPipeH };
            Vector2 origin = { kPipeW * 0.5f, kPipeH * 0.5f };
            DrawTexturePro(assets_.pipe.get(), src, dst, origin, 180.0f, WHITE);
        }

        const ::Texture2D& birdTex = bird_.frame == 0 ? assets_.birdUp.get()
                                   : bird_.frame == 1 ? assets_.birdMid.get()
                                                      : assets_.birdDown.get();
        Rectangle birdSrc = { 0.0f, 0.0f, kBirdW, kBirdH };
        Rectangle birdDst = { bird_.x, bird_.y, kBirdW, kBirdH };
        Vector2 birdOrigin = { kBirdW * 0.5f, kBirdH * 0.5f };
        DrawTexturePro(birdTex, birdSrc, birdDst, birdOrigin, bird_.angle, WHITE);

        DrawTexture(assets_.base.get(), (int)-baseScroll_, (int)kGroundY, WHITE);
        DrawTexture(assets_.base.get(),
                    (int)(-baseScroll_ + assets_.base.get().width), (int)kGroundY, WHITE);

        if (state_ == State::Ready) {
            DrawTexture(assets_.message.get(),
                        (kScreenW - assets_.message.get().width) / 2, 70, WHITE);
        }

        if (state_ != State::Ready) drawScore(score_, 40);

        if (state_ == State::GameOver) {
            DrawTexture(assets_.gameover.get(),
                        (kScreenW - assets_.gameover.get().width) / 2, 120, WHITE);
            const char* hint = "SPACE / CLICK TO RESTART";
            int x = (kScreenW - MeasureText(hint, 16)) / 2;
            DrawText(hint, x + 1, 221, 16, BLACK);
            DrawText(hint, x, 220, 16, WHITE);
        }

        EndDrawing();
    }
};

}  // namespace

int main()
{
    InitWindow(kScreenW, kScreenH, "Flappy Bird - raylib (C++)");
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)std::time(nullptr));

    {
        Game game;
        if (game.ok()) game.run();
    }

    CloseWindow();
    return 0;
}
