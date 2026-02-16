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

// struct Pelvis : Bone
// {
//     Pelvis() = default;
//
//     Pelvis(Vector2 vector2, int i)
//     {
//         position = vector2;
//         anlge = static_cast<float>(i);
//         name = "Pelvis";
//     }
// };


struct Shin : Bone
{
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

// void DrawJoint3D(Bone* hip, Bone* knee)
// {
//     DrawSphere(ToVector3(hip->position), 10, RED);
//
//
//     GuiSliderPro(Rectangle{hip->position.x + TEXTOFFSETX, hip->position.y + TEXTOFFSETY, 60, 20}, "-90",
//                  "90", &hip->anlge, -90, 90, 10);
//
//     const Vector2 rotated = RotateVector2(Vector2Scale(hip->direction, knee->length), hip->anlge);
//
//     DrawCapsuleWires(ToVector3(hip->position), ToVector3(hip->position) + ToVector3(rotated),
//                 30, 2, 2, Color{250, 0, 0, 255});
//     knee->position = hip->position + rotated;
//     knee->direction = Vector2Normalize(rotated);
// }


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
    InitWindow(800, 450, "raylib works");
    SetTargetFPS(60);


    Vector2 pelvisPos = Vector2{100, 100};
    Bone* hip = new Bone(pelvisPos, 0, 0, "Pelvis");
    Bone* thigh = new Bone(Vector2{1,0}, 0, 100, "Thigh");

    Bone* shin = new Bone(Vector2{1,0}, 0, 100, "Shin");


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
        // DrawJoint(hip, thigh);
        // DrawJoint(thigh, shin);
        // DrawJoint(shin, foot);

        auto legLength = thigh->length + shin->length;
        auto localTargetPos = controlCircle - hip->position;
        auto hipToTargetDistance = std::min(Vector2Distance(hip->position, hip->position + localTargetPos), legLength);
        DrawText(std::to_string(hipToTargetDistance).c_str(), controlCircle.x + TEXTOFFSETX,
                 controlCircle.y + TEXTOFFSETY, 20, BLACK);
        //
        // auto theta2 = acosf((powf(hip->length,2) + pow(shin->length,2) - pow(hipToTargetDistance,2))
        //     / 2 * hip->length * shin->length);
        //
        // auto psi = acosf((pow(hip->length,2) - pow(shin->length,2) + pow(hipToTargetDistance,2)/2 * hip->length * hipToTargetDistance);
        //
        // auto theta1 = psi + theta2;
        //
        // auto rotatedThigh = RotateVector2(thigh->direction, theta1);
        // DrawLine(hip->position.x, hip->position.y, rotatedThigh.x * thigh->length, rotatedThigh.y * thigh->length, BLACK);
        // auto rotatedShin = RotateVector2(shin->direction, theta2);
        //
        // auto shinEndPos = hip->position + rotatedShin;
        //
        //
        DrawCircle(hip->position.x, hip->position.y, 10, RED);
        // DrawLine(hip->position.x, hip->position.y, hip->position.x + thigh->direction.x * 100,
        //          hip->position.y + thigh->direction.y * 100, BLACK);

        // DrawLine(shinEndPos.x, shinEndPos.y, (shinEndPos + shin->position).x, (shinEndPos + shin->position).y, BLACK);        
        //

        float theta2Rad = acosf(
            (powf(thigh->length, 2) + (powf(shin->length, 2) - pow(hipToTargetDistance, 2))) / (2 * thigh->length * shin
                ->length));
        float fiRad = atan2f(controlCircle.y, controlCircle.x);

        float psiRad = acos(
            (powf(hipToTargetDistance, 2) - powf(thigh->length, 2) - pow(shin->length, 2)) / (shin->length * thigh->
                length));
        float theta1Rad = fiRad - psiRad;

        auto rotatedTheta1 = RotateVector2(thigh->direction, theta1Rad);

        auto rotatedTheta2 = RotateVector2(shin->direction, theta2Rad);

        shin->position = hip->position + rotatedTheta1 * thigh->length;
        DrawLine(hip->position.x, hip->position.y, hip->position.x + rotatedTheta1.x, hip->position.y + rotatedTheta1.y,
                 GREEN);
        DrawLine(shin->position.x, shin->position.y, shin->position.x + rotatedTheta2.x,
                 shin->position.y + rotatedTheta2.y, YELLOW);
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
