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

#include "supertux/resources.hpp"
#include "util/log.hpp"
#include "video/surface.hpp"
#include "video/sdl/sdl_texture.hpp"
#include "video/texture.hpp"

namespace {
typedef std::shared_ptr<SDL_Surface> SDLSurface_Ptr;
typedef std::shared_ptr<SDLTexture> SDLTexturePtr;
typedef std::map<std::string, SDLTexturePtr> SDLTextureMap;
typedef std::map<TTF_Font*, SDLTextureMap> GlyphMap;

GlyphMap font_glyphs;
GlyphMap shadow_glyphs;
} // namespace

class FontCache
{
public:
  static std::string color_to_string(const SDL_Color& c)
  {
    return std::to_string(c.r) + "|" + std::to_string(c.g) + "|" + std::to_string(c.b);
  }

  static TTF_Font* font_from_filename(const std::string& filename)
  {
    if(filename == "fonts/andale12.stf")
      return Resources::console_font;
    if(filename == "fonts/white-small.stf")
      return Resources::example_font_small;

    return Resources::example_font;
  }

  static TTF_Font* load_font(const std::string& filename, int size)
  {
    TTF_Font* font = TTF_OpenFont(filename.c_str(), size);
    if(font == nullptr)
    {
      log_debug << "Couldn't open font!" << std::endl;
    }
    return font;
  }

  static SurfacePtr get_glyph(TTF_Font* font, const std::string& text,
                                 const SDL_Color& color)
  {
    return Surface::create(TextureManager::current()->get(font, text, color));
  }

  static SurfacePtr get_shadow_glyph(TTF_Font* font, const std::string& text)
  {
    return Surface::create(TextureManager::current()->get(font, text, {0, 0, 0, 0}));
  }
};
