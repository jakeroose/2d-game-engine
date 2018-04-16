#ifndef CHUNK__H
#define CHUNK__H

#include <iostream>
#include "sprite.h"
#include "gamedata.h"

class Chunk : public Sprite {
public:
  explicit Chunk( const Vector2f& pos, const Vector2f vel,
                  const string& name, Image* fm);

  Chunk(const Chunk& )=default;
  Chunk(      Chunk&&)=default;
  Chunk& operator=(const Chunk& )=default;
  Chunk& operator=(      Chunk&&)=default;

  virtual ~Chunk(){
    delete image;
  }
  virtual void update(Uint32 ticks);
  bool goneTooFar() const { return tooFar; }
  void reset() {
    tooFar = false;
    distance = 0;
  }
private:
  float distance;
  float maxDistance;
  float variation;
  float lifespan;
  float age;
  bool tooFar;
  Image* image;
};
#endif
