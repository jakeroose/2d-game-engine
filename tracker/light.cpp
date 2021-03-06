#include <math.h>
#include <iostream>
#include <algorithm>
#include <list>
#include <stdlib.h>
#include <pthread.h>
#include <thread>
#include <mutex>

#include "renderContext.h"
#include "light.h"
#include "engine.h"
#include "viewport.h"
#include "gamedata.h"
#include "wall.h"
#include "ioMod.h"
#include "levelManager.h"
#include "lightRenderer.h"

// TODO: add 'chunk' based loading for wall detection. i.e. load walls up to
// 2 units away, don't unload until they are 4 units away since player will
// likely move back and forth a bit causing walls to be loaded and unloaded
// constantly with the current method

// fill polygon implementation
// http://alienryderflex.com/polygon/
// draw circle (not implemented)
// https://stackoverflow.com/questions/41524497/c-sdl2-rendering-a-circle?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
// inspiration for lighting
// http://ncase.me/sight-and-light/

Light::Light(const Vector2f& p) :
  position(p),
  rc(RenderContext::getInstance()),
  renderer(rc->getRenderer()),
  lightPolygon(std::vector<Intersection*>()),
  lightPolygonBorder(),
  intersectionPool(),
  minx(0),miny(0),maxx(0),maxy(0),
  intensity(Gamedata::getInstance().getXmlInt("lights/alpha")),
  baseIntensity(intensity),
  renderStatus(true),
  isStatic(true),
  shouldUpdate(true),
  totalTicks(0)
  {
    lightPolygon.reserve(256);
    LightRenderer::getInstance().addLight(this);
}

Light::~Light(){
  // ensures no double frees, but that shouldn't happen anyways if I do
  // everything correctly :D
  for(Intersection* i: lightPolygon) intersectionPool.push_back(i);
  intersectionPool.sort();
  intersectionPool.unique();
  for(Intersection* i: intersectionPool) delete i;
}

void Light::setPosition(const Vector2f& pos){
  shouldUpdate = (renderStatus && pos != position);
  position = pos;
  if(shouldUpdate) update();
}

/* Returns the point of intersection between ray(r1, r2) and the line(s1, s2)
*/
Intersection* Light::getIntersection(Vector2f r1, Vector2f r2, Vector2f s1, Vector2f s2){
	// RAY in parametric: Point + Direction*T1
	float r_px = r1[0];
	float r_py = r1[1];
	float r_dx = r2[0]-r_px;
	float r_dy = r2[1]-r_py;

	// SEGMENT in parametric: Point + Direction*T2
	float s_px = s1[0];
	float s_py = s1[1];
	float s_dx = s2[0]-s_px;
	float s_dy = s2[1]-s_py;

	// Are they parallel? If so, no intersect
	float r_mag = sqrt(r_dx*r_dx+r_dy*r_dy);
	float s_mag = sqrt(s_dx*s_dx+s_dy*s_dy);
	if(r_dx/r_mag==s_dx/s_mag && r_dy/r_mag==s_dy/s_mag){ // Directions are the same.
		return NULL;
	}

	// SOLVE FOR T1 & T2
	// r_px+r_dx*T1 = s_px+s_dx*T2 && r_py+r_dy*T1 = s_py+s_dy*T2
	// ==> T1 = (s_px+s_dx*T2-r_px)/r_dx = (s_py+s_dy*T2-r_py)/r_dy
	// ==> s_px*r_dy + s_dx*T2*r_dy - r_px*r_dy = s_py*r_dx + s_dy*T2*r_dx - r_py*r_dx
	// ==> T2 = (r_dx*(s_py-r_py) + r_dy*(r_px-s_px))/(s_dx*r_dy - s_dy*r_dx)
	float T2 = (r_dx*(s_py-r_py) + r_dy*(r_px-s_px))/(s_dx*r_dy - s_dy*r_dx);
	float T1 = (s_px+s_dx*T2-r_px)/r_dx;

	// Must be within parametric whatevers for RAY/SEGMENT
	if(T1<0) return NULL;
	if(T2<0 || T2>1) return NULL;

	// Return the POINT OF INTERSECTION
  if(intersectionPool.empty() == false){
    Intersection* i = intersectionPool.front();
    intersectionPool.pop_front();
    i->x = r_px+r_dx*T1;
    i->y = r_py+r_dy*T1;
    i->param = T1;
    return i;
  } else {
    return new Intersection(r_px+r_dx*T1, r_py+r_dy*T1, T1);
  }
}

/* returns the closest intersection of ray and all of our shapes
*/
Intersection* Light::getSegmentIntersections(std::vector<Vector2f> ray){
  // Find CLOSEST intersection
	Intersection* closestIntersect = NULL;
  Vector2f seg1, seg2;
  std::map<std::string, std::vector<Vector2f> > v;
  if(isStatic == false){
    v = LevelManager::getInstance().getVerticesInView(position);
  } else {
    // TODO: add 'chunk' based loading for wall detection.
    v = LevelManager::getInstance().getWallVertices();
  }

  for(auto it = v.begin();
      it != v.end(); it++){
    int j = it->second.size()-1;

    for(unsigned i = 0; i < it->second.size(); i++){

      // get intersection of ray (coord1, coord2) and the line
      // segment (coordi, coordj) from our shape vector<coords_of_shape>
      Intersection* intersect = getIntersection(ray[0], ray[1],
                                                it->second[j], it->second[i]);
      if(intersect &&
        (!closestIntersect || intersect->param < closestIntersect->param)){
        // recycle old closestIntersect;
        if(closestIntersect) intersectionPool.push_back(closestIntersect);
  			closestIntersect = intersect;
      } else {
        // recycle intersect;
        if(intersect) intersectionPool.push_back(intersect);
      }
      j = i;
    }
  }
  return closestIntersect;
}

/* Makes sure intersection is within the game */
bool validIntersect(Intersection* i){
  if(i && i->x >= -5 && i->y >= -5 &&
    i->x <= (Viewport::getInstance().getWorldWidth() +10) &&
    i->y <= (Viewport::getInstance().getWorldHeight() + 10)){
      return true;
    }
  return false;
}

/* Checks the Intersection object pool for a free Intersection. If none
   available it creates a new one.
*/
Intersection* Light::getFreeIntersection(float x, float y, float p, float a){
  if(intersectionPool.empty() == false){
    Intersection* i = intersectionPool.front();
    intersectionPool.pop_front();
    i->x = x;
    i->y = y;
    i->param = p;
    i->angle = a;
    return i;
  } else {
    return new Intersection(x, y, p, a);
  }
}

int roundInt(int i){
  int r = i % LevelManager::UNIT_SIZE;
  if(r < LevelManager::UNIT_SIZE/2){
    return i - r;
  } else {
    return i + (LevelManager::UNIT_SIZE - r);
  }
}
bool sameLine(Intersection* i1, Intersection* i2){
  // return !(static_cast<int>(i1.x) == static_cast<int>(i2.x)) !=
    //  !(static_cast<int>(i1.y) == static_cast<int>(i2.y));
  // return static_cast<int>(i1->x) == static_cast<int>(i2->x);
  return abs(i1->x - i2->x) < 1.5;
}

/* cleanPolygon is used to remove duplicate points on the same line of
   our lightPolygon to reduce its complexity
   It works by checking for 3 points in a row with the same x/y and removing
   the middle point.
*/
void Light::cleanPolygon(){
  int i = 0, j = lightPolygon.size() - 1;
  std::vector<Intersection*> newPoly = std::vector<Intersection*>();
  newPoly.reserve(lightPolygon.size()/2);

  while(i < (int)lightPolygon.size()){
    // always add j to the new list
    newPoly.push_back(getFreeIntersection(lightPolygon[j]->x, lightPolygon[j]->y, lightPolygon[j]->param, lightPolygon[j]->angle));

    if(sameLine(lightPolygon[j], lightPolygon[i])){
      while(i < (int)lightPolygon.size() &&
       sameLine(lightPolygon[(i+1)%lightPolygon.size()], lightPolygon[i]) &&
       sameLine(lightPolygon[j], lightPolygon[i])){
        i++;
      }
      j = i++;
    } else {
      j = (j+1)%lightPolygon.size();
      i++;
    }
  }

  // recycle old lightPolygon
  for(Intersection* i : lightPolygon) intersectionPool.push_back(i);

  lightPolygon = newPoly;
  cleanPolygonX();
}

bool sameLine2(Intersection* i1, Intersection* i2){
  return abs(i1->y - i2->y) < 1.5; // check if pixels are
}

void Light::cleanPolygonX(){
  int i = 0, j = lightPolygon.size() - 1;
  std::vector<Intersection*> newPoly = std::vector<Intersection*>();
  while(i < (int)lightPolygon.size()){
    newPoly.push_back(getFreeIntersection(lightPolygon[j]->x,
       lightPolygon[j]->y, lightPolygon[j]->param, lightPolygon[j]->angle));
    updateMinMaxCoords(lightPolygon[j]);
    // if((int)lightPolygon[j]->y == (int)lightPolygon[i]->y){
    //   while(i < (int)lightPolygon.size() &&
    //    static_cast<int>(lightPolygon[(i+1)%lightPolygon.size()]->y) ==
    //    static_cast<int>(lightPolygon[i]->y)){
    //     i++;
    //   }
    if(sameLine2(lightPolygon[j], lightPolygon[i])){
      while(i < (int)lightPolygon.size() &&
       sameLine2(lightPolygon[(i+1)%lightPolygon.size()], lightPolygon[i]) &&
       sameLine2(lightPolygon[j], lightPolygon[i])){
      i++;
    }
    j = i++;
    } else {
      j = (j+1)%lightPolygon.size();
      i++;
    }
  }
  for(Intersection* i : lightPolygon){ intersectionPool.push_back(i); }
  lightPolygon = newPoly;
}

void Light::updateMinMaxCoords(Intersection* i){
  if(i->x < minx){ minx = i->x; }
  if(i->y < miny){ miny = i->y; }
  if(i->x > maxx){ maxx = i->x; }
  if(i->y > maxy){ maxy = i->y; }
}

/* calculates the border coords for pixelY */
void borderThreaded(Light* l, std::vector<Intersection*>& lightPolygon,
  int pixelY, std::mutex* m){
  int polyCorners, i, j;
  std::vector<int> nodeX; nodeX.reserve(32);
  polyCorners = l->getPolygonSize();

  //  Build a list of nodes.
  j=polyCorners-1;
  for (i=0; i<polyCorners; i++) {
    if ((lightPolygon[i]->y <  (double) pixelY &&
    lightPolygon[j]->y >= (double) pixelY) ||
    (lightPolygon[j]->y <  (double) pixelY &&
    lightPolygon[i]->y >= (double) pixelY)) {
      nodeX.push_back(
        (int)(lightPolygon[i]->x+(pixelY-lightPolygon[i]->y) /
        (lightPolygon[j]->y-lightPolygon[i]->y) *
        (lightPolygon[j]->x-lightPolygon[i]->x)));
    }
    j=i;
  }

  std::sort(nodeX.begin(), nodeX.end());

  // acquire lock so that two threads don't try to edit lightPolygonBorder
  // at the same time
  std::lock_guard<std::mutex> guard(*m);
  l->setLightPolygonBorder(pixelY, nodeX);
}

/* calculate all x & y coords on the border of the light polygon */
void Light::updatePolygonBorder(){
  int pixelY;
  std::mutex border_mutex;
  std::vector<std::thread> tList; tList.reserve(maxy-miny);

  //  Loop through the rows of the light polygon
  for (pixelY=0; pixelY<maxy-miny; pixelY++) {
    // call our threaded function for each row
    tList.emplace_back(std::thread(borderThreaded, this,
      std::ref(lightPolygon), (pixelY+miny), &border_mutex));
  }

  for(auto& e : tList) e.join();
}


/* updates the lightPolygon
*/
void Light::update(Uint8 ticks) {
  // don't need to update if not drawing
  if(LightRenderer::getInstance().getRenderStatus() == false &&
     LightRenderer::getInstance().getDebugStatus()  == false) return;

  totalTicks += (int)ticks;
  // reset our light polygon
  for(Intersection* e : lightPolygon){
    intersectionPool.push_back(e);
  }
  lightPolygon.clear();
  lightPolygonBorder.clear();

  minx = 999999; miny = 999999; maxx = -5; maxy = -5;

  int x = position[0];
  int y = position[1];

	// Get all angles
  std::vector<float> uniqueAngles = std::vector<float>();
  uniqueAngles.reserve(64);
  // offset to get new ray around the intersect
  float offset = 0.0001;

  for(auto e: LevelManager::getInstance().getWallVertices()){
    for(Vector2f p: e.second){
      // check if wall is within render distance of the light.
      if(LevelManager::getInstance().withinRenderDistance(p, position) == false)
        continue;

      float angle = atan2(p[1]-y, p[0]-x);

      // if angle is not in uniqueAngles then add it
      // NOTE: May be able to remove this check since we clean up the
      // polygon later.
      if(uniqueAngles.empty() || !(std::find(uniqueAngles.begin(), uniqueAngles.end(), angle) !=  uniqueAngles.end())){
        uniqueAngles.push_back(angle-offset);
        uniqueAngles.push_back(angle+offset);
        uniqueAngles.push_back(angle);
      }
    }
  }

  // light position
  std::vector<Vector2f> ray = std::vector<Vector2f>(2);
  ray[0] = Vector2f(x, y);

  // RAYS IN ALL DIRECTIONS
  for(float angle: uniqueAngles){
		// Calculate dx & dy from angle
		float dx = cos(angle);
		float dy = sin(angle);

    // ray from position -> offset from angle
    ray[1] = Vector2f(x + dx, y + dy);


    // Find CLOSEST intersection
    Intersection* closestIntersect = getSegmentIntersections(ray);

    // add to list of lightPolygon
    if(validIntersect(closestIntersect)){
      closestIntersect->angle = angle;
      lightPolygon.push_back(closestIntersect);
    } else if(closestIntersect){
      intersectionPool.push_back(closestIntersect);
    }
	}

  // sort lightPolygon by angle
  std::sort(lightPolygon.begin(), lightPolygon.end(),
    [](Intersection* a, Intersection* b){
      return a->angle < b->angle;
    }
  );

  // remove dupes
  cleanPolygon();

  updatePolygonBorder();
}
