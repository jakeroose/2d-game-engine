#ifndef INTERSECTION__H
#define INTERSECTION__H

#include <iostream>

class Intersection {
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
#endif
