#ifndef HUDELEMENT__H
#define HUDELEMENT__H
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <functional>

class HUDElement {
public:
  HUDElement(std::string s, int w, int h, int _x, int _y);
  HUDElement(std::string s);
  HUDElement(const HUDElement& h) = delete;
  void draw() const;
  void update(Uint8 ticks=0);
  bool getDisplay() const { return display; }
  void toggleDisplay() { display = !display; }
  void getMenuItems();
  void addLine(const std::string s){ strings.push_back(s); }
  void addLines(std::vector<std::string> s);
  void clear(){ strings.erase(strings.begin(), strings.end()); }
  void setCondition(std::function<bool ()> c) { condition = c; }

  const HUDElement& operator=(const HUDElement& h) = delete;
private:
  enum class Location { world, screen };
  std::string name;
  Location location;
  int width;
  int height;
  int paddingX, paddingY;
  // where to draw in viewport
  int x;
  int y;
  SDL_Renderer* renderer;
  bool display;  // should you draw the HUDElement
  std::vector<std::string> strings;
  std::function<bool ()> condition;
};

#endif
