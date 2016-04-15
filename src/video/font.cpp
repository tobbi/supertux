//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                     Ingo Ruhnke <grumbel@gmail.com>
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
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <config.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <SDL_image.h>
#include <physfs.h>

#include "physfs/physfs_sdl.hpp"
#include "supertux/screen.hpp"
#include "supertux/resources.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/utf8_iterator.hpp"
#include "video/drawing_context.hpp"
#include "video/drawing_request.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"

Font::Font(const std::string& filename,
           int font_size,
           int shadowsize_):
  glyph_width(Font::VARIABLE),
  glyph_surfaces(),
  shadow_surfaces(),
  char_height(),
  shadowsize(shadowsize_),
  border(0),
  rtl(false),
  glyphs(65536),
  file_name(filename),
  fontsize(font_size),
  ttf_font()
{
  ttf_font = TTF_OpenFont(filename.c_str(), font_size);
  if(ttf_font == nullptr)
  {
    log_debug << "Couldn't open font!" << std::endl;
  }
}

Font::~Font()
{
  if(ttf_font != nullptr)
  {
    TTF_CloseFont(ttf_font);
    ttf_font = nullptr;
  }
}

float
Font::get_text_width(const std::string& text) const
{
  if(get_ttf_font() != nullptr)
  {
    if(text.length() == 0)
      return 0;

    auto texture_manager = TextureManager::current();
    auto texture = texture_manager->get(get_ttf_font(), text);
    return texture->get_texture_width();
  }
  float curr_width = 0;
  float last_width = 0;

  for(UTF8Iterator it(text); !it.done(); ++it)
  {
    if (*it == '\n')
    {
      last_width = std::max(last_width, curr_width);
      curr_width = 0;
    }
    else
    {
      if( glyphs.at(*it).surface_idx != -1 )
        curr_width += glyphs[*it].advance;
      else
        curr_width += glyphs[0x20].advance;
    }
  }

  return std::max(curr_width, last_width);
}

float
Font::get_text_height(const std::string& text) const
{
  if(text.length() == 0)
    return 0;

  std::string::size_type text_height;
  auto texture_manager = TextureManager::current();
  auto texture = texture_manager->get(get_ttf_font(), text);

  if(get_ttf_font() != nullptr)
  {
    text_height = texture->get_texture_height();
  }
  else
  {
    text_height = char_height;
  }

  for(std::string::const_iterator it = text.begin(); it != text.end(); ++it)
  { // since UTF8 multibyte characters are decoded with values
    // outside the ASCII range there is no risk of overlapping and
    // thus we don't need to decode the utf-8 string
    if (*it == '\n')
    {
      if(get_ttf_font() != nullptr)
      {
        text_height += texture->get_texture_height() + 2;
      }
      else
      {
        text_height += char_height + 2;
      }
    }
  }

  return text_height;
}

float
Font::get_height() const
{
  if(get_ttf_font() != nullptr)
  {
    // Adding a 2 pixel margin so that it looks better!
    return fontsize + 2;
  }
  return char_height;
}

std::string
Font::wrap_to_chars(const std::string& s, int line_length, std::string* overflow)
{
  // if text is already smaller, return full text
  if ((int)s.length() <= line_length) {
    if (overflow) *overflow = "";
    return s;
  }

  // if we can find a whitespace character to break at, return text up to this character
  int i = line_length;
  while ((i > 0) && (s[i] != ' ')) i--;
  if (i > 0) {
    if (overflow) *overflow = s.substr(i+1);
    return s.substr(0, i);
  }

  // FIXME: wrap at line_length, taking care of multibyte characters
  if (overflow) *overflow = "";
  return s;
}

std::string
Font::wrap_to_width(const std::string& s_, float width, std::string* overflow)
{
  std::string s = s_;

  // if text is already smaller, return full text
  if (get_text_width(s) <= width) {
    if (overflow) *overflow = "";
    return s;
  }

  // if we can find a whitespace character to break at, return text up to this character
  for (int i = s.length()-1; i >= 0; i--) {
    std::string s2 = s.substr(0,i);
    if (s[i] != ' ') continue;
    if (get_text_width(s2) <= width) {
      if (overflow) *overflow = s.substr(i+1);
      return s.substr(0, i);
    }
  }

  // FIXME: hard-wrap at width, taking care of multibyte characters
  if (overflow) *overflow = "";
  return s;
}

void
Font::draw(Renderer *renderer, const std::string& text, const Vector& pos_,
           FontAlignment alignment, DrawingEffect drawing_effect, Color color,
           float alpha) const
{
  float x = pos_.x;
  float y = pos_.y;

  std::string::size_type last = 0;
  for(std::string::size_type i = 0;; ++i)
  {
    if (text[i] == '\n' || i == text.size())
    {
      std::string temp = text.substr(last, i - last);

      // calculate X positions based on the alignment type
      Vector pos = Vector(x, y);

      if(alignment == ALIGN_CENTER)
        pos.x -= get_text_width(temp) / 2;
      else if(alignment == ALIGN_RIGHT)
        pos.x -= get_text_width(temp);

      // Cast font position to integer to get a clean drawing result and
      // no blurring as we would get with subpixel positions
      pos.x = static_cast<int>(pos.x);

      draw_text(renderer, temp, pos, drawing_effect, color, alpha);

      if (i == text.size())
        break;

      y += char_height + 2;
      last = i + 1;
    }
  }
}

void
Font::draw_text(Renderer *renderer, const std::string& text, const Vector& pos,
                DrawingEffect drawing_effect, Color color, float alpha) const
{
  if(shadowsize > 0)
    draw_chars(renderer, false, rtl ? std::string(text.rbegin(), text.rend()) : text,
               pos + Vector(shadowsize, shadowsize), drawing_effect, Color(1,1,1), alpha);

  draw_chars(renderer, true, rtl ? std::string(text.rbegin(), text.rend()) : text, pos, drawing_effect, color, alpha);
}

void
Font::draw_chars(Renderer *renderer, bool notshadow, const std::string& text,
                 const Vector& pos, DrawingEffect drawing_effect, Color color,
                 float alpha) const
{
  Vector p = pos;

  for(UTF8Iterator it(text); !it.done(); ++it)
  {
    if(*it == '\n')
    {
      p.x = pos.x;
      p.y += char_height + 2;
    }
    else if(*it == ' ')
    {
      p.x += glyphs[0x20].advance;
    }
    else
    {
      Glyph glyph;
      if( glyphs.at(*it).surface_idx != -1 )
        glyph = glyphs[*it];
      else
        glyph = glyphs[0x20];

      DrawingRequest request;

      request.pos = p + glyph.offset;
      request.drawing_effect = drawing_effect;
      request.color = color;
      request.alpha = alpha;

      SurfacePartRequest surfacepartrequest;
      surfacepartrequest.srcrect = glyph.rect;
      surfacepartrequest.dstsize = glyph.rect.get_size();
      surfacepartrequest.surface = notshadow ? glyph_surfaces[glyph.surface_idx].get() : shadow_surfaces[glyph.surface_idx].get();

      request.request_data = &surfacepartrequest;
      renderer->draw_surface_part(request);

      p.x += glyph.advance;
    }
  }
}

TTF_Font*
Font::get_ttf_font() const
{
  if(ttf_font != nullptr)
    return ttf_font;

  return Resources::normal_font->get_ttf_font();
}

unsigned int
Font::get_shadow_size() const
{
  return shadowsize;
}

/* EOF */
