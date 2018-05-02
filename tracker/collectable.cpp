#include "collectable.h"
#include "gamedata.h"
#include "levelManager.h"
#include "sound.h"

Collectable::Collectable(const std::string& name) :
  sprite(new MultiSprite(name)),
  collected(false),
  exploded(false),
  rotate(Gamedata::getInstance().getXmlBool("Collectable/rotation")),
  player(NULL),
  light(new Light(getPosition()))
  {
  }

bool Collectable::operator==(const Collectable& rhs){
  if(&rhs == this) return true;
  return getPosition() == rhs.getPosition(); // && collected == rhs.collected;
}

void Collectable::setPosition(const Vector2f& v){
  sprite->setPosition(v);
  // light position will get set in update, since it's based on the sprite
}

void Collectable::update(Uint8 ticks){
  if(doneExploding()) return;

  // light->setRenderStatus(false);

  sprite->update(ticks);
  // make sure light polygon has been calculated
  // NOTE: I think this doesn't need to get called anymore due to
  // lighting changes
  // if((light->getPolygonSize() == 0 || light->getTicks() < 100) ){//&&
  //     // LevelManager::getInstance().withinRenderDistance(getPosition())){
  //   light->update(ticks);
  // }
  light->setPosition(getPosition() + Vector2f(sprite->getScaledWidth()/2,
                                              sprite->getScaledHeight()/2));
}

void Collectable::draw() const {
  // if sprite is not exploded or is in the process of exploding
  if(exploded == false || sprite->isExploding()){
    sprite->draw();
  }
}

void Collectable::collect(Player* p){
  if(collected == false && exploded == false){
    SDLSound::getInstance()[2];
    player = p;
    player->addCollectable(this);
    collected = true;
  }
}

bool Collectable::doneExploding(){
  return exploded && (sprite->isExploding() == false);
}

void Collectable::explode(){
  exploded = true;
  sprite->explode();
  light->setRenderStatus(false);
}

void Collectable::reinitialize(){
  setTo(false, false, true, NULL);
  light->setIntensity(Gamedata::getInstance().getXmlInt("lights/alpha"));
}

// it will always have sprite and light so we don't need to set those.
void Collectable::setTo(bool c, bool e, bool l, Player* p){
  collected = c;
  exploded = e;
  player = p;
  light->setRenderStatus(l);
}
