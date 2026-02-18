#include <C:/raylib/raylib/src/raylib.h>
#include <C:/raylib/raylib/src/raymath.h>
#include <vector>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "TwoBoneIK2D.h"

static constexpr float k_text_offset_x = 30.0f;
static constexpr float k_text_offset_y = -10.0f;

static ik2d::Vec2 ToIKVec2(const Vector2 v) { return ik2d::Vec2{v.x, v.y}; }
static Vector2 ToRayVec2(const ik2d::Vec2 v) { return Vector2{v.x, v.y}; }

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
        const ik2d::TwoBoneResult res =
            ik2d::TwoBoneIK2D::Solve(ToIKVec2(root), L1, L2, ToIKVec2(target), bendDir, autoBend);

        knee = ToRayVec2(res.knee);
        end = ToRayVec2(res.end);
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
