#ifndef HUDELEMENT__H
#define HUDELEMENT__H
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <functional>
#include "vector2f.h"

class HUDElement {
public:
  HUDElement(std::string s, int _x, int _y);
  HUDElement(std::string s);
  HUDElement(const HUDElement& h) = delete;
  void draw() const;
  void update(Uint8 ticks=0);
  bool getDisplay() const { return display; }
  void toggleDisplay() { display = !display; }
  void addLine(const std::string s);
  void addLines(std::vector<std::string> s);
  void clear(){ strings.erase(strings.begin(), strings.end()); }
  void setCondition(std::function<bool ()> c) { condition = c; }
  const std::string getName(){ return name; }
  void setPosition(const Vector2f& v) { x = v[0]; y = v[1]; }
  void setLocationWorld() { location = Location::world; }
  void setLocationScreen() { location = Location::screen; }

  const HUDElement& operator=(const HUDElement& h) = delete;
private:
  enum class Location { world, screen };
  std::string name;
  Location location;
  int paddingX, paddingY;
  int lineHeight;
  int maxLength;
  int x;
  int y;
  SDL_Renderer* renderer;
  bool display;
  std::vector<std::string> strings;
  std::function<bool ()> condition;
};

#endif
