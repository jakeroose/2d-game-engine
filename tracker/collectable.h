#ifndef COLLECTABLE__H
#define COLLECTABLE__H

#include "vector2f.h"
#include "multisprite.h"
#include "player.h"
#include "light.h"

class Player;

class Collectable {
public:
  Collectable(const std::string& name);
  Collectable(const Collectable& c) = delete;
  ~Collectable(){ delete sprite; delete light; }

  void update(Uint8 t);
  void draw() const;
  void collect(Player* p);
  void softDelete();
  void setTo(bool c, bool d, bool l, Player* p);
  void reinitialize();

  void setPosition(const Vector2f& v);
  const Vector2f& getPosition() const { return sprite->getPosition(); }
  MultiSprite* getSprite() { return sprite; }
  Light* getLight() { return light; }
  void setLightIntensity(int i) { light->setIntensity(i); }
  void explode();
  bool getExploded() { return exploded; }
  bool doneExploding();
  bool getRotate() { return rotate; }

  Collectable& operator=(const Collectable& rhs) = delete;
  bool operator==(const Collectable& rhs);
private:
  MultiSprite* sprite;
  bool collected;
  bool exploded;
  bool rotate;
  Player* player;
  Light* light;
};
#endif
