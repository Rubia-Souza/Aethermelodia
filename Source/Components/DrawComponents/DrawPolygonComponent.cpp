//
// Created by Lucas N. Ferreira on 28/09/23.
//

#include "DrawPolygonComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"

DrawPolygonComponent::DrawPolygonComponent(Actor* owner, std::vector<Vector2>& vertices, int drawOrder)
    : DrawComponent(owner, drawOrder)
    , mVertices(vertices)
    , mColor({255, 255, 255, 255})
{
}

void DrawPolygonComponent::SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    mColor.r = r;
    mColor.g = g;
    mColor.b = b;
    mColor.a = a;
}

void DrawPolygonComponent::Draw(SDL_Renderer *renderer, const Vector3 &modColor)
{
    // Set draw color to green
    SDL_SetRenderDrawColor(renderer, mColor.r, mColor.g, mColor.b, mColor.a);

    Vector2 pos = mOwner->GetPosition();
    Vector2 cameraPos = mOwner->GetGame()->GetCameraPos();

    // Render vertices as lines
    for(int i = 0; i < mVertices.size() - 1; i++) {
        SDL_RenderDrawLine(renderer, pos.x + mVertices[i].x - cameraPos.x,
                                     pos.y + mVertices[i].y - cameraPos.y,
                                     pos.x + mVertices[i+1].x - cameraPos.x,
                                     pos.y + mVertices[i+1].y - cameraPos.y);
    }

    // Close geometry
    SDL_RenderDrawLine(renderer, pos.x + mVertices[mVertices.size() - 1].x - cameraPos.x,
                                 pos.y + mVertices[mVertices.size() - 1].y - cameraPos.y,
                                 pos.x + mVertices[0].x - cameraPos.x,
                                 pos.y + mVertices[0].y - cameraPos.y);
}