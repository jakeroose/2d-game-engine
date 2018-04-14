#include "intersection.h"

std::ostream& operator<<(std::ostream& out, const Intersection& i){
  return out << i.x << ", " << i.y << ", " << i.angle;
}

void Intersection::round(){
  if((int)(x)%2) x -= 1;
  if((int)(y)%2) y -= 1;
}
