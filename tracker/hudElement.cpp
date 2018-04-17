#include <string>
#include <regex>
#include "hudElement.h"
#include "renderContext.h"
#include "ioMod.h"
#include "viewport.h"
#include "gamedata.h"

HUDElement::HUDElement(std::string n, int _x, int _y) :
  name(n),
  location(Location::screen),
  paddingX(Gamedata::getInstance().getXmlInt("font/paddingX")),
  paddingY(Gamedata::getInstance().getXmlInt("font/paddingY")),
  lineHeight(Gamedata::getInstance().getXmlInt("font/size")),
  maxLength(),
  x(_x), y(_y),
  renderer(RenderContext::getInstance()->getRenderer()),
  display(true),
  strings(),
  condition() {
  }

HUDElement::HUDElement(std::string n) :
  name( n ),
  location(Location::screen),
  paddingX(Gamedata::getInstance().getXmlInt("font/paddingX")),
  paddingY(Gamedata::getInstance().getXmlInt("font/paddingY")),
  lineHeight(Gamedata::getInstance().getXmlInt("font/size")),
  maxLength(),
  x( Gamedata::getInstance().getXmlInt("hud/x") ),
  y( Gamedata::getInstance().getXmlInt("hud/y") ),
  renderer( RenderContext::getInstance()->getRenderer()),
  display( true ),
  strings(),
  condition() {
  }

void HUDElement::draw() const {
  if(condition() == false) return;

  int posX = x;
  int posY = y;
  if(location == Location::world){
    posX -= Viewport::getInstance().getX();
    posY -= Viewport::getInstance().getY();
  }

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 50, 50, 50, 100);
  SDL_Rect r;
  r.x = posX;
  r.y = posY;
  r.w = maxLength*lineHeight*0.63+paddingX;
  r.h = (lineHeight+paddingY+2)*strings.size();
  SDL_RenderFillRect(renderer, &r);

  // Now set the color for the outline of the hud:
  SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255/2 );
  SDL_RenderDrawRect( renderer, &r );

  for(int i = 0; i < (int)strings.size(); i++){
    IoMod::getInstance().writeText(strings[i], posX+paddingX,
                                   posY+(lineHeight+paddingY)*i+5);
  }
}

void HUDElement::update(Uint8 ticks){
  if(ticks) return;
}

void HUDElement::addLine(const std::string s){
  strings.push_back(s);
  if((int)s.length() > maxLength) maxLength = s.length();
}


void HUDElement::addLines(std::vector<std::string> s){
  for(int i = 0; i < (int)s.size(); i++){
    addLine(s[i]);
    if((int)s[i].length() > maxLength) maxLength = s[i].length();
  }
}
