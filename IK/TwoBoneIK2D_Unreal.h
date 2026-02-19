#pragma once

#include "CoreMinimal.h"

namespace ik2d
{
struct TwoBoneResultUE
{
    FVector2D Knee{0.0f, 0.0f};
    FVector2D End{0.0f, 0.0f};

    // Angles are in radians and consistent with the input coordinate system.
    float Phi = 0.0f;    // direction root -> target
    float Psi = 0.0f;    // hip offset angle
    float Gamma = 0.0f;  // internal knee angle
    float Theta1 = 0.0f; // first segment absolute angle
    float Theta2 = 0.0f; // second segment relative angle (added to Theta1)

    float Radius = 0.0f;     // clamped |target-root|
    int8 Bend = 0;           // +1 or -1 solution used
    bool bWasClamped = false; // whether Radius had to be clamped to reachable range
};

class TwoBoneIK2D_UE
{
public:
    // Solves a 2D two-segment IK chain.
    // - BendDir: +1 or -1, selects the mirrored knee solution when bAutoBend=false.
    // - bAutoBend: when true, BendDir is picked from the sign of (Target-Root).X.
    static FORCEINLINE TwoBoneResultUE Solve(const FVector2D& Root,
                                            const float L1,
                                            const float L2,
                                            const FVector2D& Target,
                                            const int32 BendDir = 1,
                                            const bool bAutoBend = false)
    {
        TwoBoneResultUE Out{};

        if (L1 <= SMALL_NUMBER || L2 <= SMALL_NUMBER)
        {
            Out.Knee = Root;
            Out.End = Root;
            Out.Radius = 0.0f;
            Out.Bend = static_cast<int8>(BendDir >= 0 ? 1 : -1);
            Out.bWasClamped = true;
            return Out;
        }

        const FVector2D LocalTarget = Target - Root;

        const float MaxReach = L1 + L2;
        const float MinReach = FMath::Abs(L1 - L2);

        const float RawRadius = LocalTarget.Size();
        const float ClampedRadius =
            FMath::Clamp(RawRadius, FMath::Max(0.001f, MinReach), MaxReach);

        const int8 Heading = static_cast<int8>(
            bAutoBend ? (LocalTarget.X >= 0.0f ? 1 : -1) : (BendDir >= 0 ? 1 : -1));

        Out.Bend = Heading;
        Out.Radius = ClampedRadius;
        Out.bWasClamped = !FMath::IsNearlyEqual(ClampedRadius, RawRadius, 1e-6f);

        Out.Phi = FMath::Atan2(LocalTarget.Y, LocalTarget.X);

        // Angle between L1 and r (root to target).
        const float CosPsi = (ClampedRadius * ClampedRadius + L1 * L1 - L2 * L2) /
                             (2.0f * L1 * ClampedRadius);
        Out.Psi = FMath::Acos(FMath::Clamp(CosPsi, -1.0f, 1.0f));

        // Internal knee angle between L1 and L2.
        const float CosGamma =
            (L1 * L1 + L2 * L2 - ClampedRadius * ClampedRadius) / (2.0f * L1 * L2);
        Out.Gamma = FMath::Acos(FMath::Clamp(CosGamma, -1.0f, 1.0f));

        Out.Theta1 = Out.Phi - static_cast<float>(Heading) * Out.Psi;
        Out.Theta2 = static_cast<float>(Heading) * (PI - Out.Gamma);

        const float C1 = FMath::Cos(Out.Theta1);
        const float S1 = FMath::Sin(Out.Theta1);
        const float C2 = FMath::Cos(Out.Theta1 + Out.Theta2);
        const float S2 = FMath::Sin(Out.Theta1 + Out.Theta2);

        Out.Knee = Root + FVector2D(C1, S1) * L1;
        Out.End = Out.Knee + FVector2D(C2, S2) * L2;

        return Out;
    }
};
} // namespace ik2d
