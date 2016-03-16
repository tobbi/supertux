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
typedef std::shared_ptr<SDLTexture> SDLTexturePtr;
typedef std::map<std::string, SDLTexturePtr> SDLTextureMap;
typedef std::map<TTF_Font*, SDLTextureMap> GlyphMap;

GlyphMap font_glyphs;
GlyphMap shadow_glyphs;
} // namespace

class FontCache
{
private:
  static std::string color_to_string(const SDL_Color& c)
  {
    return std::to_string(c.r) + "|" + std::to_string(c.g) + "|" + std::to_string(c.r);
  }

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

  static SDLTexturePtr get_glyph(TTF_Font* font, const std::string& text,
                                 const SDL_Color& color)
  {
    return font_glyphs[font][color_to_string(color) + text];
  }

  static SDLTexturePtr get_shadow_glyph(TTF_Font* font, const std::string& text)
  {
    return shadow_glyphs[font][text];
  }

  static void add_glyph(TTF_Font* font, const std::string& text,
                        const SDL_Color& color)
  {
    SDLSurfacePtr text_surf = std::shared_ptr<SDL_Surface>(
      TTF_RenderUTF8_Blended(font, text.c_str(), color));

    font_glyphs[font][color_to_string(color) + text] =
                      std::shared_ptr<SDLTexture>(new SDLTexture(text_surf.get()));
  }

  static void add_shadow_glyph(TTF_Font* font, const std::string& text)
  {
    SDLSurfacePtr shadow_surf = std::shared_ptr<SDL_Surface>(
      TTF_RenderUTF8_Blended(font, text.c_str(), {0, 0, 0, 0}));

    shadow_glyphs[font][text] =
                      std::shared_ptr<SDLTexture>(new SDLTexture(shadow_surf.get()));
  }

  static bool has_glyph(TTF_Font* font, const std::string& text,
                        const SDL_Color& color)
  {
    return font_glyphs[font][color_to_string(color) + text] != nullptr;
  }

  static bool has_shadow_glyph(TTF_Font* font, const std::string& text)
  {
    return shadow_glyphs[font][text] != nullptr;
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
