#include <C:/raylib/raylib/src/raylib.h>
#include <C:/raylib/raylib/src/raymath.h>
#include <cmath>
#include <string>
#include <vector>

__readonly constexpr float textoffsetx = 30;
__readonly constexpr float textoffsety = -10;

 
struct Bone
{
    Vector2 position;
    Vector2 direction;
    float length;
    float anlge;
    std::string name = "Bone";

    Bone(const Vector2& position, float anlge, float length, const std::string& name)
        : position(position),
          length(length),
          anlge(anlge),
          name(name)
    {
        direction = Vector2{0, 1};
    }
};

static Vector2 rotate_vector2(Vector2 vector, float angle);

int main(int argc, char* argv[])
{
    InitWindow(800, 450, "2D 2-Segment IK solver");
    SetTargetFPS(60);


    Vector2 pelvisPos = Vector2{100, 100};
    Bone* hip = new Bone(pelvisPos, 0, 0, "Pelvis");
    Bone* thigh = new Bone(Vector2{1, 0}, 0, 100, "Thigh");
    Bone* shin = new Bone(Vector2{1, 0}, 0, 100, "Shin");


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

        DrawCircle(controlCircle.x,  controlCircle.y, 10, GREEN);

        float L1 = thigh->length;
        float L2 = shin->length;
        auto legLength = L1 + L2;
        auto localTargetPos = controlCircle - hip->position;
        auto r = Clamp(Vector2Length(localTargetPos), 0.1f, legLength);
        DrawText(std::to_string(r).c_str(), controlCircle.x + textoffsetx,
                 controlCircle.y + textoffsety, 20, BLACK);

        DrawCircle(hip->position.x,  hip->position.y, 10.0f, RED);

        float theta2Rad = PI- acosf(
            Clamp((L1 * L1 + L2*L2 - r*r) / (2.0f * L1 * L2), -1.0f, 1.0f));

        float phiRad = atan2f( localTargetPos.y, localTargetPos.x);

        float psiRad = acos(Clamp((r*r + L1*L1 - L2 * L2) / (2.0f    * L1 * r), -1.0f, 1.0f));

        float theta1Rad = phiRad - psiRad;

        auto rotatedTheta1 = rotate_vector2(Vector2UnitX, theta1Rad);

        auto rotatedTheta2 = rotate_vector2(Vector2UnitX, theta1Rad + theta2Rad);


        Vector2 thighEndPos = Vector2{hip->position.x, hip->position.y} + rotatedTheta1 * L1;
        DrawLineEx(hip->position, thighEndPos, 5.0f, BLACK);

        DrawLineEx(thighEndPos, thighEndPos + Vector2Scale( rotatedTheta2,L2),
                 5.0f, BLACK);


        EndDrawing();
    }
    CloseWindow();

    return 0;
}

Vector2 rotate_vector2(const Vector2 vector, const float angle)
{
    float radAngle = angle;
    const float x = cos(radAngle) * vector.x - vector.y * sin(radAngle);
    const float y = sin(radAngle) * vector.x + vector.y * cos(radAngle);

    return Vector2{x, y};
}
