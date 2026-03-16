# 🌀 NeoCrash 3D

> *"Break layers. Chain combos. Survive the tunnel."*

A fully 3D brick-breaker game built with **OpenGL + FreeGLUT** in C++ —
developed as a lab assignment for **Computer Graphics Lab (CSE 494)**.

![Language](https://img.shields.io/badge/language-C%2B%2B17-orange)
![OpenGL](https://img.shields.io/badge/OpenGL-2.1-green)
![Course](https://img.shields.io/badge/course-CSE%20494-blueviolet)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)
![License](https://img.shields.io/badge/license-MIT-blue)

---

## 🎓 Academic Context

This project was built for the **Computer Graphics Lab (Course Code: CSE 494)** as a
graded lab task. The goal was to create an interactive 3D graphics application using
**OpenGL** and **FreeGLUT**, demonstrating the core Computer Graphics concepts
taught in the course e.g. perspective projection, lighting, collision detection, and
real-time rendering.

We implemented everything our teacher required, then went significantly beyond the
requirements to add gameplay depth, visual polish, and extra technical features.

---

## 📋 Original Lab Requirements

These were the features our teacher specified for the assignment:

| # | Requirement | Status |
|---|---|---|
| 1 | A **3D cube-shaped arena boundary** as the play area | Done |
| 2 | A **ball** that moves and bounces inside the 3D arena | Done |
| 3 | **Wall collision** — ball reflects correctly off all 6 faces of the cube | Done |
| 4 | A **movable paddle / plate** the player controls to hit the ball | Done |
| 5 | **Brick layers** arranged as flat XY planes (parallel to the back wall) inside the arena | Done |
| 6 | **Brick collision detection** — ball detects and destroys individual bricks | Done |
| 7 | **Arrow key controls** to move the paddle | Done |
| 8 | A **HUD** showing score and remaining lives | Done |
| 9 | **Lighting and shading** using OpenGL's fixed-function pipeline | Done |
| 10 | **Perspective projection** for realistic 3D depth perception | Done |

---

## 🚀 What We Added Beyond Requirements

We extended the game significantly by adding several features that were not part of the original task but were **additional requirements** from our teacher, making the game more engaging, polished, and technically rich:

| # | Extra Feature | Description |
|---|---|---|
| 1 | **12 coloured brick layers** | All 12 layers are visible simultaneously through the transparent tunnel walls; active layer is bright, future layers are dimmed |
| 2 | **4-axis paddle movement** | Paddle moves on both **X and Y axes** (requirement was X-axis only), letting the player reposition freely in the full 2D plane |
| 3 | **Trajectory prediction line** | Real-time forward ball simulation (3,000 steps) shows a rainbow dashed path and exactly where the ball will land |
| 4 | **Ghost paddle guide** | Yellow rectangle outline on the paddle plane shows where to move to intercept the ball |
| 5 | **Rainbow ball trail** | HSV-cycling motion trail follows the ball; turns orange during Fireball mode |
| 6 | **Animated starfield** | 300 twinkling stars outside the tunnel drift toward the camera, adding depth and atmosphere |
| 7 | **Shockwave rings** | Two expanding ring effects burst from the paddle face on every successful hit |
| 8 | **Screen colour flash** | Blue flash on wall bounce · Green flash on paddle hit · Red flash on life lost |
| 9 | **Brick layer shimmer** | Active brick layer animates with a travelling light-wave shimmer effect |
| 10 |**Tunnel wall pulse** | All 4 glass side walls breathe rhythmically in opacity |
| 11 |**4-wall shadow projection** | Planar shadows cast on **all 4 side walls** using a generic shadow matrix (requirement only needed basic rendering) |
| 12 | **8 Power-up types** | WIDE, LIFE, SLOW, FAST, 2×SCORE, SHRINK, FIREBALL, SHIELD — each a spinning gem with floating label, danger warning, and landing marker |
| 13 | **Combo multiplier** | Breaking bricks within 2 seconds of each other chains a combo with ×1.25 score bonus per level |
| 14 | **High score + Rank** | Session high score persists across resets; end-screen shows C / B / A / S rank based on final score |
| 15 | **Particle burst system** | Colour-matched explosion particles with gravity on every brick destroyed |
| 16 | **Fireball power-up** | Ball pierces through 5 bricks without bouncing; not a standard brick-breaker mechanic |
| 17 | **Shield power-up** | Automatically catches the ball once if missed; unique safety mechanic |
| 18 | **Paddle colour tinting** | Paddle changes colour to match the active power-up (green = wide, purple = shrink, etc.) |

---

## ✨ Full Feature List

### 🌍 3D Gameplay
- Front-to-back **tunnel perspective camera** — looks straight down the -Z axis
- **12 layered bricks** all visible at once through transparent glass walls
- Move paddle on **both X and Y axes** with arrow keys
- **AABB collision detection** with shallowest-axis bounce resolution
- Ball speed **increases per layer** (+1.2% per cleared layer)
- Layer progression — only the **front-most uncleared layer** is breakable

### 🎨 Visual Effects

| Effect | Description |
|---|---|
| Starfield | 300 twinkling stars drift toward camera with parallax |
| Rainbow trail | HSV colour-cycling glow trail behind the ball |
| Shockwave rings | Two expanding rings on every paddle hit |
| Screen flash | Colour-coded flashes for each event type |
| Brick shimmer | Moving light wave on the active brick layer |
| Tunnel pulse | Walls breathe in and out rhythmically |
| Depth fog | Linear fog for tunnel atmosphere |
| Particles | Colour-matched explosion with gravity and fade |
| 4-wall shadows | Planar projections on left, right, top, and bottom walls |

### ⚡ Gameplay Systems

| System | Description |
|---|---|
| Combo multiplier | Chain breaks within 2 s for ×1.25 bonus per level |
| High score | Best score persists across resets |
| End-game rank | C / B / A / S based on final score |
| Trajectory line | Rainbow dashed prediction path |
| Ghost paddle | Outline shows optimal paddle position |
| Layer progression | Clear a layer to unlock the next |

---

## 💊 Power-Ups (8 types)

Power-ups drop from broken bricks (35% chance) as **spinning coloured gems**
that float slowly toward your paddle.
Catch good ones — or sidestep the danger ones!

| Colour | Name | Effect | Duration |
|---|---|---|---|
| 🟢 Green | **WIDE** | Paddle grows to 2× size | 10 s |
| 🔴 Red | **LIFE** | +1 extra life | Instant |
| 🔵 Blue | **SLOW** | Ball slows to 50% speed | 8 s |
| 🟠 Orange | **FAST** ⚠️ | Ball speeds up to 180% — hard to control | 6 s |
| 🟡 Yellow | **2×SCORE** | All points doubled | 10 s |
| 🟣 Purple | **SHRINK** ⚠️ | Paddle shrinks to half size — very difficult | 7 s |
| 🔶 Deep orange | **FIREBALL** 🔥 | Ball pierces 5 bricks without bouncing | 6 s |
| 🩵 Ice blue | **SHIELD** 🛡️ | Auto-catches the ball once if missed | One-time |

> ⚠️ **FAST** and **SHRINK** are danger power-ups. Their gems flash red with
> `!!AVOID!!` labels so you can dodge them.
> The HUD shows a red landing marker at the spot where the gem will arrive.

---

## 🎮 Controls

| Key | Action |
|---|---|
| `←` `→` `↑` `↓` | Move paddle in all 4 directions |
| `SPACE` | Launch the ball |
| `R` | Restart game |
| `ESC` | Quit |

---

## 🏗️ Build Instructions

### Prerequisites

Install **FreeGLUT** and **OpenGL** development libraries.

#### 🐧 Linux (Ubuntu / Debian)
```bash
sudo apt update
sudo apt install g++ freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev
```

#### 🍎 macOS (Homebrew)
```bash
brew install freeglut
```

#### 🪟 Windows
- Install **Code::Blocks** (includes MinGW) from https://www.codeblocks.org/
- Download **FreeGLUT for MinGW** from https://www.transmissionzero.co.uk/software/freeglut-devel/
- Extract and copy `freeglut.dll` into the same folder as your compiled `.exe`
- Copy `include/GL/` headers and `lib/` files into your MinGW directories

---

### Compile & Run

#### 🐧 Linux / 🍎 macOS
```bash
g++ brick_breaker_3d.cpp -o neocrash3d -lGL -lGLU -lglut -lm && ./neocrash3d
```

#### 🪟 Windows (MinGW terminal)
```bash
g++ brick_breaker_3d.cpp -o neocrash3d.exe -lfreeglut -lopengl32 -lglu32
neocrash3d.exe
```

#### 🪟 Windows (Code::Blocks — recommended)
1. Open Code::Blocks → **File → New → Empty Project**
2. Add `brick_breaker_3d.cpp` to the project
3. Go to **Project → Build Options → Linker settings → Link libraries**
4. Add these three libraries: `freeglut` · `opengl32` · `glu32`
5. Press **Build & Run** (`F9`)

---

## 🕹️ How to Play

1. **Launch** — press `SPACE` to shoot the ball toward the back wall
2. **Break bricks** — only the **front-most uncleared layer** is breakable at any time; all other layers are visible but pass-through
3. **Intercept the ball** — use `←` `→` `↑` `↓` to position your paddle; the ball is lost if it flies past you
4. **Follow the trajectory** — the **rainbow dashed line** predicts the ball's bounce path; the **yellow ghost paddle** shows exactly where to move
5. **Collect power-ups** — spinning gems float toward you after brick breaks
   - 🟢🔴🔵🟡🔶🩵 = good, move toward them
   - 🟠🟣 = danger, flash red with `!!AVOID!!` — step aside
6. **Chain combos** — break multiple bricks within 2 seconds to earn bonus multipliers
7. **Clear all 12 layers** to win and see your final rank!

---

## 📊 HUD Reference

```
┌──────────────────────────────────────────────────────────────┐
│  SCORE: 1240 [x2]      ★ ★ ★       LAYER: 3 / 12           │
│  BEST:  3800                                                 │
│                                               ● (layer 1)    │  ← layer
│              COMBO  x4 !                     ● (layer 2)     │     progress
│              FIREBALL!  x3                   ◉ (layer 3) ←  │     dots
│                                               ○ (layer 4)    │
│  ██████████  WIDE PADDLE                     ○ ...           │  ← power-up
│  ████        2× SCORE                                        │     timer bars
│              [ SHIELD ACTIVE ]                               │
│                                                              │
│                            POWER-UPS: [WIDE][LIFE][SLOW]...  │  ← legend
│                                                              │
│   ARROWS=MOVE   SPACE=LAUNCH   R=RESTART   ESC=QUIT          │
└──────────────────────────────────────────────────────────────┘
```

| HUD Element | Meaning |
|---|---|
| **Layer dots** (right edge) | Bright ◉ = current target · Faded ○ = future · Dim = cleared |
| **Timer bars** (bottom-left) | Remaining seconds for each active power-up |
| **★ stars** | Lives remaining |
| **Rainbow dash line** | Ball's predicted bounce path |
| **Yellow crosshair** | Exact landing spot at paddle plane |
| **Ghost paddle outline** | Where to move to intercept — follows predicted landing |

---

## 🏆 Scoring System

| Action | Points |
|---|---|
| Break a brick on layer N | `10 × (N + 1)` |
| 2×SCORE power-up active | All gains doubled |
| Combo chain of C bricks | Score × `(1 + 0.25 × (C − 1))` |

**Example:** Layer-6 brick + 4× combo + 2×SCORE active:
```
10 × 7 × (1 + 0.75) × 2  =  245 points
```

### 🥇 End-Game Ranks

| Rank | Min Score | Colour |
|---|---|---|
| 🩶 **C** | 0 | Grey |
| 💙 **B** | 2,500 | Blue |
| 💚 **A** | 5,000 | Green |
| 💛 **S** | 8,000 | Gold |

---

## 🗂️ Project Structure

```
NeoCrash3D/
├── brick_breaker_3d.cpp   ← complete game — single self-contained C++ file (~1,900 lines)
└── README.md              ← this file
└── NeoCrash-3D.exe        ← exe file to play without env setup
```

No engine. No framework. No extra libraries. Just **OpenGL + FreeGLUT + pure C++**.

---

## 🧩 Computer Graphics Concepts Demonstrated

This project covers the following topics from **CSE 494**:

| Concept | How it is used |
|---|---|
| **Perspective projection** | `gluPerspective()` 58° FOV, near=0.3, far=300 |
| **View / camera transform** | `gluLookAt()` positioned in front of the tunnel along +Z |
| **Lighting model** | `GL_LIGHT0` with diffuse, specular, and ambient components |
| **Material properties** | Per-brick `glMaterialfv()` for shininess and colour response |
| **Planar shadow** | Custom 4×4 column-major shadow projection matrix (any plane) |
| **Alpha blending** | `GL_BLEND` — both `ONE_MINUS_SRC_ALPHA` (glass) and `GL_ONE` (additive glow) |
| **Depth testing** | `GL_DEPTH_TEST` with depth mask disabled for transparent passes |
| **Linear fog** | `GL_FOG` for tunnel depth atmosphere |
| **AABB collision** | Per-axis overlap with shallowest-penetration-axis resolution |
| **Particle system** | Point sprites with velocity, gravity, and life-based fade |
| **2D overlay (HUD)** | `glOrtho()` orthographic switch after 3D scene pass |
| **Colour space math** | Manual `hsvToRgb()` for rainbow trail and combo display |
| **Animation** | Timer-driven `glutTimerFunc()` at 60 FPS for smooth physics |

---

## 👥 Team

| Name | Registration No. |
|---|---|
| AL AMIN HOSSAIN | *2020331057* |
| *---* | *---* |

**Course:** Computer Graphics Lab — **CSE 494**
**Institution:** *Shahjalal University Of Science & Technology*
**Semester / Year:** *4Th Year Second Semester*
**Course Teacher:** *A.K.M. Fakhrul Hossain*

---

## 📄 License

This project is open-source under the **MIT License** —
free to use, modify, and share with attribution.

---

*Built from scratch using OpenGL + FreeGLUT. No game engine. No external libraries. Pure C++.*