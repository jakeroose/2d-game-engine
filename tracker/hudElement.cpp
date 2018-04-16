#include <string>
#include <regex>
#include "hudElement.h"
#include "renderContext.h"
#include "ioMod.h"
#include "viewport.h"

HUDElement::HUDElement(std::string n, int w, int h, int _x, int _y) :
  name(n),
  location(Location::screen),
  width(w), height(h),
  paddingX(10), paddingY(5),
  x(_x), y(_y),
  renderer(RenderContext::getInstance()->getRenderer()),
  display(true),
  strings(),
  condition() {
  }

HUDElement::HUDElement(std::string n) :
  name( n ),
  location(Location::screen),
  width( Gamedata::getInstance().getXmlInt("hud/width") ),
  height( Gamedata::getInstance().getXmlInt("hud/height") ),
  paddingX(10), paddingY(5),
  x( Gamedata::getInstance().getXmlInt("hud/x") ),
  y( Gamedata::getInstance().getXmlInt("hud/y") ),
  renderer( RenderContext::getInstance()->getRenderer()),
  display( true ),
  strings(),
  condition() {
  }

void HUDElement::draw() const {
  if(display == false) return;
  if(condition() == false) return;

  int lineHeight = 25;
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
  r.w = width;
  r.h = lineHeight*strings.size() + paddingY*2;
  SDL_RenderFillRect(renderer, &r);

  // Now set the color for the outline of the hud:
  SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255/2 );
  SDL_RenderDrawRect( renderer, &r );

  SDL_RenderPresent(renderer);

  for(int i = 0; i < (int)strings.size(); i++){
    IoMod::getInstance().writeText(strings[i], posX+paddingX,
                                   posY+lineHeight*i + paddingY);
  }
}

void HUDElement::update(Uint8 ticks){
  if(ticks) return;
}

void HUDElement::addLines(std::vector<std::string> s){
  for(int i = 0; i < (int)s.size(); i++){
    addLine(s[i]);
  }
}

/* WORK IN PROGRESS
  Idea is to grab menu items from xml and display them dynamically
*/
void HUDElement::getMenuItems(){
  // const std::map<std::string, std::string>& data = Gamedata::getInstance().getGameData();
  // auto it = data.begin();
  std::regex reg("hud*");
  // // for(auto& e: data){
  // while(it != data.end()){
  //   if(std::regex_match(*it.first, reg)){
  //     // std::cout << it << std::endl;
  //   }
  //   // std::cout << *it << std::endl;
  //   ++it;
  // }
  for(auto& elem: Gamedata::getInstance().getGameData()){
    if(std::regex_match(elem.first, reg)){
      std::cout << "match: " << elem.first << std::endl;
    }

  }
}
