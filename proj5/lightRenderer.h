#ifndef LIGHTRENDERER__H
#define LIGHTRENDERER__H

#include "light.h"
#include "intersection.h"
class LightRenderer {
public:
  LightRenderer();
  ~LightRenderer(){}
  static LightRenderer& getInstance();

  void draw() const;
  void addLight(Light* l){ lights.push_back(l); }
  void toggleDebug() { debug = !debug; }

  LightRenderer(const LightRenderer& l) = delete;
  const LightRenderer& operator=(const LightRenderer& l) = delete;
private:
  SDL_Renderer* const renderer;
  std::vector<Light*> lights;

  bool debug;
  bool renderLights;
};
#endif
