#ifndef LIGHT__H
#define LIGHT__H
#include <list>
#include "vector2f.h"
#include "renderContext.h"
#include "wall.h"
#include "intersection.h"

class Light {
public:
  Light(const Vector2f& p);
  Light(const Light&) = delete;
  ~Light();
  void draw() const;
  void update(Uint8 ticks=0);
  const Light& operator=(const Light& l) {
     rc = l.rc; return *this;
  }
  void setPosition(const Vector2f& pos);
  const Vector2f& getPosition(){ return position; }
  void cleanPolygon();
  void cleanPolygonX();
  int  getPolygonSize() const { return (int)lightPolygon.size();}
  std::vector<Intersection*>& getPolygon() { return lightPolygon; }
  void updateMinMaxCoords(Intersection* i);
  int getMinx(){ return minx;}
  int getMiny(){ return miny;}
  int getMaxx(){ return maxx;}
  int getMaxy(){ return maxy;}
  void setIntensity(int i) { intensity = i; }
  int getIntensity() const { return intensity; }
  int getBaseIntensity() const { return baseIntensity; }
  bool shouldDraw() const { return renderStatus; }
  void setRenderStatus(bool s) { renderStatus = s; }
  bool getRenderStatus() const { return renderStatus; }
  void setStatic(bool s) { isStatic = s; }

  int getIntersectionPoolSize() const { return (int)intersectionPool.size(); }
  int getTicks() const { return totalTicks; }

  const std::map<int, std::vector<int> >& getLightPolygonBorder() {
    return lightPolygonBorder;
  }
  std::vector<int>& getLightPolygonBorder(int i) {
    // return (*(lightPolygonBorder.find(i))).second;
    return lightPolygonBorder[i];
  }
  void setLightPolygonBorder(int i, std::vector<int> v) {
    lightPolygonBorder[i] = v;
  }
  void updatePolygonBorder();

private:
  Vector2f position;
  const RenderContext* rc;
  SDL_Renderer* const renderer;
  std::vector<Intersection*> lightPolygon;
  std::map<int, std::vector<int> > lightPolygonBorder;
  std::list<Intersection*> intersectionPool;
  int minx, miny, maxx, maxy;
  int intensity, baseIntensity;
  bool renderStatus;
  bool isStatic;
  bool shouldUpdate;
  int totalTicks;

  /* methods */
  Intersection* getIntersection(Vector2f r1, Vector2f r2, Vector2f s1, Vector2f s2);
  Intersection* getSegmentIntersections(std::vector<Vector2f> ray);
  std::vector<Intersection>* getAllPoints();
  Intersection* getFreeIntersection(float x, float y, float p, float a);
};

#endif
