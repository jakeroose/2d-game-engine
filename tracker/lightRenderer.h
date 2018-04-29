#ifndef LIGHTRENDERER__H
#define LIGHTRENDERER__H

#include "light.h"
#include "intersection.h"
class LightRenderer {
public:
  static LightRenderer& getInstance();

  void draw() const;
  void addLight(Light* l){ lights.push_back(l); }
  void toggleDebug() { debug = !debug; }
  const std::vector<Light*>& getLights() { return lights; }

  void calculateBorder(Light* l) const;
private:
  SDL_Renderer* const renderer;
  std::vector<Light*> lights;
  bool debug;
  bool renderLights;
  bool diffusion;
  int diffusionRadius;
  SDL_Color baseColor;

  float calcIntensity(float d) const;
  LightRenderer();
  ~LightRenderer(){}
  LightRenderer(const LightRenderer& l) = delete;
  const LightRenderer& operator=(const LightRenderer& l) = delete;
};
#endif
