#pragma once
#include <vector>
#include "Particle.h"

class ParticleEmitter
{
public:
    ParticleEmitter(const Vector2& origin);

    void EmitExplosion(int count);

    void Update(float deltaTime);
    void Draw(SDL_Renderer* renderer);

    void AddParticle(Particle* p);
    void DelParticle(size_t index);

    bool IsDead() const;

private:
    Vector2 mOrigin;
    std::vector<Particle*> mParticles;
};