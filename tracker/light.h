#ifndef LIGHT__H
#define LIGHT__H

#include "vector2f.h"
#include "renderContext.h"
#include "wall.h"


class Intersection;

class Light {
public:
  Light(const Vector2f& p);
  Light(const Light&) = delete;
  ~Light();
  void draw();
  void update();
  const Light& operator=(const Light& l) {
     rc = l.rc; return *this;
  }
  void toggleDebug(){ debug = !debug; }
  void setPosition(const Vector2f& pos){ position = pos; }
  void cleanPolygon();
  void cleanPolygonX();
private:
  Vector2f position;
  const RenderContext* rc;
  SDL_Renderer* const renderer;
  std::vector<Intersection> lightPolygon;
  bool debug;
  bool renderLights;

  /* methods */
  Intersection* getIntersection(Vector2f r1, Vector2f r2, Vector2f s1, Vector2f s2);
  Intersection* getSegmentIntersections(std::vector<Vector2f> ray);
  std::vector<Intersection>* getAllPoints();
};

#endif
