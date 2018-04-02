#include "collectable.h"

Collectable::Collectable(const std::string& name) :
  sprite(new MultiSprite(name)),
  collected(false),
  player(NULL),
  light(new Light(getPosition()))
  {
  }
Collectable::Collectable(const Collectable& c) :
  sprite(c.sprite),
  collected(c.collected),
  player(c.player),
  light(c.light)
  {}

Collectable& Collectable::operator=(const Collectable& rhs){
  collected = rhs.collected;
  player = rhs.player;
  return *this;
}

void Collectable::update(){
  if(collected && player->getState() != PlayerState::idle){
    // will probably need to be handled by player so that they circle around
    // the player
    light->setPosition(player->getPosition() + Vector2f(5, 5));
    light->update();
    sprite->setPosition(light->getPosition());
  } else {
    light->setPosition(getPosition());
    // make sure light has been calculated
    if(light->getPolygonSize() == 0) light->update();
  }
}

void Collectable::draw() const {
  if(collected == false){
    sprite->draw();
  }
}


void Collectable::collect(Player* p){
  if(collected == false){
    player = p;
    player->addCollectable();
    player->addLight(light);
    collected = true;
  }
}
