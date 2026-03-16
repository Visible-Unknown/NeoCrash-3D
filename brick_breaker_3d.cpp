/*
 * ================================================================
 *  3D BRICK BREAKER  –  OpenGL / FreeGLUT  (Enhanced Edition)
 *  Author : @alamin
 * ================================================================
 *
 *  FEATURES
 *  ─────────────────────────────────────────────────────────────
 *  • Front-to-back 3D tunnel perspective camera
 *  • 12 coloured brick layers – all visible at once
 *  • Move paddle with ALL FOUR arrow keys (X + Y axes)
 *  • Trajectory prediction line with rainbow dashes
 *  • Animated starfield background (300 stars)
 *  • Rainbow motion trail on the ball
 *  • Combo multiplier – chain bricks for bonus score
 *  • Shockwave ring burst on every paddle hit
 *  • Screen colour flash (wall = blue, hit = green, death = red)
 *  • Active layer brick shimmer animation
 *  • Tunnel wall pulse / breathing effect
 *  • 8 power-ups including FIREBALL and SHIELD
 *  • High score persists across resets
 *  • End-game rank: C / B / A / S
 *  • Planar shadows projected on ALL 4 walls
 *
 *  CONTROLS
 *  ─────────────────────────────────────────────────────────────
 *  Arrow keys   Move paddle (left / right / up / down)
 *  SPACE        Launch ball
 *  R            Restart game
 *  ESC          Quit
 *
 *  BUILD
 *  ─────────────────────────────────────────────────────────────
 *  Linux / macOS:
 *    g++ brick_breaker_3d.cpp -o bb3d -lGL -lGLU -lglut -lm && ./bb3d
 *
 *  Windows (MinGW / Code::Blocks):
 *    g++ brick_breaker_3d.cpp -o bb3d.exe -lfreeglut -lopengl32 -lglu32
 * ================================================================
 */

// ──────────────────────────────────────────────────────────────
//  Platform & includes
// ──────────────────────────────────────────────────────────────
#ifdef _WIN32
    #include <windows.h>
#endif
#include <GL/glut.h>
#include <GL/glu.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <ctime>

// ──────────────────────────────────────────────────────────────
//  Window dimensions
// ──────────────────────────────────────────────────────────────
static int WIN_W = 900;
static int WIN_H = 700;

// ──────────────────────────────────────────────────────────────
//  Arena half-extents  (world units)
//    X : left(-)  / right(+)
//    Y : bottom(-) / top(+)
//    Z : front(+, player side) / back(-, far wall)
// ──────────────────────────────────────────────────────────────
static const float ARENA_W = 10.0f;
static const float ARENA_H = 10.0f;
static const float ARENA_D = 34.0f;

// ──────────────────────────────────────────────────────────────
//  Brick grid constants
// ──────────────────────────────────────────────────────────────
static const int   NUM_LAYERS   = 12;
static const int   BRICK_COLS   = 5;
static const int   BRICK_ROWS   = 4;
static const float BRICK_W      = 3.2f;   // X half-extent
static const float BRICK_H      = 1.8f;   // Y half-extent
static const float BRICK_D      = 0.8f;   // Z half-thickness
static const float BRICK_GAP_X  = 0.5f;
static const float BRICK_GAP_Y  = 0.5f;

// Z centre of each layer (layer 0 = nearest player, 11 = far back)
static const float LAYER_Z[NUM_LAYERS] = {
    -3.0f, -5.5f, -8.0f, -10.5f, -13.0f, -15.5f,
    -18.0f, -20.5f, -23.0f, -25.5f, -28.0f, -30.5f
};

// RGB colour for each layer
static const float LAYER_COLOR[NUM_LAYERS][3] = {
    { 1.00f, 0.10f, 0.20f },   //  0  red
    { 1.00f, 0.50f, 0.05f },   //  1  orange
    { 1.00f, 0.90f, 0.05f },   //  2  yellow
    { 0.10f, 0.90f, 0.25f },   //  3  green
    { 0.05f, 0.70f, 1.00f },   //  4  cyan
    { 0.60f, 0.10f, 1.00f },   //  5  violet
    { 1.00f, 0.20f, 0.70f },   //  6  pink
    { 0.20f, 1.00f, 0.80f },   //  7  mint
    { 1.00f, 0.65f, 0.65f },   //  8  salmon
    { 0.65f, 0.85f, 1.00f },   //  9  sky blue
    { 1.00f, 1.00f, 0.40f },   // 10  lime
    { 0.90f, 0.40f, 0.10f }    // 11  amber
};

// ──────────────────────────────────────────────────────────────
//  Ball constants
// ──────────────────────────────────────────────────────────────
static const float BALL_RADIUS = 0.55f;
static const float BALL_SPEED  = 0.22f;

// ──────────────────────────────────────────────────────────────
//  Paddle constants
// ──────────────────────────────────────────────────────────────
static const float PADDLE_W     = 1.8f;   // X half-width  (base size)
static const float PADDLE_H     = 1.8f;   // Y half-height (base size)
static const float PADDLE_DEPTH = 0.3f;   // Z half-thickness
static const float PADDLE_Z_POS = ARENA_D - 0.8f;
static const float PADDLE_SPEED = 0.5f;   // units per frame when key held

// ──────────────────────────────────────────────────────────────
//  Structs
// ──────────────────────────────────────────────────────────────
struct Vec3 {
    float x, y, z;
    Vec3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
};

static inline Vec3  operator+(Vec3 a, Vec3 b)   { return Vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
static inline Vec3  operator*(Vec3 a, float s)   { return Vec3(a.x * s,   a.y * s,   a.z * s  ); }
static inline float vlen(Vec3 v)                 { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);    }

struct Brick {
    Vec3 center;
    int  layer;
    bool alive;    // always true (all layers visible from start)
    bool broken;   // true = destroyed
};

struct Particle {
    Vec3  pos, vel;
    float life;
    float r, g, b;
    float size;
};

struct TrailPoint {
    Vec3  pos;
    float age;        // 1.0 = fresh, 0.0 = gone
    float r, g, b;
};

struct Shockwave {
    Vec3  pos;
    float radius;
    float alpha;
    bool  active;
};

struct Star {
    float x, y, z;
    float brightness;
};

// ──────────────────────────────────────────────────────────────
//  Power-up system
// ──────────────────────────────────────────────────────────────
enum PowerUpType {
    PU_WIDE_PADDLE   = 0,   // Paddle grows 2x  (green)
    PU_EXTRA_LIFE    = 1,   // +1 life          (red)
    PU_SLOW_BALL     = 2,   // Ball slows 50%   (blue)
    PU_FAST_BALL     = 3,   // Ball speeds 180% (orange)  ← DANGER
    PU_MULTI_SCORE   = 4,   // 2× score 10 sec  (yellow)
    PU_SHRINK_PADDLE = 5,   // Paddle shrinks   (purple)  ← DANGER
    PU_FIREBALL      = 6,   // Pierce 5 bricks  (deep orange)
    PU_SHIELD        = 7,   // Auto-catch once  (ice blue)
    PU_COUNT         = 8
};

static const float PU_COLOR[PU_COUNT][3] = {
    { 0.0f, 1.0f, 0.4f },   // WIDE_PADDLE   green
    { 1.0f, 0.2f, 0.2f },   // EXTRA_LIFE    red
    { 0.2f, 0.6f, 1.0f },   // SLOW_BALL     blue
    { 1.0f, 0.5f, 0.0f },   // FAST_BALL     orange
    { 1.0f, 1.0f, 0.0f },   // MULTI_SCORE   yellow
    { 0.8f, 0.0f, 0.8f },   // SHRINK_PADDLE purple
    { 1.0f, 0.3f, 0.0f },   // FIREBALL      deep orange
    { 0.4f, 0.9f, 1.0f }    // SHIELD        ice blue
};

static const char* PU_LABEL[PU_COUNT] = {
    "WIDE", "LIFE", "SLOW", "FAST", "2xSCORE", "SHRINK", "FIREBALL", "SHIELD"
};

struct PowerUp {
    Vec3        pos, vel;
    PowerUpType type;
    float       spin;
    bool        active;
};

// ──────────────────────────────────────────────────────────────
//  Game state – containers
// ──────────────────────────────────────────────────────────────
static std::vector<Brick>      gBricks;
static std::vector<Particle>   gParticles;
static std::vector<PowerUp>    gPowerUps;
static std::vector<TrailPoint> gTrail;
static std::vector<Shockwave>  gShockwaves;
static std::vector<Star>       gStars;

// ──────────────────────────────────────────────────────────────
//  Game state – ball & paddle
// ──────────────────────────────────────────────────────────────
static Vec3  gBallPos;
static Vec3  gBallVel;
static bool  gBallOnPaddle = true;

static float gPaddleX    = 0.0f;
static float gPaddleY    = 0.0f;
static float gPaddleWCur = 1.8f;   // dynamic width  (modified by power-ups)
static float gPaddleHCur = 1.8f;   // dynamic height

static bool  gFireball     = false;
static int   gFireballLeft = 0;
static bool  gShield       = false;

// ──────────────────────────────────────────────────────────────
//  Game state – power-up timers & multipliers
// ──────────────────────────────────────────────────────────────
static float gBallSpeedMul = 1.0f;
static float gScoreMul     = 1.0f;
static int   gWidePaddleTimer = 0;
static int   gSlowBallTimer   = 0;
static int   gFastBallTimer   = 0;
static int   gScoreTimer      = 0;
static int   gShrinkTimer     = 0;
static int   gFireballTimer   = 0;

static char  gPickupMessage[48] = "";
static int   gPickupMessageTimer = 0;

// ──────────────────────────────────────────────────────────────
//  Game state – score, lives, progression
// ──────────────────────────────────────────────────────────────
static int   gScore        = 0;
static int   gLives        = 3;
static int   gCurrentLayer = 0;
static int   gHighScore    = 0;
static bool  gGameOver     = false;
static bool  gYouWin       = false;
static float gCameraShake  = 0.0f;

// Input keys held
static bool  gKeyLeft  = false;
static bool  gKeyRight = false;
static bool  gKeyUp    = false;
static bool  gKeyDown  = false;

// ──────────────────────────────────────────────────────────────
//  Visual effect state
// ──────────────────────────────────────────────────────────────
static int   gCombo        = 0;     // current combo chain count
static int   gComboTimer   = 0;     // frames left to extend chain
static float gComboFlash   = 0.0f;  // fade for combo display

static float gFlashAmount  = 0.0f;  // screen colour flash strength
static float gFlashR = 1.0f, gFlashG = 1.0f, gFlashB = 1.0f;

static float gTunnelPulse  = 0.0f;  // for wall breathing animation
static float gBrickShimmer = 0.0f;  // per-frame shimmer accumulator
static float gGlobalTime   = 0.0f;  // global frame counter

// Trajectory prediction
static float gPredX      = 0.0f;
static float gPredY      = 0.0f;
static float gPredPulse  = 0.0f;

// ──────────────────────────────────────────────────────────────
//  Utility helpers
// ──────────────────────────────────────────────────────────────
static float frand(float lo, float hi)
{
    return lo + (hi - lo) * (float)rand() / (float)RAND_MAX;
}

static void setSpeed(Vec3& v, float speed)
{
    float len = vlen(v);
    if (len > 1e-6f) v = v * (speed / len);
}

// Convert hue-saturation-value to RGB  (h in [0,1])
static void hsvToRgb(float h, float s, float v,
                     float& r, float& g, float& b)
{
    int   i = (int)(h * 6.0f);
    float f = h * 6.0f - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);
    switch (i % 6) {
        case 0: r=v; g=t; b=p; break;
        case 1: r=q; g=v; b=p; break;
        case 2: r=p; g=v; b=t; break;
        case 3: r=p; g=q; b=v; break;
        case 4: r=t; g=p; b=v; break;
        default:r=v; g=p; b=q; break;
    }
}

// ──────────────────────────────────────────────────────────────
//  Starfield initialisation
// ──────────────────────────────────────────────────────────────
static void initStars()
{
    gStars.clear();
    for (int i = 0; i < 300; i++) {
        Star s;
        s.x          = frand(-ARENA_W * 4.0f, ARENA_W * 4.0f);
        s.y          = frand(-ARENA_H * 4.0f, ARENA_H * 4.0f);
        s.z          = frand(-ARENA_D * 2.0f, ARENA_D * 3.0f);
        s.brightness = frand(0.3f, 1.0f);
        gStars.push_back(s);
    }
}

// ──────────────────────────────────────────────────────────────
//  Brick grid initialisation
// ──────────────────────────────────────────────────────────────
static void initBricks()
{
    gBricks.clear();
    float totalW = BRICK_COLS * (BRICK_W + BRICK_GAP_X) - BRICK_GAP_X;
    float totalH = BRICK_ROWS * (BRICK_H + BRICK_GAP_Y) - BRICK_GAP_Y;
    float originX = -totalW * 0.5f + BRICK_W * 0.5f;
    float originY = -totalH * 0.5f + BRICK_H * 0.5f;

    for (int L = 0; L < NUM_LAYERS; L++) {
        for (int row = 0; row < BRICK_ROWS; row++) {
            for (int col = 0; col < BRICK_COLS; col++) {
                Brick b;
                b.layer  = L;
                b.alive  = true;
                b.broken = false;
                b.center = Vec3(
                    originX + col * (BRICK_W + BRICK_GAP_X),
                    originY + row * (BRICK_H + BRICK_GAP_Y),
                    LAYER_Z[L]
                );
                gBricks.push_back(b);
            }
        }
    }
}

static bool isLayerCleared(int layer)
{
    for (size_t i = 0; i < gBricks.size(); i++)
        if (gBricks[i].layer == layer && !gBricks[i].broken)
            return false;
    return true;
}

// ──────────────────────────────────────────────────────────────
//  Particle helpers
// ──────────────────────────────────────────────────────────────
static void spawnBrickParticles(Vec3 pos, int layer, int count = 22)
{
    for (int i = 0; i < count; i++) {
        Particle p;
        p.pos  = pos;
        p.vel  = Vec3(frand(-0.4f, 0.4f), frand(-0.4f, 0.4f), frand(-0.35f, 0.35f));
        p.life = 1.0f;
        p.size = frand(3.0f, 7.0f);
        p.r    = LAYER_COLOR[layer][0];
        p.g    = LAYER_COLOR[layer][1];
        p.b    = LAYER_COLOR[layer][2];
        gParticles.push_back(p);
    }
}

static void spawnShockwave(Vec3 pos)
{
    Shockwave sw;
    sw.pos    = pos;
    sw.radius = 0.2f;
    sw.alpha  = 1.0f;
    sw.active = true;
    gShockwaves.push_back(sw);
}

// ──────────────────────────────────────────────────────────────
//  Power-up: apply effect on collection
// ──────────────────────────────────────────────────────────────
static void applyPowerUp(PowerUpType type)
{
    snprintf(gPickupMessage, sizeof(gPickupMessage), "%s!", PU_LABEL[type]);
    gPickupMessageTimer = 180;

    switch (type) {
        case PU_WIDE_PADDLE:
            gPaddleWCur = 3.8f;
            gPaddleHCur = 3.8f;
            gWidePaddleTimer = 600;   // 10 seconds
            break;
        case PU_EXTRA_LIFE:
            gLives++;
            break;
        case PU_SLOW_BALL:
            gBallSpeedMul = 0.5f;
            gSlowBallTimer = 480;     // 8 seconds
            setSpeed(gBallVel, BALL_SPEED * gBallSpeedMul);
            break;
        case PU_FAST_BALL:
            gBallSpeedMul = 1.8f;
            gFastBallTimer = 360;     // 6 seconds
            setSpeed(gBallVel, BALL_SPEED * gBallSpeedMul);
            break;
        case PU_MULTI_SCORE:
            gScoreMul    = 2.0f;
            gScoreTimer  = 600;       // 10 seconds
            break;
        case PU_SHRINK_PADDLE:
            gPaddleWCur = 0.85f;
            gPaddleHCur = 0.85f;
            gShrinkTimer = 420;       // 7 seconds
            break;
        case PU_FIREBALL:
            gFireball      = true;
            gFireballLeft  = 5;
            gFireballTimer = 360;
            break;
        case PU_SHIELD:
            gShield = true;
            break;
        default:
            break;
    }
}

static void trySpawnPowerUp(Vec3 pos)
{
    if ((rand() % 100) >= 35) return;   // 35% drop chance
    PowerUp pu;
    pu.pos    = pos;
    pu.vel    = Vec3(frand(-0.025f, 0.025f), frand(-0.025f, 0.025f), 0.16f);
    pu.type   = (PowerUpType)(rand() % PU_COUNT);
    pu.spin   = 0.0f;
    pu.active = true;
    gPowerUps.push_back(pu);
}

static void tickPowerUpTimers()
{
    if (gWidePaddleTimer > 0) {
        gWidePaddleTimer--;
        if (gWidePaddleTimer == 0) { gPaddleWCur = 1.8f; gPaddleHCur = 1.8f; }
    }
    if (gSlowBallTimer > 0) {
        gSlowBallTimer--;
        if (gSlowBallTimer == 0) { gBallSpeedMul = 1.0f; setSpeed(gBallVel, BALL_SPEED); }
    }
    if (gFastBallTimer > 0) {
        gFastBallTimer--;
        if (gFastBallTimer == 0) { gBallSpeedMul = 1.0f; setSpeed(gBallVel, BALL_SPEED); }
    }
    if (gScoreTimer > 0) {
        gScoreTimer--;
        if (gScoreTimer == 0) gScoreMul = 1.0f;
    }
    if (gShrinkTimer > 0) {
        gShrinkTimer--;
        if (gShrinkTimer == 0) { gPaddleWCur = 1.8f; gPaddleHCur = 1.8f; }
    }
    if (gFireballTimer > 0) {
        gFireballTimer--;
        if (gFireballTimer == 0) gFireball = false;
    }
    if (gPickupMessageTimer > 0) gPickupMessageTimer--;
    if (gComboTimer > 0) {
        gComboTimer--;
        if (gComboTimer == 0) { gCombo = 0; gComboFlash = 0.0f; }
    }
    if (gComboFlash  > 0.0f) gComboFlash  -= 0.03f;
    if (gFlashAmount > 0.0f) gFlashAmount -= 0.04f;
    if (gBrickShimmer < 9999.0f) gBrickShimmer += 1.0f;
}

// ──────────────────────────────────────────────────────────────
//  Reset / new game
// ──────────────────────────────────────────────────────────────
static void resetGame()
{
    if (gScore > gHighScore) gHighScore = gScore;

    gScore        = 0;
    gLives        = 3;
    gCurrentLayer = 0;
    gGameOver     = false;
    gYouWin       = false;
    gBallOnPaddle = true;
    gPaddleX      = 0.0f;
    gPaddleY      = 0.0f;
    gBallPos      = Vec3(0.0f, 0.0f, PADDLE_Z_POS - PADDLE_DEPTH - BALL_RADIUS - 0.1f);
    gBallVel      = Vec3(0.0f, 0.0f, 0.0f);

    gParticles.clear();
    gPowerUps.clear();
    gTrail.clear();
    gShockwaves.clear();

    gPaddleWCur    = 1.8f;
    gPaddleHCur    = 1.8f;
    gBallSpeedMul  = 1.0f;
    gScoreMul      = 1.0f;
    gFireball      = false;
    gFireballLeft  = 0;
    gShield        = false;

    gWidePaddleTimer = gSlowBallTimer = gFastBallTimer = 0;
    gScoreTimer      = gShrinkTimer   = gFireballTimer = 0;

    gCombo       = 0;
    gComboTimer  = 0;
    gComboFlash  = 0.0f;
    gFlashAmount = 0.0f;
    gBrickShimmer = 0.0f;

    gPickupMessage[0]    = '\0';
    gPickupMessageTimer  = 0;

    initBricks();
}

// ──────────────────────────────────────────────────────────────
//  OpenGL draw helpers
// ──────────────────────────────────────────────────────────────
static void drawSolidBox(float cx, float cy, float cz,
                         float hw, float hh, float hd)
{
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    glScalef(hw * 2.0f, hh * 2.0f, hd * 2.0f);
    glutSolidCube(1.0);
    glPopMatrix();
}

static void drawWireBox(float cx, float cy, float cz,
                        float hw, float hh, float hd)
{
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    glScalef(hw * 2.0f, hh * 2.0f, hd * 2.0f);
    glutWireCube(1.0);
    glPopMatrix();
}

// ──────────────────────────────────────────────────────────────
//  Starfield draw
// ──────────────────────────────────────────────────────────────
static void drawStars()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glPointSize(2.0f);

    glBegin(GL_POINTS);
    for (size_t i = 0; i < gStars.size(); i++) {
        float twinkle = 0.6f + 0.4f * sinf(gGlobalTime * 0.05f + gStars[i].x * 0.3f);
        glColor4f(0.7f, 0.85f, 1.0f, gStars[i].brightness * twinkle * 0.6f);
        glVertex3f(gStars[i].x, gStars[i].y, gStars[i].z);
    }
    glEnd();
    glDisable(GL_BLEND);
}

// ──────────────────────────────────────────────────────────────
//  Glass tunnel – all 4 side walls identical + pulsing opacity
// ──────────────────────────────────────────────────────────────
static void drawGlassTunnel()
{
    float W = ARENA_W, H = ARENA_H, D = ARENA_D;
    float alpha = 0.07f + 0.015f * sinf(gTunnelPulse);   // breathing effect

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);

    // Left / Right / Top / Bottom – same colour, same opacity
    glColor4f(0.1f, 0.4f, 0.9f, alpha);

    glBegin(GL_QUADS);  // Left wall  X = -W
        glVertex3f(-W, -H, -D);  glVertex3f(-W,  H, -D);
        glVertex3f(-W,  H,  D);  glVertex3f(-W, -H,  D);
    glEnd();

    glBegin(GL_QUADS);  // Right wall X = +W
        glVertex3f( W, -H,  D);  glVertex3f( W,  H,  D);
        glVertex3f( W,  H, -D);  glVertex3f( W, -H, -D);
    glEnd();

    glBegin(GL_QUADS);  // Top wall   Y = +H
        glVertex3f(-W,  H, -D);  glVertex3f( W,  H, -D);
        glVertex3f( W,  H,  D);  glVertex3f(-W,  H,  D);
    glEnd();

    glBegin(GL_QUADS);  // Bottom wall Y = -H
        glVertex3f(-W, -H,  D);  glVertex3f( W, -H,  D);
        glVertex3f( W, -H, -D);  glVertex3f(-W, -H, -D);
    glEnd();

    glColor4f(0.05f, 0.2f, 0.6f, 0.12f);
    glBegin(GL_QUADS);  // Back wall  Z = -D
        glVertex3f(-W, -H, -D);  glVertex3f( W, -H, -D);
        glVertex3f( W,  H, -D);  glVertex3f(-W,  H, -D);
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

// ──────────────────────────────────────────────────────────────
//  Wall grid + bright border edges on ALL 4 side walls
// ──────────────────────────────────────────────────────────────
static void drawWallGrid()
{
    glDisable(GL_LIGHTING);
    float W = ARENA_W, H = ARENA_H, D = ARENA_D;

    // Interior depth grid
    float gridAlpha = 0.18f + 0.08f * sinf(gTunnelPulse * 0.7f);
    glLineWidth(1.0f);
    glColor4f(0.0f, 0.5f, 1.0f, gridAlpha);

    glBegin(GL_LINES);
    for (float y = -H; y <= H; y += 2.5f) { glVertex3f(-W, y, -D); glVertex3f(-W, y,  D); }
    for (float z = -D; z <= D; z += 4.0f) { glVertex3f(-W,-H,  z); glVertex3f(-W, H,  z); }
    for (float y = -H; y <= H; y += 2.5f) { glVertex3f( W, y, -D); glVertex3f( W, y,  D); }
    for (float z = -D; z <= D; z += 4.0f) { glVertex3f( W,-H,  z); glVertex3f( W, H,  z); }
    for (float x = -W; x <= W; x += 2.5f) { glVertex3f( x,-H, -D); glVertex3f( x,-H,  D); }
    for (float z = -D; z <= D; z += 4.0f) { glVertex3f(-W,-H,  z); glVertex3f( W,-H,  z); }
    for (float x = -W; x <= W; x += 2.5f) { glVertex3f( x, H, -D); glVertex3f( x, H,  D); }
    for (float z = -D; z <= D; z += 4.0f) { glVertex3f(-W, H,  z); glVertex3f( W, H,  z); }
    glEnd();

    // Bright border frame on all 4 walls
    float edgeAlpha = 0.5f + 0.15f * sinf(gTunnelPulse);
    glLineWidth(2.2f);
    glColor4f(0.0f, 0.7f, 1.0f, edgeAlpha);

    glBegin(GL_LINES);
    // Left wall
    glVertex3f(-W,-H,-D); glVertex3f(-W, H,-D);  glVertex3f(-W,-H, D); glVertex3f(-W, H, D);
    glVertex3f(-W,-H,-D); glVertex3f(-W,-H, D);  glVertex3f(-W, H,-D); glVertex3f(-W, H, D);
    // Right wall
    glVertex3f( W,-H,-D); glVertex3f( W, H,-D);  glVertex3f( W,-H, D); glVertex3f( W, H, D);
    glVertex3f( W,-H,-D); glVertex3f( W,-H, D);  glVertex3f( W, H,-D); glVertex3f( W, H, D);
    // Top wall
    glVertex3f(-W, H,-D); glVertex3f( W, H,-D);  glVertex3f(-W, H, D); glVertex3f( W, H, D);
    glVertex3f(-W, H,-D); glVertex3f(-W, H, D);  glVertex3f( W, H,-D); glVertex3f( W, H, D);
    // Bottom wall
    glVertex3f(-W,-H,-D); glVertex3f( W,-H,-D);  glVertex3f(-W,-H, D); glVertex3f( W,-H, D);
    glVertex3f(-W,-H,-D); glVertex3f(-W,-H, D);  glVertex3f( W,-H,-D); glVertex3f( W,-H, D);
    glEnd();

    glLineWidth(1.0f);
}

// ──────────────────────────────────────────────────────────────
//  Generic planar shadow matrix
//  Plane equation: a*x + b*y + c*z + d = 0
//  Light at (lx, ly, lz, 1)
// ──────────────────────────────────────────────────────────────
static void buildShadowMatrix(float mat[16],
                               float a, float b, float c, float d,
                               float lx, float ly, float lz)
{
    float dot = a*lx + b*ly + c*lz + d;
    mat[ 0] = dot - a*lx;  mat[ 4] =     - a*ly;  mat[ 8] =     - a*lz;  mat[12] =     - a;
    mat[ 1] =     - b*lx;  mat[ 5] = dot - b*ly;  mat[ 9] =     - b*lz;  mat[13] =     - b;
    mat[ 2] =     - c*lx;  mat[ 6] =     - c*ly;  mat[10] = dot - c*lz;  mat[14] =     - c;
    mat[ 3] =     - d*lx;  mat[ 7] =     - d*ly;  mat[11] =     - d*lz;  mat[15] = dot - d;
}

static void renderShadowOnPlane(float a, float b, float c, float d,
                                 float lx, float ly, float lz)
{
    float mat[16];
    buildShadowMatrix(mat, a, b, c, d, lx, ly, lz);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.35f);

    glPushMatrix();
    glMultMatrixf(mat);

    // Ball shadow
    glPushMatrix();
    glTranslatef(gBallPos.x, gBallPos.y, gBallPos.z);
    glutSolidSphere(BALL_RADIUS, 10, 10);
    glPopMatrix();

    // Active-layer brick shadows
    for (size_t i = 0; i < gBricks.size(); i++) {
        if (gBricks[i].broken || gBricks[i].layer != gCurrentLayer) continue;
        const Vec3& c2 = gBricks[i].center;
        drawSolidBox(c2.x, c2.y, c2.z, BRICK_W*0.5f, BRICK_H*0.5f, BRICK_D*0.5f);
    }

    // Paddle shadow
    drawSolidBox(gPaddleX, gPaddleY, PADDLE_Z_POS,
                 gPaddleWCur, gPaddleHCur, PADDLE_DEPTH);

    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

static void drawAllWallShadows(float lx, float ly, float lz)
{
    //  Left   plane  x = -ARENA_W  →  1·x + ARENA_W = 0
    renderShadowOnPlane( 1.0f, 0.0f, 0.0f,  ARENA_W, lx, ly, lz);
    //  Right  plane  x = +ARENA_W  → -1·x + ARENA_W = 0
    renderShadowOnPlane(-1.0f, 0.0f, 0.0f,  ARENA_W, lx, ly, lz);
    //  Bottom plane  y = -ARENA_H  →  1·y + ARENA_H = 0
    renderShadowOnPlane( 0.0f, 1.0f, 0.0f,  ARENA_H, lx, ly, lz);
    //  Top    plane  y = +ARENA_H  → -1·y + ARENA_H = 0
    renderShadowOnPlane( 0.0f,-1.0f, 0.0f,  ARENA_H, lx, ly, lz);
}

// ──────────────────────────────────────────────────────────────
//  Draw bricks (2 passes: dimmed future/past, bright active)
// ──────────────────────────────────────────────────────────────
static void drawBricks()
{
    // Pass 1: non-active layers, back-to-front, semi-transparent
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (int L = NUM_LAYERS - 1; L >= 0; L--) {
        if (L == gCurrentLayer) continue;

        for (size_t i = 0; i < gBricks.size(); i++) {
            if (gBricks[i].broken || gBricks[i].layer != L) continue;

            const float* col = LAYER_COLOR[L];
            const Vec3&  ctr = gBricks[i].center;
            bool  isFuture   = (L > gCurrentLayer);
            float dim        = isFuture ? 0.38f : 0.15f;
            float alp        = isFuture ? 0.50f : 0.20f;

            GLfloat diff[] = { col[0]*dim, col[1]*dim, col[2]*dim, alp };
            GLfloat ambi[] = { col[0]*dim*0.5f, col[1]*dim*0.5f, col[2]*dim*0.5f, alp };
            GLfloat spec[] = { 0.2f, 0.2f, 0.2f, alp };
            glMaterialfv(GL_FRONT, GL_DIFFUSE,  diff);
            glMaterialfv(GL_FRONT, GL_AMBIENT,  ambi);
            glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
            glMaterialf (GL_FRONT, GL_SHININESS, 20.0f);
            drawSolidBox(ctr.x, ctr.y, ctr.z, BRICK_W*0.5f, BRICK_H*0.5f, BRICK_D*0.5f);

            glDisable(GL_LIGHTING);
            glColor4f(col[0], col[1], col[2], alp * 0.6f);
            glLineWidth(0.6f);
            drawWireBox(ctr.x, ctr.y, ctr.z,
                        BRICK_W*0.5f+0.03f, BRICK_H*0.5f+0.03f, BRICK_D*0.5f+0.03f);
            glEnable(GL_LIGHTING);
        }
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // Pass 2: active layer – fully opaque with animated shimmer
    int          L   = gCurrentLayer;
    const float* lc  = LAYER_COLOR[L];

    for (size_t i = 0; i < gBricks.size(); i++) {
        if (gBricks[i].broken || gBricks[i].layer != L) continue;

        const Vec3& ctr = gBricks[i].center;
        float shimPhase = gBrickShimmer * 0.04f + ctr.x * 0.3f + ctr.y * 0.2f;
        float shimmer   = 0.85f + 0.15f * sinf(shimPhase);

        float cr = lc[0], cg = lc[1], cb = lc[2];
        if (gFireball) { cr = 1.0f; cg = 0.35f * shimmer; cb = 0.0f; }

        GLfloat diff[] = { cr*shimmer, cg*shimmer, cb*shimmer, 1.0f };
        GLfloat ambi[] = { cr*0.4f,   cg*0.4f,   cb*0.4f,   1.0f };
        GLfloat spec[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  diff);
        glMaterialfv(GL_FRONT, GL_AMBIENT,  ambi);
        glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
        glMaterialf (GL_FRONT, GL_SHININESS, 90.0f);
        drawSolidBox(ctr.x, ctr.y, ctr.z, BRICK_W*0.5f, BRICK_H*0.5f, BRICK_D*0.5f);

        glDisable(GL_LIGHTING);
        glColor3f(cr, cg, cb);
        glLineWidth(gFireball ? 2.5f : 1.5f);
        drawWireBox(ctr.x, ctr.y, ctr.z,
                    BRICK_W*0.5f+0.04f, BRICK_H*0.5f+0.04f, BRICK_D*0.5f+0.04f);
        glEnable(GL_LIGHTING);
    }
}

// ──────────────────────────────────────────────────────────────
//  Draw paddle (tinted by active power-up, shield ring)
// ──────────────────────────────────────────────────────────────
static void drawPaddle()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);

    float pr = 0.0f, pg = 0.85f, pb = 1.0f;   // default: cyan
    if (gWidePaddleTimer > 0)  { pr = 0.0f;  pg = 1.0f;   pb = 0.4f; }  // green
    if (gShrinkTimer    > 0)   { pr = 0.8f;  pg = 0.0f;   pb = 0.8f; }  // purple
    if (gSlowBallTimer  > 0)   { pr = 0.2f;  pg = 0.5f;   pb = 1.0f; }  // blue
    if (gFastBallTimer  > 0)   { pr = 1.0f;  pg = 0.5f;   pb = 0.0f; }  // orange
    if (gScoreTimer     > 0)   { pr = 1.0f;  pg = 1.0f;   pb = 0.0f; }  // yellow
    if (gFireball)              { pr = 1.0f;  pg = 0.35f;  pb = 0.0f; }  // fire orange
    if (gShield)                { pr = 0.4f;  pg = 0.9f;   pb = 1.0f; }  // ice blue

    float pulseAlpha = 0.30f + 0.08f * sinf(gGlobalTime * 0.12f);
    glColor4f(pr, pg, pb, pulseAlpha);
    drawSolidBox(gPaddleX, gPaddleY, PADDLE_Z_POS,
                 gPaddleWCur, gPaddleHCur, PADDLE_DEPTH);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // Bright wireframe outline
    glColor3f(pr > 0.0f ? pr : 0.0f, pg, pb);
    glLineWidth(2.8f);
    drawWireBox(gPaddleX, gPaddleY, PADDLE_Z_POS,
                gPaddleWCur + 0.05f, gPaddleHCur + 0.05f, PADDLE_DEPTH + 0.05f);
    glLineWidth(1.0f);

    // Pulsing shield ring
    if (gShield) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float sr  = sqrtf(gPaddleWCur*gPaddleWCur + gPaddleHCur*gPaddleHCur) + 0.5f;
        float sAlpha = 0.4f + 0.4f * sinf(gGlobalTime * 0.18f);
        glColor4f(0.4f, 0.9f, 1.0f, sAlpha);
        glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP);
        for (int k = 0; k < 48; k++) {
            float a = k * 6.2831f / 48.0f;
            glVertex3f(gPaddleX + cosf(a)*sr, gPaddleY + sinf(a)*sr, PADDLE_Z_POS);
        }
        glEnd();
        glLineWidth(1.0f);
        glDisable(GL_BLEND);
    }
}

// ──────────────────────────────────────────────────────────────
//  Draw ball (rainbow trail + glow halo + lit sphere)
// ──────────────────────────────────────────────────────────────
static void drawBall()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Rainbow trail
    for (size_t i = 0; i < gTrail.size(); i++) {
        const TrailPoint& tp = gTrail[i];
        float trailSz = BALL_RADIUS * (0.4f + tp.age * 0.8f);
        glColor4f(tp.r, tp.g, tp.b, tp.age * 0.55f);
        glPushMatrix();
        glTranslatef(tp.pos.x, tp.pos.y, tp.pos.z);
        glutSolidSphere(trailSz, 8, 8);
        glPopMatrix();
    }

    // Outer glow
    if (gFireball) {
        glColor4f(1.0f, 0.4f, 0.0f, 0.15f + 0.1f * sinf(gGlobalTime * 0.3f));
        glPushMatrix();
        glTranslatef(gBallPos.x, gBallPos.y, gBallPos.z);
        glutSolidSphere(BALL_RADIUS * 3.5f, 12, 12);
        glPopMatrix();
    } else {
        glColor4f(0.5f, 0.85f, 1.0f, 0.10f);
        glPushMatrix();
        glTranslatef(gBallPos.x, gBallPos.y, gBallPos.z);
        glutSolidSphere(BALL_RADIUS * 2.6f, 10, 10);
        glPopMatrix();
    }

    glDisable(GL_BLEND);

    // Lit core sphere
    glEnable(GL_LIGHTING);
    float br = gFireball ? 1.0f : 0.9f;
    float bg = gFireball ? 0.4f : 1.0f;
    float bb = gFireball ? 0.0f : 1.0f;
    GLfloat bd[] = { br, bg, bb, 1.0f };
    GLfloat bs[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   bd);
    glMaterialfv(GL_FRONT, GL_AMBIENT,   bd);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  bs);
    glMaterialf (GL_FRONT, GL_SHININESS, 120.0f);
    glPushMatrix();
    glTranslatef(gBallPos.x, gBallPos.y, gBallPos.z);
    glutSolidSphere(BALL_RADIUS, 16, 16);
    glPopMatrix();
}

// ──────────────────────────────────────────────────────────────
//  Draw particles
// ──────────────────────────────────────────────────────────────
static void drawParticles()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (size_t i = 0; i < gParticles.size(); i++) {
        const Particle& p = gParticles[i];
        glPointSize(p.size * p.life);
        glColor4f(p.r, p.g, p.b, p.life * 0.85f);
        glBegin(GL_POINTS);
        glVertex3f(p.pos.x, p.pos.y, p.pos.z);
        glEnd();
    }
    glDisable(GL_BLEND);
}

// ──────────────────────────────────────────────────────────────
//  Draw shockwave rings
// ──────────────────────────────────────────────────────────────
static void drawShockwaves()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (size_t i = 0; i < gShockwaves.size(); i++) {
        const Shockwave& sw = gShockwaves[i];
        if (!sw.active) continue;

        glLineWidth(2.5f);
        glColor4f(0.3f, 0.9f, 1.0f, sw.alpha * 0.8f);
        glBegin(GL_LINE_LOOP);
        for (int k = 0; k < 48; k++) {
            float a = k * 6.2831f / 48.0f;
            glVertex3f(sw.pos.x + cosf(a)*sw.radius,
                       sw.pos.y + sinf(a)*sw.radius,
                       sw.pos.z);
        }
        glEnd();

        glColor4f(0.6f, 1.0f, 0.8f, sw.alpha * 0.4f);
        glBegin(GL_LINE_LOOP);
        for (int k = 0; k < 36; k++) {
            float a = k * 6.2831f / 36.0f;
            glVertex3f(sw.pos.x + cosf(a)*sw.radius*0.7f,
                       sw.pos.y + sinf(a)*sw.radius*0.7f,
                       sw.pos.z);
        }
        glEnd();
    }
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}

// ──────────────────────────────────────────────────────────────
//  Draw power-up gems (spinning octahedra with floating labels)
// ──────────────────────────────────────────────────────────────
static void drawPowerUps()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (size_t i = 0; i < gPowerUps.size(); i++) {
        const PowerUp& pu  = gPowerUps[i];
        if (!pu.active) continue;

        const float* col = PU_COLOR[pu.type];
        bool  danger     = (pu.type == PU_FAST_BALL || pu.type == PU_SHRINK_PADDLE);
        float pulse      = 0.5f + 0.5f * sinf(gGlobalTime * 0.15f + (float)i);

        // Spinning gem
        glPushMatrix();
        glTranslatef(pu.pos.x, pu.pos.y, pu.pos.z);
        glRotatef(pu.spin, 0.0f, 1.0f, 1.0f);

        if (danger)
            glColor4f(1.0f, 0.1f * pulse, 0.1f * pulse, 0.20f);
        else
            glColor4f(col[0], col[1], col[2], 0.18f);
        glutSolidSphere(1.0f, 8, 8);

        glColor4f(col[0], col[1], col[2], 0.92f);
        glScalef(0.52f, 0.52f, 0.52f);
        glutSolidOctahedron();
        glColor4f(1.0f, 1.0f, 1.0f, 0.75f);
        glutWireOctahedron();
        glPopMatrix();

        // Pulsing identification ring
        float ringR = 1.3f + 0.25f * sinf(gGlobalTime * 0.2f);
        glLineWidth(danger ? 3.0f : 2.0f);
        if (danger) glColor4f(1.0f, 0.15f, 0.15f, 0.85f * pulse);
        else        glColor4f(0.2f, 1.0f,  0.6f,  0.7f);
        glBegin(GL_LINE_LOOP);
        for (int k = 0; k < 32; k++) {
            float a = k * 6.2831f / 32.0f;
            glVertex3f(pu.pos.x + cosf(a)*ringR, pu.pos.y + sinf(a)*ringR, pu.pos.z);
        }
        glEnd();
        glLineWidth(1.0f);

        // Floating label above gem
        const char* label = danger
            ? (pu.type == PU_FAST_BALL ? "!!FAST!!" : "!!SHRINK!!")
            : PU_LABEL[pu.type];
        float labelR = danger ? 1.0f        : col[0];
        float labelG = danger ? 0.2f*pulse  : col[1];
        float labelB = danger ? 0.2f*pulse  : col[2];
        glColor4f(labelR, labelG, labelB, 0.95f);
        glRasterPos3f(pu.pos.x - 0.6f, pu.pos.y + 2.0f, pu.pos.z);
        for (const char* p = label; *p; p++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p);
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// ──────────────────────────────────────────────────────────────
//  HUD helpers
// ──────────────────────────────────────────────────────────────
static void drawText2D(float x, float y, const char* s,
                       float r, float g, float b,
                       void* font = GLUT_BITMAP_HELVETICA_18)
{
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (const char* p = s; *p; p++)
        glutBitmapCharacter(font, *p);
}

static void drawText2DAlpha(float x, float y, const char* s,
                             float r, float g, float b, float a,
                             void* font = GLUT_BITMAP_HELVETICA_18)
{
    glColor4f(r, g, b, a);
    glRasterPos2f(x, y);
    for (const char* p = s; *p; p++)
        glutBitmapCharacter(font, *p);
}

static void drawLayerProgressDots()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < NUM_LAYERS; i++) {
        float cy = 0.88f + i * (-0.155f);
        float cr = LAYER_COLOR[i][0];
        float cg = LAYER_COLOR[i][1];
        float cb = LAYER_COLOR[i][2];
        float s  = 0.022f;

        if (i < gCurrentLayer) {
            glColor4f(cr*0.25f, cg*0.25f, cb*0.25f, 0.35f);
        } else if (i == gCurrentLayer) {
            s = 0.028f;
            float pulse = 0.9f + 0.1f * sinf(gGlobalTime * 0.15f);
            glColor4f(cr*pulse, cg*pulse, cb*pulse, 1.0f);
        } else {
            glColor4f(cr*0.5f, cg*0.5f, cb*0.5f, 0.6f);
        }

        glBegin(GL_QUADS);
        glVertex2f(0.87f - s, cy - s);  glVertex2f(0.87f + s, cy - s);
        glVertex2f(0.87f + s, cy + s);  glVertex2f(0.87f - s, cy + s);
        glEnd();
    }
    glDisable(GL_BLEND);
}

static void drawLivesHearts()
{
    for (int i = 0; i < gLives; i++) {
        glColor3f(1.0f, 0.25f, 0.35f);
        glRasterPos2f(-0.97f + i * 0.08f, 0.74f);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '*');
    }
}

static void drawTimerBar(float x, float y, float filled,
                          float r, float g, float b, const char* label)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.15f, 0.15f, 0.15f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f(x,        y);        glVertex2f(x + 0.30f, y);
    glVertex2f(x + 0.30f, y + 0.05f); glVertex2f(x,        y + 0.05f);
    glEnd();

    glColor4f(r, g, b, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(x,              y);
    glVertex2f(x + 0.30f*filled, y);
    glVertex2f(x + 0.30f*filled, y + 0.05f);
    glVertex2f(x,              y + 0.05f);
    glEnd();

    glDisable(GL_BLEND);
    drawText2D(x + 0.01f, y + 0.008f, label, 0.0f, 0.0f, 0.0f,
               GLUT_BITMAP_HELVETICA_12);
}

// ──────────────────────────────────────────────────────────────
//  Ranking helpers
// ──────────────────────────────────────────────────────────────
static const char* getScoreRank(int score)
{
    if (score >= 8000) return "S";
    if (score >= 5000) return "A";
    if (score >= 2500) return "B";
    return "C";
}

static void getRankColour(int score, float& r, float& g, float& b)
{
    if      (score >= 8000) { r = 1.0f;  g = 0.9f;  b = 0.0f; }  // gold  S
    else if (score >= 5000) { r = 0.6f;  g = 1.0f;  b = 0.4f; }  // green A
    else if (score >= 2500) { r = 0.4f;  g = 0.7f;  b = 1.0f; }  // blue  B
    else                    { r = 0.7f;  g = 0.7f;  b = 0.7f; }  // grey  C
}

// ──────────────────────────────────────────────────────────────
//  Trajectory prediction + landing crosshair
// ──────────────────────────────────────────────────────────────
static void computeAndDrawTrajectory()
{
    if (gBallOnPaddle || gGameOver || gYouWin || gBallVel.z <= 0.0f) return;

    gPredPulse += 0.08f;
    if (gPredPulse > 6.2831f) gPredPulse -= 6.2831f;

    // Simulate ball forward (walls only, ignore bricks)
    const int   MAX_SIM_STEPS = 3000;
    const float SIM_STEP_SIZE = 0.25f;
    float px = gBallPos.x, py = gBallPos.y, pz = gBallPos.z;
    float vx = gBallVel.x * (SIM_STEP_SIZE / BALL_SPEED);
    float vy = gBallVel.y * (SIM_STEP_SIZE / BALL_SPEED);
    float vz = gBallVel.z * (SIM_STEP_SIZE / BALL_SPEED);
    float targetZ = PADDLE_Z_POS - PADDLE_DEPTH;

    std::vector<Vec3> path;
    path.push_back(Vec3(px, py, pz));

    for (int s = 0; s < MAX_SIM_STEPS; s++) {
        px += vx;  py += vy;  pz += vz;

        if (px + BALL_RADIUS > ARENA_W)  { px =  ARENA_W - BALL_RADIUS; vx *= -1.0f; path.push_back(Vec3(px,py,pz)); }
        if (px - BALL_RADIUS < -ARENA_W) { px = -ARENA_W + BALL_RADIUS; vx *= -1.0f; path.push_back(Vec3(px,py,pz)); }
        if (py + BALL_RADIUS > ARENA_H)  { py =  ARENA_H - BALL_RADIUS; vy *= -1.0f; path.push_back(Vec3(px,py,pz)); }
        if (py - BALL_RADIUS < -ARENA_H) { py = -ARENA_H + BALL_RADIUS; vy *= -1.0f; path.push_back(Vec3(px,py,pz)); }
        if (pz - BALL_RADIUS < -ARENA_D) { pz = -ARENA_D + BALL_RADIUS; vz *= -1.0f; path.push_back(Vec3(px,py,pz)); }

        if (pz + BALL_RADIUS >= targetZ && vz > 0.0f) {
            float t = (targetZ - (pz - vz)) / vz;
            px = (px - vx) + vx * t;
            py = (py - vy) + vy * t;
            path.push_back(Vec3(px, py, targetZ));
            break;
        }
    }

    gPredX = path.back().x;
    gPredY = path.back().y;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST);

    // Rainbow dashed trajectory line
    glLineWidth(1.5f);
    float dashLen = 0.8f, gapLen = 0.6f, segBuf = 0.0f;
    bool  drawingDash = true;

    glBegin(GL_LINES);
    for (size_t i = 0; i + 1 < path.size(); i++) {
        Vec3  a  = path[i];
        Vec3  b2 = path[i + 1];
        float dx = b2.x - a.x, dy = b2.y - a.y, dz = b2.z - a.z;
        float sl = sqrtf(dx*dx + dy*dy + dz*dz);
        if (sl < 1e-5f) continue;
        float nx = dx/sl, ny = dy/sl, nz = dz/sl;
        float walked = 0.0f;

        while (walked < sl) {
            float remaining = (drawingDash ? dashLen : gapLen) - segBuf;
            float seg       = fminf(remaining, sl - walked);

            if (drawingDash) {
                float tt = (float)i / (float)(path.size() - 1);
                float tr, tg, tb;
                hsvToRgb(tt * 0.5f + 0.5f, 0.7f, 1.0f, tr, tg, tb);
                glColor4f(tr, tg, tb, 0.7f - tt * 0.3f);
                glVertex3f(a.x + nx*walked,       a.y + ny*walked,       a.z + nz*walked);
                glVertex3f(a.x + nx*(walked+seg),  a.y + ny*(walked+seg),  a.z + nz*(walked+seg));
            }

            walked  += seg;
            segBuf  += seg;
            if (segBuf >= (drawingDash ? dashLen : gapLen) - 1e-4f) {
                segBuf = 0.0f;
                drawingDash = !drawingDash;
            }
        }
    }
    glEnd();

    // Landing crosshair + pulsing ring
    float pulse  = 0.55f + 0.45f * sinf(gPredPulse);
    float landZ  = PADDLE_Z_POS - PADDLE_DEPTH - 0.05f;
    float ringR  = 0.8f + 0.3f * sinf(gPredPulse);

    glLineWidth(2.0f);
    glColor4f(1.0f, 1.0f, 0.0f, pulse * 0.9f);
    glBegin(GL_LINE_LOOP);
    for (int k = 0; k < 24; k++) {
        float a = k * 6.2831f / 24.0f;
        glVertex3f(gPredX + cosf(a)*ringR, gPredY + sinf(a)*ringR, landZ);
    }
    glEnd();

    glPointSize(8.0f);
    glColor4f(1.0f, 0.9f, 0.0f, pulse);
    glBegin(GL_POINTS);
    glVertex3f(gPredX, gPredY, landZ);
    glEnd();

    glLineWidth(1.2f);
    glColor4f(1.0f, 1.0f, 0.0f, pulse * 0.6f);
    float ch = 1.5f;
    glBegin(GL_LINES);
    glVertex3f(gPredX - ch, gPredY,      landZ);  glVertex3f(gPredX + ch, gPredY,      landZ);
    glVertex3f(gPredX,      gPredY - ch, landZ);  glVertex3f(gPredX,      gPredY + ch, landZ);
    glEnd();

    // Guide line from predicted point to current paddle centre
    glLineWidth(1.0f);
    glColor4f(0.0f, 1.0f, 1.0f, 0.35f);
    glBegin(GL_LINES);
    glVertex3f(gPredX, gPredY, landZ);
    glVertex3f(gPaddleX, gPaddleY, landZ);
    glEnd();

    // Ghost paddle outline at predicted landing position
    glLineWidth(1.8f);
    glColor4f(1.0f, 1.0f, 0.0f, pulse * 0.5f);
    float pw = gPaddleWCur, ph = gPaddleHCur;
    glBegin(GL_LINE_LOOP);
    glVertex3f(gPredX - pw, gPredY - ph, landZ);
    glVertex3f(gPredX + pw, gPredY - ph, landZ);
    glVertex3f(gPredX + pw, gPredY + ph, landZ);
    glVertex3f(gPredX - pw, gPredY + ph, landZ);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLineWidth(1.0f);
}

// ──────────────────────────────────────────────────────────────
//  Physics update  (called every 16 ms via glutTimerFunc)
// ──────────────────────────────────────────────────────────────
static void updatePhysics(int)
{
    gGlobalTime  += 1.0f;
    gTunnelPulse += 0.03f;
    if (gTunnelPulse > 6.2831f) gTunnelPulse -= 6.2831f;

    // Move stars toward camera
    for (size_t i = 0; i < gStars.size(); i++) {
        gStars[i].z += 0.12f;
        if (gStars[i].z > ARENA_D + 5.0f) {
            gStars[i].z = frand(-ARENA_D * 2.0f, -ARENA_D * 0.5f);
            gStars[i].x = frand(-ARENA_W * 4.0f,  ARENA_W * 4.0f);
            gStars[i].y = frand(-ARENA_H * 4.0f,  ARENA_H * 4.0f);
        }
    }

    // Expand shockwave rings
    for (size_t i = 0; i < gShockwaves.size(); i++) {
        if (!gShockwaves[i].active) continue;
        gShockwaves[i].radius += 0.18f;
        gShockwaves[i].alpha  -= 0.03f;
        if (gShockwaves[i].alpha <= 0.0f) gShockwaves[i].active = false;
    }
    gShockwaves.erase(
        std::remove_if(gShockwaves.begin(), gShockwaves.end(),
                       [](const Shockwave& w){ return !w.active; }),
        gShockwaves.end());

    if (!gGameOver && !gYouWin) {

        // ── Paddle movement ──────────────────────────────────
        if (gKeyLeft)  { gPaddleX -= PADDLE_SPEED; if (gPaddleX - gPaddleWCur < -ARENA_W) gPaddleX = -ARENA_W + gPaddleWCur; }
        if (gKeyRight) { gPaddleX += PADDLE_SPEED; if (gPaddleX + gPaddleWCur >  ARENA_W) gPaddleX =  ARENA_W - gPaddleWCur; }
        if (gKeyUp)    { gPaddleY += PADDLE_SPEED; if (gPaddleY + gPaddleHCur >  ARENA_H) gPaddleY =  ARENA_H - gPaddleHCur; }
        if (gKeyDown)  { gPaddleY -= PADDLE_SPEED; if (gPaddleY - gPaddleHCur < -ARENA_H) gPaddleY = -ARENA_H + gPaddleHCur; }

        if (gBallOnPaddle) {
            gBallPos = Vec3(gPaddleX, gPaddleY,
                            PADDLE_Z_POS - PADDLE_DEPTH - BALL_RADIUS - 0.1f);
        } else {
            gBallPos = gBallPos + gBallVel;

            // ── Wall bounces (all 4 sides identical) ─────────
            if (gBallPos.x + BALL_RADIUS >  ARENA_W) {
                gBallPos.x = ARENA_W - BALL_RADIUS;  gBallVel.x *= -1.0f;
                gCameraShake = 0.18f;
                gFlashAmount = 0.12f; gFlashR = 0.3f; gFlashG = 0.6f; gFlashB = 1.0f;
            }
            if (gBallPos.x - BALL_RADIUS < -ARENA_W) {
                gBallPos.x = -ARENA_W + BALL_RADIUS;  gBallVel.x *= -1.0f;
                gCameraShake = 0.18f;
                gFlashAmount = 0.12f; gFlashR = 0.3f; gFlashG = 0.6f; gFlashB = 1.0f;
            }
            if (gBallPos.y + BALL_RADIUS >  ARENA_H) {
                gBallPos.y = ARENA_H - BALL_RADIUS;  gBallVel.y *= -1.0f;
                gCameraShake = 0.18f;
                gFlashAmount = 0.12f; gFlashR = 0.3f; gFlashG = 0.6f; gFlashB = 1.0f;
            }
            if (gBallPos.y - BALL_RADIUS < -ARENA_H) {
                gBallPos.y = -ARENA_H + BALL_RADIUS;  gBallVel.y *= -1.0f;
                gCameraShake = 0.18f;
                gFlashAmount = 0.12f; gFlashR = 0.3f; gFlashG = 0.6f; gFlashB = 1.0f;
            }
            if (gBallPos.z - BALL_RADIUS < -ARENA_D) {
                gBallPos.z = -ARENA_D + BALL_RADIUS;  gBallVel.z *= -1.0f;
                gCameraShake = 0.15f;
            }

            // ── Paddle collision ─────────────────────────────
            bool inX  = gBallPos.x > gPaddleX - gPaddleWCur &&
                        gBallPos.x < gPaddleX + gPaddleWCur;
            bool inY  = gBallPos.y > gPaddleY - gPaddleHCur &&
                        gBallPos.y < gPaddleY + gPaddleHCur;
            bool hitZ = gBallPos.z + BALL_RADIUS >= PADDLE_Z_POS - PADDLE_DEPTH &&
                        gBallPos.z - BALL_RADIUS <= PADDLE_Z_POS + PADDLE_DEPTH;

            if (inX && inY && hitZ && gBallVel.z > 0.0f) {
                gBallVel.z *= -1.0f;
                float offX = (gBallPos.x - gPaddleX) / gPaddleWCur;
                float offY = (gBallPos.y - gPaddleY) / gPaddleHCur;
                gBallVel.x += offX * 0.06f;
                gBallVel.y += offY * 0.06f;
                setSpeed(gBallVel, (BALL_SPEED + gCurrentLayer * 0.012f) * gBallSpeedMul);
                gBallPos.z    = PADDLE_Z_POS - PADDLE_DEPTH - BALL_RADIUS - 0.05f;
                gCameraShake  = 0.14f;
                gFlashAmount  = 0.18f; gFlashR = 0.3f; gFlashG = 1.0f; gFlashB = 0.5f;
                spawnShockwave(Vec3(gBallPos.x, gBallPos.y, PADDLE_Z_POS));
            }

            // ── Ball missed – shield / life lost ─────────────
            if (gBallPos.z + BALL_RADIUS > ARENA_D + 1.0f) {
                if (gShield) {
                    gShield         = false;
                    gBallVel.z     *= -1.0f;
                    gBallPos.z      = PADDLE_Z_POS - PADDLE_DEPTH - BALL_RADIUS - 0.2f;
                    gBallPos.x      = gPaddleX;
                    gBallPos.y      = gPaddleY;
                    gFlashAmount    = 0.4f; gFlashR = 0.4f; gFlashG = 0.9f; gFlashB = 1.0f;
                    snprintf(gPickupMessage, sizeof(gPickupMessage), "SHIELD SAVED!");
                    gPickupMessageTimer = 150;
                } else {
                    gLives--;
                    gFlashAmount = 0.6f; gFlashR = 1.0f; gFlashG = 0.1f; gFlashB = 0.1f;
                    gCombo    = 0;
                    gComboTimer = 0;
                    if (gLives <= 0) gGameOver    = true;
                    else             gBallOnPaddle = true;
                }
            }

            // ── Brick collision ──────────────────────────────
            for (size_t i = 0; i < gBricks.size(); i++) {
                Brick& b = gBricks[i];
                if (b.broken || b.layer != gCurrentLayer) continue;

                float dx = fabsf(gBallPos.x - b.center.x);
                float dy = fabsf(gBallPos.y - b.center.y);
                float dz = fabsf(gBallPos.z - b.center.z);
                float ex = BRICK_W * 0.5f + BALL_RADIUS;
                float ey = BRICK_H * 0.5f + BALL_RADIUS;
                float ez = BRICK_D * 0.5f + BALL_RADIUS;

                if (dx < ex && dy < ey && dz < ez) {
                    // Bounce (skip if fireball – pierce mode)
                    if (!gFireball) {
                        float ox = ex - dx, oy = ey - dy, oz = ez - dz;
                        if      (oz <= ox && oz <= oy) gBallVel.z *= -1.0f;
                        else if (ox <= oy)              gBallVel.x *= -1.0f;
                        else                            gBallVel.y *= -1.0f;
                    }

                    setSpeed(gBallVel,
                             (BALL_SPEED + gCurrentLayer * 0.012f) * gBallSpeedMul);
                    b.broken = true;
                    spawnBrickParticles(b.center, b.layer);
                    trySpawnPowerUp(b.center);

                    // Combo scoring
                    gCombo++;
                    gComboTimer = 120;   // 2-second window to extend chain
                    gComboFlash = 1.0f;
                    float comboMul = 1.0f + (gCombo > 1 ? (gCombo - 1) * 0.25f : 0.0f);
                    gScore += (int)(10 * (gCurrentLayer + 1) * gScoreMul * comboMul);

                    gCameraShake = 0.22f;
                    gFlashAmount = 0.15f; gFlashR = 1.0f; gFlashG = 0.9f; gFlashB = 0.3f;

                    // Consume one fireball pierce charge
                    if (gFireball) {
                        gFireballLeft--;
                        if (gFireballLeft <= 0) {
                            gFireball      = false;
                            gFireballTimer = 0;
                        }
                    }

                    if (isLayerCleared(gCurrentLayer)) {
                        gCurrentLayer++;
                        gCombo = 0;
                        if (gCurrentLayer >= NUM_LAYERS)
                            gYouWin = true;
                        else
                            gBrickShimmer = 0.0f;
                    }
                    break;
                }
            }
        }

        // ── Power-up drift, bounce, collection ──────────────
        for (size_t i = 0; i < gPowerUps.size(); i++) {
            PowerUp& pu = gPowerUps[i];
            if (!pu.active) continue;

            pu.pos  = pu.pos + pu.vel;
            pu.spin += 3.5f;

            // Remove if out of arena bounds
            if (pu.pos.z > ARENA_D + 2.0f ||
                fabsf(pu.pos.x) > ARENA_W + 1.0f ||
                fabsf(pu.pos.y) > ARENA_H + 1.0f) {
                pu.active = false;
                continue;
            }

            // Bounce off side walls
            if (pu.pos.x + 0.8f >  ARENA_W) { pu.pos.x =  ARENA_W - 0.8f; pu.vel.x *= -1.0f; }
            if (pu.pos.x - 0.8f < -ARENA_W) { pu.pos.x = -ARENA_W + 0.8f; pu.vel.x *= -1.0f; }
            if (pu.pos.y + 0.8f >  ARENA_H) { pu.pos.y =  ARENA_H - 0.8f; pu.vel.y *= -1.0f; }
            if (pu.pos.y - 0.8f < -ARENA_H) { pu.pos.y = -ARENA_H + 0.8f; pu.vel.y *= -1.0f; }

            // Paddle collection zone
            bool nearX = pu.pos.x > gPaddleX - gPaddleWCur - 1.0f &&
                         pu.pos.x < gPaddleX + gPaddleWCur + 1.0f;
            bool nearY = pu.pos.y > gPaddleY - gPaddleHCur - 1.0f &&
                         pu.pos.y < gPaddleY + gPaddleHCur + 1.0f;
            bool nearZ = pu.pos.z > PADDLE_Z_POS - PADDLE_DEPTH - 1.2f &&
                         pu.pos.z < PADDLE_Z_POS + PADDLE_DEPTH + 0.5f;

            if (nearX && nearY && nearZ) {
                applyPowerUp(pu.type);
                pu.active = false;

                // Collect particles burst
                for (int k = 0; k < 18; k++) {
                    Particle p;
                    p.pos  = pu.pos;
                    p.vel  = Vec3(frand(-0.35f, 0.35f),
                                  frand(-0.35f, 0.35f),
                                  frand(-0.25f, 0.25f));
                    p.life = 1.0f;
                    p.size = 5.0f;
                    p.r    = PU_COLOR[pu.type][0];
                    p.g    = PU_COLOR[pu.type][1];
                    p.b    = PU_COLOR[pu.type][2];
                    gParticles.push_back(p);
                }
            }
        }

        gPowerUps.erase(
            std::remove_if(gPowerUps.begin(), gPowerUps.end(),
                           [](const PowerUp& p){ return !p.active; }),
            gPowerUps.end());

        tickPowerUpTimers();

        // Keep paddle inside arena
        if (gPaddleX - gPaddleWCur < -ARENA_W) gPaddleX = -ARENA_W + gPaddleWCur;
        if (gPaddleX + gPaddleWCur >  ARENA_W) gPaddleX =  ARENA_W - gPaddleWCur;
        if (gPaddleY - gPaddleHCur < -ARENA_H) gPaddleY = -ARENA_H + gPaddleHCur;
        if (gPaddleY + gPaddleHCur >  ARENA_H) gPaddleY =  ARENA_H - gPaddleHCur;

    }   // end if (!gGameOver && !gYouWin)

    // ── Ball trail ──────────────────────────────────────────
    if (!gBallOnPaddle) {
        TrailPoint tp;
        tp.pos = gBallPos;
        tp.age = 1.0f;
        float hue = fmod(gGlobalTime * 0.006f, 1.0f);
        if (gFireball) { tp.r = 1.0f; tp.g = 0.4f; tp.b = 0.0f; }
        else           hsvToRgb(hue, 1.0f, 1.0f, tp.r, tp.g, tp.b);
        gTrail.push_back(tp);
    }
    for (size_t i = 0; i < gTrail.size(); i++) gTrail[i].age -= 0.045f;
    gTrail.erase(
        std::remove_if(gTrail.begin(), gTrail.end(),
                       [](const TrailPoint& tp){ return tp.age <= 0.0f; }),
        gTrail.end());

    // ── Particle update ────────────────────────────────────
    for (size_t i = 0; i < gParticles.size(); i++) {
        gParticles[i].pos  = gParticles[i].pos + gParticles[i].vel;
        gParticles[i].vel.y -= 0.009f;
        gParticles[i].life  -= 0.020f;
    }
    gParticles.erase(
        std::remove_if(gParticles.begin(), gParticles.end(),
                       [](const Particle& p){ return p.life <= 0.0f; }),
        gParticles.end());

    gCameraShake *= 0.70f;
    glutPostRedisplay();
    glutTimerFunc(16, updatePhysics, 0);
}

// ──────────────────────────────────────────────────────────────
//  Display callback
// ──────────────────────────────────────────────────────────────
static void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ── Perspective 3D projection ────────────────────────
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(58.0, (double)WIN_W / WIN_H, 0.3, 300.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camera: front of tunnel, looking along -Z axis
    float shake = gCameraShake * frand(-1.0f, 1.0f);
    gluLookAt(shake,  shake,  ARENA_D + 9.0f,
              0.0f,   0.0f,   0.0f,
              0.0f,   1.0f,   0.0f);

    // ── Scene lighting ───────────────────────────────────
    float lx = 0.0f, ly = ARENA_H * 1.5f, lz = ARENA_D * 0.5f;
    GLfloat lpos[] = { lx, ly, lz, 1.0f };
    GLfloat ldif[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lamb[] = { 0.12f, 0.16f, 0.26f, 1.0f };
    GLfloat lspc[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  ldif);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lamb);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lspc);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    // ── 3D scene ─────────────────────────────────────────
    glDisable(GL_LIGHTING);
    drawStars();
    drawAllWallShadows(lx, ly, lz);
    drawGlassTunnel();
    drawWallGrid();

    // Outer box wireframe
    glColor4f(0.0f, 0.7f, 1.0f, 0.45f);
    glLineWidth(1.8f);
    drawWireBox(0.0f, 0.0f, 0.0f, ARENA_W, ARENA_H, ARENA_D);
    glLineWidth(1.0f);

    glEnable(GL_LIGHTING);
    drawBricks();
    glDisable(GL_LIGHTING);
    drawPaddle();
    drawBall();
    computeAndDrawTrajectory();
    drawShockwaves();
    drawParticles();
    drawPowerUps();

    // ── 2D HUD overlay ───────────────────────────────────
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Full-screen colour flash
    if (gFlashAmount > 0.0f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(gFlashR, gFlashG, gFlashB, gFlashAmount * 0.35f);
        glBegin(GL_QUADS);
        glVertex2f(-1,-1); glVertex2f(1,-1); glVertex2f(1,1); glVertex2f(-1,1);
        glEnd();
        glDisable(GL_BLEND);
    }

    drawLayerProgressDots();

    // Score + best score
    char buf[128];
    snprintf(buf, sizeof(buf), "SCORE: %d%s", gScore, gScoreTimer > 0 ? " [x2]" : "");
    drawText2D(-0.97f, 0.90f, buf, 0.0f, 1.0f, 1.0f);
    snprintf(buf, sizeof(buf), "BEST: %d", gHighScore);
    drawText2D(-0.97f, 0.82f, buf, 0.7f, 0.7f, 0.3f, GLUT_BITMAP_HELVETICA_12);

    drawLivesHearts();

    snprintf(buf, sizeof(buf), "LAYER: %d / %d", gCurrentLayer + 1, NUM_LAYERS);
    drawText2D(-0.97f, 0.66f, buf, 1.0f, 0.85f, 0.0f);

    // FIREBALL indicator
    if (gFireball) {
        float fb = 0.5f + 0.5f * sinf(gGlobalTime * 0.3f);
        snprintf(buf, sizeof(buf), "FIREBALL!  x%d", gFireballLeft);
        drawText2DAlpha(-0.30f, 0.55f, buf, 1.0f, 0.4f, 0.0f, fb,
                        GLUT_BITMAP_TIMES_ROMAN_24);
    }

    // Combo display
    if (gCombo > 1 && gComboFlash > 0.0f) {
        float cr, cg, cb;
        hsvToRgb((float)gCombo * 0.08f, 0.9f, 1.0f, cr, cg, cb);
        snprintf(buf, sizeof(buf), "COMBO  x%d !", gCombo);
        drawText2DAlpha(-0.22f, 0.40f, buf, cr, cg, cb, gComboFlash,
                        GLUT_BITMAP_TIMES_ROMAN_24);
    }

    // Active power-up timer bars
    float bx = -0.97f, by = -0.72f, bStep = 0.065f;
    int   barIdx = 0;

    if (gWidePaddleTimer > 0) {
        drawTimerBar(bx, by + barIdx*bStep,
                     (float)gWidePaddleTimer / 600.0f,
                     0.0f, 1.0f, 0.4f, "WIDE PADDLE");
        barIdx++;
    }
    if (gSlowBallTimer > 0) {
        drawTimerBar(bx, by + barIdx*bStep,
                     (float)gSlowBallTimer / 480.0f,
                     0.2f, 0.6f, 1.0f, "SLOW BALL");
        barIdx++;
    }
    if (gFastBallTimer > 0) {
        drawTimerBar(bx, by + barIdx*bStep,
                     (float)gFastBallTimer / 360.0f,
                     1.0f, 0.5f, 0.0f, "FAST BALL");
        barIdx++;
    }
    if (gScoreTimer > 0) {
        drawTimerBar(bx, by + barIdx*bStep,
                     (float)gScoreTimer / 600.0f,
                     1.0f, 1.0f, 0.0f, "2x SCORE");
        barIdx++;
    }
    if (gShrinkTimer > 0) {
        drawTimerBar(bx, by + barIdx*bStep,
                     (float)gShrinkTimer / 420.0f,
                     0.8f, 0.0f, 0.8f, "SHRINK !");
        barIdx++;
    }
    if (gFireballTimer > 0) {
        drawTimerBar(bx, by + barIdx*bStep,
                     (float)gFireballTimer / 360.0f,
                     1.0f, 0.35f, 0.0f, "FIREBALL");
        barIdx++;
    }
    if (gShield) {
        drawText2D(bx, by + barIdx*bStep,
                   "[ SHIELD ACTIVE ]", 0.4f, 0.9f, 1.0f,
                   GLUT_BITMAP_HELVETICA_12);
        barIdx++;
    }

    // Power-up legend  (bottom-right)
    drawText2D(0.47f, -0.68f, "POWER-UPS:", 0.6f, 0.6f, 0.6f, GLUT_BITMAP_HELVETICA_12);
    for (int i = 0; i < PU_COUNT; i++) {
        snprintf(buf, sizeof(buf), "[%s]", PU_LABEL[i]);
        drawText2D(0.47f, -0.68f - 0.072f*(i+1), buf,
                   PU_COLOR[i][0], PU_COLOR[i][1], PU_COLOR[i][2],
                   GLUT_BITMAP_HELVETICA_12);
    }

    // Pick-up flash message
    if (gPickupMessageTimer > 0) {
        float fade = (float)gPickupMessageTimer / 180.0f;
        float fy   = 0.30f + (1.0f - fade) * 0.08f;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 0.3f, fade);
        snprintf(buf, sizeof(buf), "  %s  ", gPickupMessage);
        glRasterPos2f(-0.22f, fy);
        for (const char* p = buf; *p; p++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *p);
        glDisable(GL_BLEND);
    }

    // Controls hint
    drawText2D(-0.60f, -0.94f,
               "ARROWS = MOVE   SPACE = LAUNCH   R = RESTART   ESC = QUIT",
               0.4f, 0.4f, 0.4f, GLUT_BITMAP_HELVETICA_12);

    if (gBallOnPaddle && !gGameOver && !gYouWin)
        drawText2D(-0.30f, -0.82f, "PRESS SPACE TO LAUNCH", 0.9f, 0.9f, 0.2f);

    // ── Game-over / Win screen ────────────────────────────
    if (gGameOver || gYouWin) {
        if (gScore > gHighScore) gHighScore = gScore;

        // Dark semi-transparent overlay
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.05f, 0.72f);
        glBegin(GL_QUADS);
        glVertex2f(-1,-1); glVertex2f(1,-1); glVertex2f(1,1); glVertex2f(-1,1);
        glEnd();
        glDisable(GL_BLEND);

        if (gGameOver)
            drawText2D(-0.26f,  0.22f, "GAME  OVER",  1.0f, 0.15f, 0.15f,
                       GLUT_BITMAP_TIMES_ROMAN_24);
        else
            drawText2D(-0.20f,  0.22f, "YOU  WIN !",  0.2f, 1.0f,  0.5f,
                       GLUT_BITMAP_TIMES_ROMAN_24);

        snprintf(buf, sizeof(buf), "Score :  %d", gScore);
        drawText2D(-0.16f,  0.06f, buf, 1.0f, 1.0f, 0.0f);

        snprintf(buf, sizeof(buf), "Best  :  %d", gHighScore);
        drawText2D(-0.16f, -0.06f, buf, 0.7f, 0.7f, 0.3f);

        float rr, rg, rb;
        getRankColour(gScore, rr, rg, rb);
        snprintf(buf, sizeof(buf), "RANK   %s", getScoreRank(gScore));
        drawText2D(-0.18f, -0.20f, buf, rr, rg, rb, GLUT_BITMAP_TIMES_ROMAN_24);

        drawText2D(-0.24f, -0.36f, "Press  R  to  restart", 0.6f, 0.6f, 0.6f);
    }

    glEnable(GL_DEPTH_TEST);
    glutSwapBuffers();
}

// ──────────────────────────────────────────────────────────────
//  Window reshape
// ──────────────────────────────────────────────────────────────
static void reshape(int w, int h)
{
    WIN_W = w;
    WIN_H = h ? h : 1;
    glViewport(0, 0, WIN_W, WIN_H);
}

// ──────────────────────────────────────────────────────────────
//  Keyboard input
// ──────────────────────────────────────────────────────────────
static void specialKeyDown(int key, int, int)
{
    if (key == GLUT_KEY_LEFT)  gKeyLeft  = true;
    if (key == GLUT_KEY_RIGHT) gKeyRight = true;
    if (key == GLUT_KEY_UP)    gKeyUp    = true;
    if (key == GLUT_KEY_DOWN)  gKeyDown  = true;
}

static void specialKeyUp(int key, int, int)
{
    if (key == GLUT_KEY_LEFT)  gKeyLeft  = false;
    if (key == GLUT_KEY_RIGHT) gKeyRight = false;
    if (key == GLUT_KEY_UP)    gKeyUp    = false;
    if (key == GLUT_KEY_DOWN)  gKeyDown  = false;
}

static void normalKeyboard(unsigned char key, int, int)
{
    if (key == 27)                         exit(0);        // ESC – quit
    if (key == 'r' || key == 'R')          resetGame();    // R   – restart
    if (key == ' ' && gBallOnPaddle &&
        !gGameOver   && !gYouWin) {
        gBallOnPaddle = false;
        gBallVel = Vec3(frand(-0.08f, 0.08f),
                        frand(-0.04f, 0.04f),
                        -BALL_SPEED);
        setSpeed(gBallVel, BALL_SPEED);
    }
}

// ──────────────────────────────────────────────────────────────
//  OpenGL initialisation
// ──────────────────────────────────────────────────────────────
static void initGL()
{
    glClearColor(0.01f, 0.01f, 0.06f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Depth fog to give sense of tunnel length
    glEnable(GL_FOG);
    GLfloat fogColour[] = { 0.01f, 0.01f, 0.06f, 1.0f };
    glFogi(GL_FOG_MODE,    GL_LINEAR);
    glFogfv(GL_FOG_COLOR,  fogColour);
    glFogf(GL_FOG_START,   50.0f);
    glFogf(GL_FOG_END,     120.0f);
}

// ──────────────────────────────────────────────────────────────
//  Entry point
// ──────────────────────────────────────────────────────────────
int main(int argc, char** argv)
{
    srand((unsigned int)time(nullptr));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("NeoCrash 3D  @alamin ");

    initGL();
    initStars();
    resetGame();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(normalKeyboard);
    glutSpecialFunc(specialKeyDown);
    glutSpecialUpFunc(specialKeyUp);
    glutTimerFunc(16, updatePhysics, 0);

    glutMainLoop();
    return 0;
}
