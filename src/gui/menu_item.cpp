//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2015 Hume2 <teratux.mail@gmail.com>
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

#include "gui/menu_item.hpp"

#include <stdio.h>

#include "gui/menu.hpp"
#include "math/vector.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/resources.hpp"
#include "supertux/timer.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/font_functions.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

#ifdef WIN32
#  define snprintf _snprintf
#endif

//static const float FLICK_CURSOR_TIME = 0.5f;

MenuItem::MenuItem(const std::string& text_, int _id) :
  id(_id),
  text(text_),
  help()
{
  recalculate_width();
}

MenuItem::~MenuItem() {

}

void
MenuItem::change_text(const  std::string& text_)
{
  text = text_;
  recalculate_width();
}

void
MenuItem::set_help(const std::string& help_text)
{
  std::string overflow;
  help = Resources::normal_font->wrap_to_width(help_text, 600, &overflow);
  while (!overflow.empty())
  {
    help += "\n";
    help += Resources::normal_font->wrap_to_width(overflow, 600, &overflow);
  }
}

void
MenuItem::draw(DrawingContext& context, Vector pos, int menu_width, bool active) {
  context.draw_text(Resources::normal_font, text,
                    Vector( pos.x + menu_width/2 , pos.y - int(Resources::normal_font->get_height())/2 ),
                    ALIGN_CENTER, LAYER_GUI, active ? ColorScheme::Menu::active_color : get_color());
}

Color
MenuItem::get_color() const {
  return ColorScheme::Menu::default_color;
}

void
MenuItem::recalculate_width() {
  if(text.length() == 0) {
    width = 0;
    return;
  }
  auto font = Resources::normal_font->get_ttf_font();
  Uint8 r = static_cast<Uint8>(get_color().red * 255);
  Uint8 g = static_cast<Uint8>(get_color().green * 255);
  Uint8 b = static_cast<Uint8>(get_color().blue * 255);
  Uint8 a = static_cast<Uint8>(get_color().alpha * 255);
  width = TextureManager::current()->get(font, text, {r, g, b, a})->get_texture_width() + 16;
}

int
MenuItem::get_width() const {
  return width;
}

/* EOF */
