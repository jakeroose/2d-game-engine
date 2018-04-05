#ifndef SMARTSPRITE__H
#define SMARTSPRITE__H
#include <string>
#include "twowaymultisprite.h"
#include "collisionStrategy.h"

class SmartSprite : public TwoWayMultiSprite {
public:
  SmartSprite(const std::string& s, const Vector2f& pos, int w, int h);
  SmartSprite(const std::string& s, const Vector2f& pos, int w, int h, int x, int y);
  SmartSprite(const SmartSprite&);
  virtual ~SmartSprite();

  virtual void update(Uint32 ticks);
  void setPlayerPos(const Vector2f& p) { playerPos = p; }
  float getDistance();
private:
  enum MODE {NORMAL, EVADE, ATTACK};
  enum DIRECTION {LEFT, RIGHT};
  RectangularCollisionStrategy* strategy;
  Wall* floor;
  Vector2f playerPos;
  int playerWidth;
  int playerHeight;
  MODE currentMode;
  DIRECTION direction;
  float safeDistance;

  void goLeft();
  void goRight();
  void goUp();
  void goDown();
};
#endif
