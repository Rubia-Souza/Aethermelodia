//
// Created by rubia on 14/06/2025.
//

#ifndef COLLECTABLE_H
#define COLLECTABLE_H
#include <string>

#include "Actor.h"


class Collectable : public Actor {
public:
    Collectable(Game* game, const std::string &texturePath);

    ~Collectable();

    void Collect();

private:
    class AABBColliderComponent* mColliderComponent;
};



#endif //COLLECTABLE_H
