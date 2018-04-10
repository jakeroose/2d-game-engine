#include "collectable.h"

Collectable::Collectable(const std::string& name) :
  sprite(new MultiSprite(name)),
  collected(false),
  deleted(false),
  player(NULL),
  light(new Light(getPosition()))
  {
  }
Collectable::Collectable(const Collectable& c) :
  sprite(c.sprite),
  collected(c.collected),
  deleted(c.deleted),
  player(c.player),
  light(c.light)
  {}

Collectable& Collectable::operator=(const Collectable& rhs){
  collected = rhs.collected;
  player = rhs.player;
  deleted = rhs.deleted;
  return *this;
}

bool Collectable::operator==(const Collectable& rhs){
  return getPosition() == rhs.getPosition();// && collected == rhs.collected;
}

void Collectable::setPosition(const Vector2f& v){
  sprite->setPosition(v);
  light->setPosition(v);
}


void Collectable::update(){
  if(deleted){
    return;
  }
  if(collected){
    light->update();
  } else {
    light->setPosition(getPosition());
    // make sure light has been calculated
    if(light->getPolygonSize() == 0) light->update();
  }
}

void Collectable::draw() const {
  if(deleted == false){
    sprite->draw();
  }
}


void Collectable::collect(Player* p){
  if(collected == false){
    player = p;
    player->addCollectable(this);
    player->addLight(light);
    collected = true;
  }
}

void Collectable::softDelete(){
  deleted = true;
  light->setRenderStatus(false);
}
