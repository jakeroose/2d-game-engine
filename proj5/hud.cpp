#include <string>
#include <regex>
#include "hud.h"
#include "renderContext.h"
#include "ioMod.h"

HUD::HUD(std::string n, int w, int h, int _x, int _y) :
  name(n), width(w), height(h), x(_x), y(_y), renderer(RenderContext::getInstance()->getRenderer()), display(true) {}

HUD::HUD(std::string n) :
  name( n ),
  width( Gamedata::getInstance().getXmlInt("hud/width") ),
  height( Gamedata::getInstance().getXmlInt("hud/height") ),
  x( Gamedata::getInstance().getXmlInt("hud/x") ),
  y( Gamedata::getInstance().getXmlInt("hud/y") ),
  renderer( RenderContext::getInstance()->getRenderer()),
  display( true ) {}


// NOTE: this IS kind of pointless since it just draws at middle of screen
// which could be set during initialization..
void HUD::drawCentered() const {
  if(display == false) return;
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255/2);
  SDL_Rect r;
  r.x = x - width/2;
  r.y = y - height/2;
  r.w = width;
  r.h = height;
  SDL_RenderFillRect(renderer, &r);

  // Now set the color for the outline of the hud:
  SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255/2 );
  SDL_RenderDrawRect( renderer, &r );

  SDL_RenderPresent(renderer);

  IoMod::getInstance().writeText(name, x + 10, y + 10);
}

void HUD::draw() const {
  if(display == false) return;
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 50, 50, 50, 100);
  SDL_Rect r;
  r.x = x;
  r.y = y;
  r.w = width;
  r.h = height;
  SDL_RenderFillRect(renderer, &r);

  // Now set the color for the outline of the hud:
  SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255/2 );
  SDL_RenderDrawRect( renderer, &r );

  SDL_RenderPresent(renderer);

  // draw title
  // IoMod::getInstance().writeText(name, x + 10, y + 10);
}


/* WORK IN PROGRESS
  Idea is to grab menu items from xml and display them dynamically
*/
void HUD::getMenuItems(){
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
