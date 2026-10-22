# Flappy Bird (C++ / raylib)

![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![raylib](https://img.shields.io/badge/raylib-5.x-red)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)

A desktop Flappy Bird clone written in C++17 with [raylib](https://www.raylib.com/). It uses the classic Flappy Bird sprite set, delta-time physics, and RAII-managed resources.

## Features

- Classic flap-and-dodge gameplay with pipe gap randomization
- Sprite-based rendering using the original-style bird, pipes, ground, background, score digits, and splash screens
- Frame-rate independent physics: gravity, flap impulse, terminal velocity, and velocity-based bird rotation
- Ready, playing, and game-over states with restart
- Leak-safe C++ resource handling (textures unloaded via RAII, no manual `new`/`delete`)

## Preview
<img width="300" height="543" alt="Screenshot 2026-10-23 010632" src="https://github.com/user-attachments/assets/31888272-31f0-4cac-9a1e-8f0feb37c668" />
<img width="292" height="542" alt="Screenshot 2026-10-23 010609" src="https://github.com/user-attachments/assets/53492eb3-6124-4c1f-83bc-54931df7ed13" />
<img width="298" height="543" alt="image" src="https://github.com/user-attachments/assets/dab32b34-f696-4e39-80df-72b2a6e858d3" />

## Controls

| Action | Input |
|---|---|
| Flap | `Space` or left mouse click |
| Start game | `Space` or left mouse click |
| Restart after game over | `Space` or left mouse click |
| Quit | `Esc` |

## Requirements

- Linux with X11
- C++17 compiler (`g++` or `clang++`)
- raylib development package (`libraylib-dev` or built from source)

Install raylib on Debian/Ubuntu:

```bash
sudo apt install libraylib-dev
```

## Build and Run

```bash
g++ -std=c++17 -O2 main.cpp -o flappy \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./flappy
```

## Project Structure

```
flappy_bird/
├── main.cpp
├── assets/
│   └── sprites/
│       ├── background-day.png
│       ├── base.png
│       ├── pipe-green.png
│       ├── yellowbird-*.png
│       ├── message.png
│       ├── gameover.png
│       └── 0.png … 9.png
└── README.md
```

## Physics

| Parameter | Value |
|---|---|
| Gravity | 1000 px/s² |
| Flap impulse | -320 px/s |
| Terminal fall speed | 620 px/s |
| Pipe speed | 120 px/s |
| Pipe gap | 110 px |
| Spawn interval | 1.5 s |

Gameplay uses a clamped delta time, so physics remain stable across frame rates.

## Assets

The sprites come from the [FlapPyBird](https://github.com/sourabhv/FlapPyBird) project, distributed under the MIT License. Flappy Bird is a game by Dong Nguyen; this project is a non-commercial learning clone.

## License

Source code in this repository is provided for educational purposes. See the asset attribution above for sprite licensing.
