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
  Collectable(const Collectable& c);
  ~Collectable(){ delete sprite; delete light; }

  void update(Uint8 t);
  void draw() const;
  void collect(Player* p);
  void softDelete();
  void setTo(bool c, bool d, Player* p);

  void setPosition(const Vector2f& v);
  const Vector2f& getPosition() const { return sprite->getPosition(); }
  MultiSprite* getSprite() { return sprite; }
  Light* getLight() { return light; }
  Collectable& operator=(const Collectable& rhs);
  bool operator==(const Collectable& rhs);
private:
  MultiSprite* sprite;
  bool collected;
  bool deleted;
  Player* player;
  Light* light;
};
#endif
