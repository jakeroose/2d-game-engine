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
  std::vector<SDL_Rect>* rects;
  void decompose(SDL_Rect r);
  void decompose(const std::vector<Vector2f>& s);
  void decomposeWalls();
  void toggleDebug(){ debug = !debug; }
  void setPosition(const Vector2f& pos){ position = pos; }
  void addWall(Wall* w){walls.emplace(walls.size(), w);}
private:
  Vector2f position;
  const RenderContext* rc;
  SDL_Renderer* const renderer;
  std::map<int, std::vector<Vector2f> >* segments;
  std::map<int, Wall* > walls;
  std::vector<Intersection> lightPolygon;
  bool debug;
  bool renderLights;

  /* methods */
  Intersection* getIntersection(Vector2f r1, Vector2f r2, Vector2f s1, Vector2f s2);
  Intersection* getSegmentIntersections(std::vector<Vector2f> ray);
  std::vector<Intersection>* getAllPoints();
};

#endif
