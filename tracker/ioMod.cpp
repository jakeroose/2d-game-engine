#include <SDL_image.h>
#include "ioMod.h"
#include "gamedata.h"
#include "renderContext.h"

IoMod& IoMod::getInstance() {
  static IoMod instance;
  return instance;
}

IoMod::~IoMod() {
  for(auto e : textures) SDL_DestroyTexture(e.second);
  TTF_CloseFont(font);
  TTF_Quit();
}

IoMod::IoMod() :
  init(TTF_Init()),
  renderer( RenderContext::getInstance()->getRenderer() ),
  font(TTF_OpenFont(Gamedata::getInstance().getXmlStr("font/file").c_str(),
                    Gamedata::getInstance().getXmlInt("font/size"))),
  textColor({0xff, 0, 0, 0}),
  textures()
{
  if ( init == -1 ) {
    throw std::string("error: Couldn't init font");
  }
  if (font == NULL) {
    throw std::string("error: font not found");
  }
  textColor.r = Gamedata::getInstance().getXmlInt("font/red");
  textColor.g = Gamedata::getInstance().getXmlInt("font/green");
  textColor.b = Gamedata::getInstance().getXmlInt("font/blue");
  textColor.a = Gamedata::getInstance().getXmlInt("font/alpha");
}

SDL_Texture* IoMod::readTexture(const std::string& filename) {
  SDL_Texture *texture = IMG_LoadTexture(renderer, filename.c_str());
  if ( texture == NULL ) {
    throw std::string("Couldn't load ") + filename;
  }
  return texture;
}

SDL_Surface* IoMod::readSurface(const std::string& filename) {
  SDL_Surface *surface = IMG_Load(filename.c_str());
  if ( !surface ) {
    throw std::string("Couldn't load ") + filename;
  }
  return surface;
}

void IoMod::writeText(const std::string& msg, int x, int y) {
  writeText(msg, x, y, textColor);
}

void IoMod::writeText(const std::string& msg, int x, int y, SDL_Color _textColor) {
  int textWidth, textHeight;
  SDL_Texture* texture = getTexture(msg, _textColor);
  SDL_QueryTexture(texture, NULL, NULL, &textWidth, &textHeight);

  SDL_Rect dst = {x, y, textWidth, textHeight};
  SDL_RenderCopy(renderer, texture, NULL, &dst);
}

SDL_Texture* IoMod::getTexture(const std::string& msg,
                               SDL_Color _textColor) {
  auto it = textures.find(msg);

  if(it != textures.end()){
    return (*it).second;
  }
  else {
    SDL_Texture* t = createTextTexture(msg, _textColor);
    textures.insert(std::pair<std::string, SDL_Texture*>(msg, t));
    // std::cout << "created texture: " << msg << std::endl;
    return t;
  }
}

// need to use this for key value if text is more than 1 color
std::string IoMod::colorToString(SDL_Color c) const {
  return (std::to_string(c.r)+std::to_string(c.g)
          +std::to_string(c.b)+std::to_string(c.r));
}

SDL_Texture* IoMod::createTextTexture(const std::string& msg,
                                      SDL_Color _textColor) const {
  SDL_Surface* surface =
    TTF_RenderText_Solid(font, msg.c_str(), _textColor);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  return texture;
}
