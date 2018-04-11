#ifndef BACKGROUND__H
#define BACKGROUND__H
#include "image.h"
#include <vector>

class BackgroundSprite;

class Background {
public:
  Background() = delete;
  Background(int c);
  Background(const Background& b) = delete;
  Background& operator=(const Background& rhs) = delete;
  ~Background();
  void initialize();
  void draw() const;
  void update(Uint32 t);
private:
  std::vector<BackgroundSprite*> images;
  int ticks, objectCount;
};
#endif
