#include <C:/raylib/raylib/src/raylib.h>
#include <C:/raylib/raylib/src/raymath.h>
#include <vector>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

static constexpr float k_text_offset_x = 30.0f;
static constexpr float k_text_offset_y = -10.0f;

static Vector2 RotateVector2(const Vector2 vector, const float angle)
{
    const float x = cosf(angle) * vector.x - vector.y * sinf(angle);
    const float y = sinf(angle) * vector.x + vector.y * cosf(angle);
    return Vector2{x, y};
}

static Color GetNextColor(const int index)
{
    static const Color palette[] = {
        BLACK,
        ORANGE,
        YELLOW,
        PURPLE,
        GRAY,
        BLUE
    };

    return palette[index % std::size(palette)];
}

struct Chain
{
    const float target_radius = 10.0f;
    const float line_thickness = 5.0f;
    const float joint_radius = 8.0f;

    Vector2 root{};
    //Thigh length
    float L1 = 100.0f;
    //Shin length
    float L2 = 100.0f;

    Color chainColor = BLACK;
    Color targetColor = GREEN;
    Color rootColor = RED;

    //IK target pos
    Vector2 target{};

    // Whether to mirror the knee bend direction based on the X position of the IK target. 
    // Upward bend by default 
    int bendDir = 1;

    Vector2 knee{};
    Vector2 end{};

    void Solve(bool autoBend)
    {
        const Vector2 local_target = target - root;

        const float max_reach = L1 + L2;
        const float min_reach = fabsf(L1 - L2);

        // CLamp IK target at max distance (thigh + shin)
        const float radius = Clamp(Vector2Length(local_target), fmaxf(0.001f, min_reach), max_reach);

        const int heading =
            autoBend ? (local_target.x >= 0.0f ? 1 : -1) : ((bendDir >= 0) ? 1 : -1);

        const float phiRad = atan2f(local_target.y, local_target.x);

        // Angle between L1 and r (hip to IK target).
        const float cosPsi =
            (radius * radius + L1 * L1 - L2 * L2) / (2.0f * L1 * radius);
        const float psiRad = acosf(Clamp(cosPsi, -1.0f, 1.0f));

        // Internal knee angle between L1 and L2.
        const float cos_knee =
            (L1 * L1 + L2 * L2 - radius * radius) / (2.0f * L1 * L2);
        const float kneeInternalRad = acosf(Clamp(cos_knee, -1.0f, 1.0f));

        const float theta1Rad = phiRad - heading * psiRad;
        const float theta2Rad = heading * (PI - kneeInternalRad);

        const Vector2 rotatedThigh = RotateVector2(Vector2UnitX, theta1Rad);
        const Vector2 rotatedShin = RotateVector2(Vector2UnitX, theta1Rad + theta2Rad);

        knee = root + rotatedThigh * L1;
        end = knee + rotatedShin * L2;
    }

    bool CheckIKTargetPressed(const Vector2 point, const float hitRadius = 20.0f) const
    {
        return CheckCollisionPointCircle(point, target, hitRadius);
    }

    void Draw(const bool drawDebug = false) const
    {
        DrawCircleV(target, target_radius, targetColor);
        DrawCircleV(root, joint_radius, rootColor);
        DrawCircleV(knee, joint_radius, chainColor);
        DrawCircleV(end, joint_radius * 0.75f, chainColor);

        DrawLineEx(root, knee, line_thickness, chainColor);
        DrawLineEx(knee, end, line_thickness, chainColor);

        if (drawDebug)
        {
            DrawLineV(root, target, Fade(targetColor, 0.35f));
            DrawText(TextFormat("L1=%.0f L2=%.0f", L1, L2),
                     static_cast<int>(target.x + k_text_offset_x),
                     static_cast<int>(target.y + k_text_offset_y),
                     18,
                     DARKGRAY);
        }
    }
};

void CreateNewChain(std::vector<Chain>* chains, const Vector2 mouse)
{
    const int idx = static_cast<int>(chains->size());
    Chain chain{};
    chain.root = mouse;
    chain.L1 = 100.0f;
    chain.L2 = 100.0f;
    chain.target = mouse + Vector2{150.0f, 0.0f};
    chain.chainColor = GetNextColor(idx);
    chains->push_back(chain);
}

bool noMirrorBend;

int main()
{
    InitWindow(800, 450, "2D Multi-Chain IK");
    SetTargetFPS(60);

    std::vector<Chain> chains = std::vector<Chain>(5);

    int activeDrag = -1;
    Vector2 dragOffset{0.0f, 0.0f};

    while (!WindowShouldClose())
    {
        const Vector2 mouse = GetMousePosition();


        if (IsKeyPressed(KEY_N))
        {
            CreateNewChain(&chains, mouse);
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !chains.empty())
        {
            chains.pop_back();
            activeDrag = -1;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            activeDrag = -1;
            for (int i = static_cast<int>(chains.size()) - 1; i >= 0; --i)
            {
                if (chains[i].CheckIKTargetPressed(mouse))
                {
                    activeDrag = i;
                    dragOffset = mouse - chains[i].target;
                    break;
                }
            }
        }

        if (activeDrag >= 0)
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                chains[activeDrag].target = mouse - dragOffset;
            else
                activeDrag = -1;
        }

        for (Chain& chain : chains)
            chain.Solve(noMirrorBend);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int i = 0; i < static_cast<int>(chains.size()); ++i)
        {
            const bool debug = (i == activeDrag);
            chains[i].Draw(debug);
        }

        DrawText("N: new chain", 10, 10, 18, DARKGRAY);
        DrawText("Bksp: delete last chain", 10, 50, 18, DARKGRAY);

        if (GuiCheckBox(Rectangle{10, 75, 40, 40}, "MIRROR", &noMirrorBend))
        {
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
