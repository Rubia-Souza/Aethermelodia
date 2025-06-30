#pragma once
#include "DrawComponent.h"


class DrawRectangleComponent : public DrawComponent
{
public:
    // Constructor updated with gap and mirrored parameters
    DrawRectangleComponent(class Actor* owner, int width, int height,
                           const Vector2& gap = Vector2::Zero, bool mirrored = false, int drawOrder = 100);

    // Override the Draw function from the base class
    void Draw(SDL_Renderer* renderer, const Vector3& modColor) override;

    // Set the color of the rectangle
    void SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void SetColor(SDL_Color color);

    // Set the gap (offset) from the actor's position
    void SetGap(const Vector2& gap) { mGap = gap; }

    // Set whether the rectangle should be mirrored
    void SetMirrored(bool mirrored) { mIsMirrored = mirrored; }

    // Getters
    int GetWidth() const { return mWidth; }
    void SetWidth(int width) { mWidth = width; }

    int GetHeight() const { return mHeight; }
    void SetHeight(int height) { mHeight = height; }

    const Vector2& GetGap() const { return mGap; }
    bool IsMirrored() const { return mIsMirrored; }

protected:
    // Dimensions of the rectangle
    int mWidth;
    int mHeight;

    // Offset from the actor's position
    Vector2 mGap;

    // Flag to determine if the rectangle is mirrored on the X-axis
    bool mIsMirrored;

    // Color of the rectangle
    SDL_Color mColor;
};
