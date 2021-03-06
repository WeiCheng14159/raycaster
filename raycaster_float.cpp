// floating-point implementation for testing/comparison

#include "raycaster_float.h"
#include <math.h>

bool RayCasterFloat::IsWall(float rayX, float rayY)
{
    int tileX = static_cast<int>(rayX);
    int tileY = static_cast<int>(rayY);

    if (tileX < 0 || tileY < 0 || tileX > MAP_X - 1 || tileY > MAP_Y - 1) {
        return true;
    }
    return g_map[(tileX >> 3) + (tileY << (MAP_XS - 3))] &
           (1 << (8 - (tileX & 0x7)));
}

float p2pDist(float x1, float y1, float x2, float y2)
{
    float delta_x = x1 - x2;
    float delta_y = y1 - y2;
    return sqrt(delta_x * delta_x + delta_y * delta_y);
}

float RayCasterFloat::Distance(float playerX,
                               float playerY,
                               float rayA,
                               float *hitOffset,
                               int *hitDirection)
{
    while (rayA < 0) {
        rayA += 2.0f * M_PI;
    }
    while (rayA >= 2.0f * M_PI) {
        rayA -= 2.0f * M_PI;
    }

    float tanA = tan(rayA);
    float cotA = 1 / tanA;
    float rayX, rayY, vx, vy;
    float xOffset, yOffset, vertHitDis, horiHitDis;
    int depth = 0;
    const int maxDepth = 100;

    // Check for vertical hit
    depth = 0;
    vertHitDis = 0;
    if (sin(rayA) > 0.001) {  // rayA pointing rightward
        rayX = static_cast<int>(playerX) + 1;
        rayY = (rayX - playerX) * cotA + playerY;
        xOffset = 1;
        yOffset = xOffset * cotA;
    } else if (sin(rayA) < -0.001) {  // rayA pointing leftward
        rayX = static_cast<int>(playerX) - 0.001;
        rayY = (rayX - playerX) * cotA + playerY;
        xOffset = -1;
        yOffset = xOffset * cotA;
    } else {  // rayA pointing up or down
        rayX = playerX;
        rayY = playerY;
        xOffset = 0;
        yOffset = 0;
        depth = maxDepth;
    }

    while (depth < maxDepth) {
        if (IsWall(rayX, rayY)) {
            vertHitDis = p2pDist(playerX, playerY, rayX, rayY);
            break;
        } else {
            rayX += xOffset;
            rayY += yOffset;
            depth += 1;
        }
    }
    vx = rayX;
    vy = rayY;

    // Check for horizontal hit
    depth = 0;
    horiHitDis = 0;
    if (cos(rayA) > 0.001) {  // rayA pointing upward
        rayY = static_cast<int>(playerY) + 1;
        rayX = (rayY - playerY) * tanA + playerX;
        yOffset = 1;
        xOffset = yOffset * tanA;
    } else if (cos(rayA) < -0.001) {  // rayA pointing downward
        rayY = static_cast<int>(playerY) - 0.001;
        rayX = (rayY - playerY) * tanA + playerX;
        yOffset = -1;
        xOffset = yOffset * tanA;
    } else {  // rayA pointing leftward or rightward
        rayX = playerX;
        rayY = playerY;
        xOffset = 0;
        yOffset = 0;
        depth = maxDepth;
    }

    while (depth < maxDepth) {
        if (IsWall(rayX, rayY)) {
            horiHitDis = p2pDist(playerX, playerY, rayX, rayY);
            break;
        } else {
            rayX += xOffset;
            rayY += yOffset;
            depth += 1;
        }
    }

    if (vertHitDis < horiHitDis) {  // Vertical hit
        rayX = vx;
        rayY = vy;
        *hitDirection = true;
        *hitOffset = rayY;
    } else {  // Horizontal hit
        *hitDirection = false;
        *hitOffset = rayX;
    }

    return vertHitDis < horiHitDis ? vertHitDis : horiHitDis;
}

void RayCasterFloat::Trace(uint16_t screenX,
                           uint8_t *screenY,
                           uint8_t *textureNo,
                           uint8_t *textureX,
                           uint16_t *textureY,
                           uint16_t *textureStep)
{
    float hitOffset;
    int hitDirection;
    float deltaAngle = atanf(((int16_t) screenX - SCREEN_WIDTH / 2.0f) /
                             (SCREEN_WIDTH / 2.0f) * FOV / 2);
    float lineDistance = Distance(_playerX, _playerY, _playerA + deltaAngle,
                                  &hitOffset, &hitDirection);
    float distance = lineDistance * cos(deltaAngle);
    float dum;
    *textureX = (uint8_t)(256.0f * modff(hitOffset, &dum));
    *textureNo = hitDirection;
    *textureY = 0;
    *textureStep = 0;
    if (distance > 0) {
        uint16_t tmpY = INV_FACTOR / distance;
        auto txs = (tmpY * 2.0f);
        *screenY = tmpY;
        if (txs != 0) {
            *textureStep = (256 / txs) * 256;
            if (txs > SCREEN_HEIGHT) {
                *screenY = SCREEN_HEIGHT >> 1;
                auto wallHeight = (txs - SCREEN_HEIGHT) / 2;
                *textureY = wallHeight * (256 / txs) * 256;
            }
        }
    } else {
        *screenY = 0;
    }
}

void RayCasterFloat::Start(uint16_t playerX, uint16_t playerY, int16_t playerA)
{
    _playerX = (playerX / 1024.0f) * 4.0f;
    _playerY = (playerY / 1024.0f) * 4.0f;
    _playerA = (playerA / 1024.0f) * 2.0f * M_PI;
}

RayCasterFloat::RayCasterFloat() : RayCaster() {}

RayCasterFloat::~RayCasterFloat() {}
