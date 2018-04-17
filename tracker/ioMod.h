#ifndef IOMOD__H
#define IOMOD__H

#include <iostream>
#include <string>
#include <map>
#include <SDL.h>
#include <SDL_ttf.h>

class IoMod {
public:
  static IoMod& getInstance();
  ~IoMod();
  SDL_Texture* readTexture(const std::string& filename);
  SDL_Surface* readSurface(const std::string& filename);
  void writeText(const std::string&, int, int);
  void writeText(const std::string&, int, int, SDL_Color);
  SDL_Renderer* getRenderer() const { return renderer; }
private:
  int init;
  SDL_Renderer* renderer;
  TTF_Font* font;
  SDL_Color textColor;
  std::map<std::string, SDL_Texture*> textures;

  std::string  colorToString(SDL_Color) const;
  SDL_Texture* getTexture(const std::string& msg, SDL_Color _textColor) ;
  SDL_Texture* createTextTexture(const std::string& msg,
                                 SDL_Color _textColor) const;

  IoMod();
  IoMod(const IoMod&) = delete;
  IoMod& operator=(const IoMod&) = delete;
};
#endif
