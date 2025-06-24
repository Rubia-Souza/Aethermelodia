//
// Created by aline on 23/06/2025.
//
#include "ParticleEmitter.h"
#include "../Random.h"
#include "../Game.h"

ParticleEmitter::ParticleEmitter(const Vector2& origin)
    : mOrigin(origin)
{
}

// Partially doing the Sistemas de Emissores logic
void ParticleEmitter::EmitExplosion(int count)
{
    for (int i = 0; i < count; ++i)
    {
        float angle = Random::GetFloatRange(0.0f, Math::TwoPi);
        float speed = Random::GetFloatRange(100.0f, 300.0f);
        Vector2 vel = Vector2(cosf(angle), sinf(angle)) * speed;

        SDL_Color color = {0, 191, 255, 255}; // blue
        float size = Random::GetFloatRange(1.0f, 3.0f);
        float lifetime = Random::GetFloatRange(0.5f, 1.5f);

        auto* p = new Particle();
        p->Init(mOrigin, vel, lifetime, color, size);
        AddParticle(p);
    }
}

void ParticleEmitter::AddParticle(Particle* p)
{
    mParticles.emplace_back(p);
}

void ParticleEmitter::DelParticle(size_t index)
{
    delete mParticles[index];
    mParticles.erase(mParticles.begin() + index);
}

void ParticleEmitter::Update(float deltaTime)
{
    for (size_t i = 0; i < mParticles.size(); )
    {
        Particle* p = mParticles[i];
        p->Update(deltaTime);

        if (p->IsDead()) {
            DelParticle(i);
        } else {
            ++i;
        }
    }
}

void ParticleEmitter::Draw(SDL_Renderer* renderer)
{
    for (auto* p : mParticles)
    {
        p->Draw(renderer);
    }
}

bool ParticleEmitter::IsDead() const
{
    return mParticles.empty();
}
