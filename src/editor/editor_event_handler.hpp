//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@gmail.com>
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

#pragma once

#include <functional>
#include <vector>
#include <string>

#include <physfs.h>

#include "editor/overlay_widget.hpp"
#include "editor/tilebox.hpp"
#include "editor/toolbar_widget.hpp"
#include "editor/toolbox_widget.hpp"
#include "editor/layers_widget.hpp"
#include "editor/scroller_widget.hpp"
#include "interface/control.hpp"
#include "supertux/screen.hpp"
#include "supertux/world.hpp"
#include "util/currenton.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/script_manager.hpp"
#include "util/string_util.hpp"
#include "video/surface_ptr.hpp"

class EditorToolbarButtonWidget;
class GameObject;
class Level;
class ObjectGroup;
class Path;
class Savegame;
class Sector;
class TileSet;
class World;

class EditorEventHandler final
{
  void on_event(const SDL_Event& ev) override;
  void on_window_resize() override;
}
