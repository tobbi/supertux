//  SuperTux
//  Copyright (C) 2016 Tobias Markus <tobbi.bugs@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>

#include <SDL_ttf.h>

#include "util/log.hpp"


namespace {

TTF_Font* load_font(const std::string& filename, int size)
{
  TTF_Font* font = TTF_OpenFont(filename.c_str(), size);
  if(font == nullptr)
  {
    log_debug << "Couldn't open font!" << std::endl;
  }
  return font;
}

} // namespace

class FontCache
{
  static std::map<TTF_Font*,
                 std::map<std::string, SDL_Surface*>> font_glyphs;

  static SDL_Surface* get_glyph(TTF_Font* font, const std::string& text)
  {
    return font_glyphs[font][text];
  }

  static void add_glyph(TTF_Font* font, const std::string& text, SDL_Surface* glyph)
  {
    font_glyphs[font][text] = glyph;
  }

  static bool has_glyph(TTF_Font* font, const std::string& text)
  {
    return font_glyphs[font][text] != nullptr;
  }
};
