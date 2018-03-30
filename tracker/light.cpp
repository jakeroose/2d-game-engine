#include <math.h>
#include <iostream>
#include <algorithm>

#include "renderContext.h"
#include "light.h"
#include "engine.h"
#include "viewport.h"
#include "gamedata.h"
#include "wall.h"
#include "ioMod.h"
#include "levelManager.h"

// TODO:
// - object pooling for lightPolygon

// fill polygon implementation
// http://alienryderflex.com/polygon/
// draw circle (not implemented)
// https://stackoverflow.com/questions/41524497/c-sdl2-rendering-a-circle?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
// inspiration for lighting
// http://ncase.me/sight-and-light/

class Intersection{
public:
  Intersection(float _x=0, float _y=0, float p=0, float a=0) : x(_x), y(_y), param(p), angle(a){}
  float x;
  float y;
  float param;
  float angle;

  // sort based on angle!
  bool operator< (const Intersection& i){
    return (angle < i.angle);
  }
  bool operator==(const Intersection& i){
    return (angle == i.angle && x == i.x && y == i.y );
  }
  const Intersection& operator=(const Intersection& rhs){
    x = rhs.x; y = rhs.y; param = rhs.param; angle = rhs.angle;
    return *this;
  }
};

std::ostream& operator<<(std::ostream& out, const Intersection& i){
  return out << i.x << ", " << i.y << ", " << i.angle;
}


/* =============================== */
/* ==== START OF LIGHT CLASS  ==== */
/* =============================== */
Light::Light(const Vector2f& p) :
  position(p),
  rc(RenderContext::getInstance()),
  renderer(rc->getRenderer()),
  lightPolygon(std::vector<Intersection>()),
  debug(Gamedata::getInstance().getXmlBool("lights/debug")),
  renderLights(Gamedata::getInstance().getXmlBool("lights/renderLights")){
  lightPolygon.reserve(256);
}

Light::~Light(){
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
  Intersection* i = new Intersection(r_px+r_dx*T1, r_py+r_dy*T1, T1);
  return i;
}

/* returns the closest intersection of ray and all of our shapes
*/
Intersection* Light::getSegmentIntersections(std::vector<Vector2f> ray){
  // Find CLOSEST intersection
	Intersection* closestIntersect = NULL;
  Vector2f seg1, seg2;

  for(auto it = LevelManager::getInstance().getWallVertices().begin();
      it != LevelManager::getInstance().getWallVertices().end(); it++){
    int j = it->second.size()-1;

    for(unsigned i = 0; i < it->second.size(); i++){

      // get intersection of ray (coord1, coord2) and the line
      // segment (coordi, coordj) from our shape vector<coords_of_shape>
      Intersection* intersect = getIntersection(ray[0], ray[1], it->second[j], it->second[i]);
      if(intersect && (!closestIntersect || intersect->param < closestIntersect->param)){
        delete closestIntersect;
  			closestIntersect = intersect;
      } else {
        delete intersect;
      }
      j = i;
    }
  }

  return closestIntersect;
}

/* Makes sure intersection is within the game */
bool validIntersect(Intersection* i){
  if(i != NULL && i->x >= -5 && i->y >= -5 &&
    i->x <= (Viewport::getInstance().getWorldWidth() +10) &&
    i->y <= (Viewport::getInstance().getViewHeight() + 10)){
      return true;
    }
  return false;
}

bool sameLine(Intersection& i1, Intersection& i2){
  // return !(static_cast<int>(i1.x) == static_cast<int>(i2.x)) !=
    //  !(static_cast<int>(i1.y) == static_cast<int>(i2.y));
  return static_cast<int>(i1.x) == static_cast<int>(i2.x);
}

/* cleanPolygon is used to remove duplicate points on the same line of
   our lightPolygon to reduce its complexity
   It works by checking for 3 points in a row with the same x/y and removing
   the middle point.
*/
void Light::cleanPolygon(){
  int i = 0, j = lightPolygon.size() - 1;
  std::vector<Intersection> newPoly = std::vector<Intersection>();
  newPoly.reserve(lightPolygon.size()/2);
  if(debug) std::cout << "lightPolygon vertices, before: " << j+1;
  while(i < (int)lightPolygon.size()){
    newPoly.push_back(lightPolygon[j]);
    if(sameLine(lightPolygon[j], lightPolygon[i])){
      while(i < (int)lightPolygon.size() &&
       sameLine(lightPolygon[(i+1)%lightPolygon.size()], lightPolygon[i]) &&
       sameLine(lightPolygon[j], lightPolygon[i])
    ){
        // std::cout << lightPolygon[i] << std::endl;
        // lightPolygon.erase(lightPolygon.begin() + i);
        i++;
      }
      j = i++;
    } else {
      j = (j+1)%lightPolygon.size();
      i++;
    }
  }
  lightPolygon = newPoly;
  cleanPolygonX();
}
void Light::cleanPolygonX(){
  int i = 0, j = lightPolygon.size() - 1;
  std::vector<Intersection> newPoly = std::vector<Intersection>();
  while(i < (int)lightPolygon.size()){
    newPoly.push_back(lightPolygon[j]);
    if((int)lightPolygon[j].y == (int)lightPolygon[i].y){
      while(i < (int)lightPolygon.size() &&
       static_cast<int>(lightPolygon[(i+1)%lightPolygon.size()].y) ==
       static_cast<int>(lightPolygon[i].y)){
        // lightPolygon.erase(lightPolygon.begin() + i);
        i++;
      }
      j = i++;
    } else {
      j = (j+1)%lightPolygon.size();
      i++;
    }
  }
  lightPolygon = newPoly;
  if(debug) std::cout << ", after: " << lightPolygon.size() << std::endl;
}

/* updates the lightPolygon
*/
void Light::update() {
  // reset our light polygon
  lightPolygon.clear();

  int x = position[0];
  int y = position[1];

	// Get all angles
  std::vector<float> uniqueAngles = std::vector<float>();
  uniqueAngles.reserve(64);
  // offset to get new ray around the intersect
  float offset = 0.0001;

  for(auto e: LevelManager::getInstance().getWallVertices()){
    for(Vector2f p: e.second){
      float angle = atan2(p[1]-y, p[0]-x);

      // if angle is not in uniqueAngles then add it
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
      lightPolygon.push_back(*closestIntersect);
    }
    delete closestIntersect;
	}

  // sort lightPolygon by angle
  std::sort(lightPolygon.begin(), lightPolygon.end());

  // remove dupes
  cleanPolygon();
}

/* Draws all of the cool light stuff!
*/
void Light::draw() {
  int minx = 999999, miny=999999, maxx = -1, maxy = -1,
      vx = Viewport::getInstance().getX(),
      vy = Viewport::getInstance().getY();
  /* Optimization so that we only draw what is in the viewport and within a
     rectangle surrounding the lightPolygon
     NOTE: May be able to do this when calculating the lightPolygon. Consider
     doing this if lightPolygon has consderable number of vertices
  */
  for(Intersection i: lightPolygon){
    if(i.x < minx){ minx = i.x; }
    if(i.y < miny){ miny = i.y; }
    if(i.x > maxx){ maxx = i.x; }
    if(i.y > maxy){ maxy = i.y; }
  }
  if(minx < vx){ minx = vx;}
  if(miny < vy){ miny = vy;}
  if(maxx > vx + Viewport::getInstance().getViewWidth()){ maxx = Viewport::getInstance().getViewWidth()+vx;}
  if(maxy > Viewport::getInstance().getViewHeight()+vy) {maxy = Viewport::getInstance().getViewHeight()+vy;}

  /* Render the Lighting Polygon */
  // fill algorithm courtesy of http://alienryderflex.com/polygon_fill/
  int polyCorners = lightPolygon.size();
  int nodes, pixelX, pixelY, i, j, swap, IMAGE_RIGHT = maxx, IMAGE_LEFT = minx;
  std::vector<int> nodeX;
  nodeX.reserve(1024);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor( renderer, 200, 200, 200, 255/2 );

  if(renderLights){
    //  Loop through the rows of the image.
    for (pixelY=miny; pixelY<maxy; pixelY++) {

      //  Build a list of nodes.
      nodes=0; j=polyCorners-1;
      for (i=0; i<polyCorners; i++) {
        if ((lightPolygon[i].y<(double) pixelY && lightPolygon[j].y>=(double) pixelY)
        ||  (lightPolygon[j].y<(double) pixelY && lightPolygon[i].y>=(double) pixelY)) {
          nodeX[nodes++] = (int)(lightPolygon[i].x+(pixelY-lightPolygon[i].y) /
          (lightPolygon[j].y-lightPolygon[i].y) *
          (lightPolygon[j].x-lightPolygon[i].x));
        }
        j=i;
      }

      //  Sort the nodes, via a simple “Bubble” sort.
      // TODO: don't use bubble sort...
      i=0;
      while (i<nodes-1) {
        if (nodeX[i]>nodeX[i+1]) {
          swap=nodeX[i];
          nodeX[i]=nodeX[i+1];
          nodeX[i+1]=swap;
          if (i) i--;
        }
        else { i++; }
      }

      //  Fill the pixels between node pairs.
      for (i=0; i<nodes; i+=2) {
        if   (nodeX[i  ]>=IMAGE_RIGHT) break;
        if   (nodeX[i+1]> IMAGE_LEFT ) {
          if (nodeX[i  ]< IMAGE_LEFT ) nodeX[i  ]=IMAGE_LEFT ;
          if (nodeX[i+1]> IMAGE_RIGHT) nodeX[i+1]=IMAGE_RIGHT;
          for (pixelX=nodeX[i]; pixelX<nodeX[i+1]; pixelX++){
            // have to render lighting to the viewport (vx & vy)
            SDL_RenderDrawPoint(renderer, pixelX - vx, pixelY - vy);
          }
        }
      }
    }
  }
  // END fill polygon

  /* === Render debug lines & points === */
  if(debug){
    SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255/2 );
    for(auto w : LevelManager::getInstance().getWalls()){
      w.second->draw();
    }
    SDL_SetRenderDrawColor( renderer, 0, 0, 255, 255 );
    Intersection lastIntersect = lightPolygon[0];
    for(Intersection intersect: lightPolygon){
      SDL_SetRenderDrawColor( renderer, 0, 255, 0, 255/2 );

      // Draw ray laser
      SDL_RenderDrawLine(renderer, position[0]-vx, position[1]-vy,
                         intersect.x - vx, intersect.y - vy);

      // Draw Intersection dot
      SDL_Rect r = {(int)intersect.x-2-vx, (int)intersect.y-2-vy, 4, 4};
      SDL_RenderFillRect(renderer, &r);

      // Connect Polygon
      if(intersect == lightPolygon[0]) continue;
      SDL_RenderDrawLine(renderer, lastIntersect.x - vx, lastIntersect.y-vy,
                         intersect.x-vx, intersect.y-vy);
      lastIntersect = intersect;
    }

    // draw outline of lightPolygon
    int j = lightPolygon.size()-1;
    for(int i = 0; i < (int)lightPolygon.size(); i++){
      SDL_SetRenderDrawColor( renderer, 0, i*10 % 256, 255, 255 );
      SDL_RenderDrawLine(renderer, lightPolygon[i].x - vx,
         lightPolygon[i].y-vy, lightPolygon[j].x-vx, lightPolygon[j].y-vy);
      j=i;
    }
  }



  // OPTIONAL CODE
  // float radius = 10000, distSq;
  // int playerX = player->getPosition()[0], playerY = player->getPosition()[1];
  // change intensity around circle around pointer
  // distSq = pow((playerX - pixelX), 2)+pow((playerY - pixelY), 2);
  // if( distSq < radius){
  //   SDL_SetRenderDrawColor( renderer, 0, 0, 255, (float)(distSq/radius*255)/3);
  // } else{
  //   SDL_SetRenderDrawColor( renderer, 0, 0, 255, 255/3);
  // }

  SDL_RenderPresent(renderer);
}
