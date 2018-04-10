#include "intersection.h"

std::ostream& operator<<(std::ostream& out, const Intersection& i){
  return out << i.x << ", " << i.y << ", " << i.angle;
}
