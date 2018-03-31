#ifndef COLLECTABLE__H
#define COLLECTABLE__H

#include "vector2f.h"
#include "multisprite.h"
#include "player.h"

class Collectable {
public:
  Collectable(const std::string& name);
  Collectable(const Collectable& c);
  ~Collectable(){ delete sprite;}

  void update();
  void draw() const;
  void collect(Player* p);

  void setPosition(const Vector2f& v){ sprite->setPosition(v); }
  MultiSprite* getSprite() { return sprite; }
  Collectable& operator=(const Collectable& rhs);
private:
  MultiSprite* sprite;
  bool collected;
  Player* player;
};
#endif
