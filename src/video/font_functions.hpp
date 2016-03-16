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
#include "video/sdl/sdl_texture.hpp"
#include "video/texture.hpp"

namespace {
typedef std::shared_ptr<SDL_Surface> SDLSurfacePtr;
typedef std::map<std::string, SDLSurfacePtr> SDLSurfaceMap;
typedef std::map<TTF_Font*, SDLSurfaceMap> GlyphMap;

GlyphMap font_glyphs;
} // namespace

class FontCache
{
public:
  static TTF_Font* load_font(const std::string& filename, int size)
  {
    TTF_Font* font = TTF_OpenFont(filename.c_str(), size);
    if(font == nullptr)
    {
      log_debug << "Couldn't open font!" << std::endl;
    }
    return font;
  }

  static SDLSurfacePtr get_glyph(TTF_Font* font, const std::string& text)
  {
    return font_glyphs[font][text];
  }

  static void add_glyph(TTF_Font* font, const std::string& text, SDLSurfacePtr glyph)
  {
    font_glyphs[font][text] = glyph;
  }

  static bool has_glyph(TTF_Font* font, const std::string& text)
  {
    return font_glyphs[font][text] != nullptr;
  }

  static void delete_textures()
  {
    for(auto it = font_glyphs.begin(); it != font_glyphs.end(); ++it)
    {
      auto map = it->second;
      for(auto tex_it = map.begin(); tex_it != map.end(); ++tex_it)
      {
        auto texturePtr = tex_it->second;
        texturePtr.reset();
      }
    }
  }
};
