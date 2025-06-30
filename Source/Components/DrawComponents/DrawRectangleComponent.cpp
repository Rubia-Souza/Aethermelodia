#include "DrawRectangleComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"


DrawRectangleComponent::DrawRectangleComponent(Actor* owner, int width, int height,
                                                 const Vector2& gap, bool mirrored, int drawOrder)
    : DrawComponent(owner, drawOrder)
    , mWidth(width)
    , mHeight(height)
    , mGap(gap)
    , mIsMirrored(mirrored)
    , mColor({255, 255, 255, 255}) // Default color is white
{
}

void DrawRectangleComponent::SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    mColor.r = r;
    mColor.g = g;
    mColor.b = b;
    mColor.a = a;
}

void DrawRectangleComponent::SetColor(SDL_Color color)
{
    mColor = color;
}


void DrawRectangleComponent::Draw(SDL_Renderer* renderer, const Vector3& modColor)
{
    // Set the draw color to the component's color
    SDL_SetRenderDrawColor(renderer, mColor.r, mColor.g, mColor.b, mColor.a);

    // Get the owner's position and the camera's position
    Vector2 ownerPos = mOwner->GetPosition();
    Vector2 cameraPos = mOwner->GetGame()->GetCameraPos();

    // Create an SDL_Rect to define the rectangle's position and dimensions
    SDL_Rect rect;

    // Calculate the base X position including the gap and camera offset
    float finalX = ownerPos.x + mGap.x - cameraPos.x;

    // // If mirrored, adjust the X position to flip it horizontally.
    // // This moves the drawing starting point to the left by the rectangle's width
    // // and the gap, effectively mirroring it across the actor's origin.
    // if (mIsMirrored)
    // {
    //     finalX -= (mWidth + (mGap.x * 2));
    // }

    // The rectangle's top-left corner is the actor's position, adjusted for gap and camera.
    // We cast to int because SDL_Rect uses integers.
    rect.x = static_cast<int>(finalX);
    rect.y = static_cast<int>(ownerPos.y + mGap.y - cameraPos.y);
    rect.w = mIsMirrored ?  -mWidth : mWidth;
    rect.h = mHeight;

    // Draw the filled rectangle
    SDL_RenderFillRect(renderer, &rect);
}
