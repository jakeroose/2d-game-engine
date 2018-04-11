#ifndef BACKGROUND__H
#define BACKGROUND__H
#include "image.h"
#include <vector>

class BackgroundSprite;

class Background {
public:
  Background();
  Background(const std::string& name);
  Background(const Background& b) = delete;
  Background& operator=(const Background& rhs) = delete;
  ~Background();
  void initialize();
  void draw() const;
  void update(Uint32 t);
private:
  Image* image;
  std::vector<BackgroundSprite*> images;
  int ticks;
  std::string name;
};
#endif
