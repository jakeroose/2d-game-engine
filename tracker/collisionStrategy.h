#ifndef COLLISIONSTRATEGY__H
#define COLLISIONSTRATEGY__H
#include <cmath>
#include "drawable.h"
#include "wall.h"

class CollisionStrategy {
public:
  virtual bool execute(const Drawable&, const Drawable&) const = 0;
  virtual void draw() const = 0;
  virtual ~CollisionStrategy() {}
};

class RectangularCollisionStrategy : public CollisionStrategy {
public:
  RectangularCollisionStrategy() {}
  ~RectangularCollisionStrategy() {}
  virtual bool execute(const Drawable&, const Drawable&) const;
  virtual void draw() const;
  bool checkWallCollision(Drawable* d, Wall* w);
  bool checkWallCollisions(Drawable* d);
  bool collisionRight(Drawable* d, Wall* w);
  bool collisionLeft(Drawable* d, Wall* w);
  bool collisionTop(Drawable* d, Wall* w);
  bool collisionBottom(Drawable* d, Wall* w);
};

class MidPointCollisionStrategy : public CollisionStrategy {
public:
  MidPointCollisionStrategy() {}
  ~MidPointCollisionStrategy() {}
  virtual bool execute(const Drawable&, const Drawable&) const;
  virtual void draw() const;
  float distance(float, float, float, float) const;
};

class PerPixelCollisionStrategy : public CollisionStrategy {
public:
  PerPixelCollisionStrategy() {}
  ~PerPixelCollisionStrategy() {}
  virtual bool execute(const Drawable&, const Drawable&) const;
  virtual void draw() const;
private:
  bool isVisible(Uint32, SDL_Surface*) const;
};
#endif
