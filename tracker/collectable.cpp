#include "collectable.h"

Collectable::Collectable(const std::string& name) :
  sprite(new MultiSprite(name)),
  collected(false),
  player(NULL)
  {

  }
Collectable::Collectable(const Collectable& c) :
  sprite(c.sprite),
  collected(c.collected),
  player(c.player){}

Collectable& Collectable::operator=(const Collectable& rhs){
  collected = rhs.collected;
  player = rhs.player;
  return *this;
}

void Collectable::update(){

}

void Collectable::draw() const {
  if(collected == false){
    sprite->draw();
  }
}


void Collectable::collect(Player* p){
  player = p;
  collected = true;
}
