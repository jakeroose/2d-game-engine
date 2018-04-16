#include <iostream>
#include <cmath>

#include "chunk.h"
#include "gamedata.h"

Chunk::Chunk( const Vector2f& pos, const Vector2f vel,
                const string& name, Image* fm) :
  Sprite(name, pos, vel, fm),
  distance(0),
  maxDistance(Gamedata::getInstance().getXmlInt(name+"/distance")),
  variation(Gamedata::getInstance().getXmlFloat(name+"/percentVariation")/100.0),
  lifespan(Gamedata::getInstance().getXmlInt(name+"/lifespan")),
  age(0),
  tooFar(false),
  image(fm)
{
  lifespan = Gamedata::getInstance().getRandFloat(lifespan*(1-variation),
    lifespan*(1+variation));
}

void Chunk::update(Uint32 ticks) {
  float yincr = getVelocityY() * static_cast<float>(ticks) * 0.001;
  setY( getY()- yincr );
  float xincr = getVelocityX() * static_cast<float>(ticks) * 0.001;
  setX( getX()- xincr );
  distance += ( hypot(xincr, yincr) );
  age += (int) ticks;
  if (distance > maxDistance || age > lifespan) tooFar = true;
}
