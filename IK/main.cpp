#include <C:/raylib/raylib/src/raylib.h>
#include <C:/raylib/raylib/src/raymath.h>
#define  RAYGUI_IMPLEMENTATION
#include <cmath>
#include <string>
#include <vector>
#include "raygui.h"
#include <math.h>

struct Bone
{
    Vector2 position;
    Vector2 direction;
    float length;
    float anlge;
    std::string name = "Bone";

    Bone(const Vector2& position, float anlge, float length, const std::string& name)
        : position(position),
          anlge(anlge),
          length(length),
          name(name)
    {
        direction = Vector2{0, 1};
    }
};

struct Legs
{
    float hipAngle;
    float kneeAngle;
    float ankleAngle;
};

__readonly float TEXTOFFSETX = 30;
__readonly float TEXTOFFSETY = -10;


Vector3 ToVector3(const Vector2& vector2)
{
    return Vector3{vector2.x, vector2.y, 0};
}

Vector2 RotateVector2(Vector2 vector, float angle);

void DrawJoint(Bone* hip, Bone* knee)
{
    DrawCircle(hip->position.x, hip->position.y, 10, RED);


    GuiSliderPro(Rectangle{hip->position.x + TEXTOFFSETX, hip->position.y + TEXTOFFSETY, 60, 20}, "-90",
                 "90", &hip->anlge, -90, 90, 10);

    const Vector2 rotated = RotateVector2(Vector2Scale(hip->direction, knee->length), hip->anlge);

    DrawLine(hip->position.x, hip->position.y,
             hip->position.x + rotated.x, hip->position.y + rotated.y,
             Color{250, 0, 0, 255});
    knee->position = hip->position + rotated;
    knee->direction = Vector2Normalize(rotated);
}

int main(int argc, char* argv[])
{
    InitWindow(800, 450, "2D 2-Segment IK solver");
    SetTargetFPS(60);


    Vector2 pelvisPos = Vector2{100, 100};
    Bone* hip = new Bone(pelvisPos, 0, 0, "Pelvis");
    Bone* thigh = new Bone(Vector2{1, 0}, 0, 100, "Thigh");
    Bone* shin = new Bone(Vector2{1, 0}, 0, 100, "Shin");
    Bone* foot = new Bone(Vector2(), 0, 50, "Foot");


    Vector2 controlCircle = Vector2{20, 20};
    Camera3D camera;
    camera.position = {0.0f, 0.0f, 200.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_ORTHOGRAPHIC;
    UpdateCamera(&camera, 0);


    bool dragging = false;
    Vector2 dragOffset = {0, 0};

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            CheckCollisionPointCircle(mouse, controlCircle, 20))
        {
            dragging = true;
            dragOffset = {mouse.x - controlCircle.x, mouse.y - controlCircle.y};
        }

        if (dragging)
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                controlCircle = {mouse.x - dragOffset.x, mouse.y - dragOffset.y};
            else
                dragging = false;
        }

        DrawCircle(controlCircle.x, controlCircle.y, 10, GREEN);

        float L1 = thigh->length;
        float L2 = shin->length;
        auto legLength = L1 + L2;
        auto localTargetPos = controlCircle - hip->position;
        auto r = Clamp(Vector2Length(localTargetPos), 0.1f, legLength);
        DrawText(std::to_string(r).c_str(), controlCircle.x + TEXTOFFSETX,
                 controlCircle.y + TEXTOFFSETY, 20, BLACK);

        DrawCircle(hip->position.x, hip->position.y, 10, RED);

        float theta2Rad = PI- acosf(
            Clamp((L1 * L1 + L2*L2 - r*r) / (2.0f * L1 * L2), -1.0f, 1.0f));

        float phiRad = atan2f( localTargetPos.y, localTargetPos.x);

        float psiRad = acos(Clamp((r*r + L1*L1 - L2 * L2) / (2.0f    * L1 * r), -1.0f, 1.0f));

        float theta1Rad = phiRad - psiRad;

        auto rotatedTheta1 = RotateVector2(Vector2UnitX, theta1Rad);

        auto rotatedTheta2 = RotateVector2(Vector2UnitX, theta1Rad + theta2Rad);


        Vector2 thighEndPos = Vector2{hip->position.x, hip->position.y} + rotatedTheta1 * L1;
        DrawLine(hip->position.x, hip->position.y
                 , thighEndPos.x, thighEndPos.y,
                 BLACK);

        DrawLine(thighEndPos.x, thighEndPos.y, thighEndPos.x + L2 * rotatedTheta2.x,
                 thighEndPos.y + L2 * rotatedTheta2.y, BLACK);


        EndDrawing();
    }
    CloseWindow();

    return 0;
}

Vector2 RotateVector2(const Vector2 vector, const float angle)
{
    float radAngle = angle;
    const float x = cos(radAngle) * vector.x - vector.y * sin(radAngle);
    const float y = sin(radAngle) * vector.x + vector.y * cos(radAngle);

    return Vector2{x, y};
}
