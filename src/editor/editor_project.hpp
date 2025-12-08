//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
//                2025 Tobias Markus <tobbi.bugs@gmail.com>
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
#include <memory>
#include <string>

#include "supertux/tile_set.hpp"
#include "util/string_util.hpp"

class Level;
class World;

/**
 * This class contains most of the information about the current editor project,
 * including the currently opened level, levelset and world.
 */
class EditorProject
{
public:
  EditorProject();

private:
  static bool is_autosave_file(const std::string& filename) {
    return StringUtil::has_suffix(filename, "~");
  }
  static std::string get_levelname_from_autosave(const std::string& filename) {
    return is_autosave_file(filename) ? filename.substr(0, filename.size() - 1) : filename;
  }
  static std::string get_autosave_from_levelname(const std::string& filename) {
    return is_autosave_file(filename) ? filename : filename + "~";
  }

public:
  inline Level* get_level() const { return m_level.get(); }
  void set_level(std::unique_ptr<Level> level, bool reset = true);
  inline void set_level(const std::string& levelfile) { m_levelfile = levelfile; }

  inline const std::string& get_levelfile() const { return m_levelfile; }

  inline World* get_world() const { return m_world.get(); }
  inline void set_world(std::unique_ptr<World> w) { m_world = std::move(w); }

  void level_from_nothing();
  bool save_level(const std::string& filename = "", bool switch_file = false);

  std::string get_level_directory() const;
  void open_level_directory();

  inline TileSet* get_tileset() const { return m_tileset; }
  void change_tileset();

  inline bool is_temp_level() const { return m_temp_level; }

  void check_deprecated_tiles(bool focus = false);
  inline bool has_deprecated_tiles() const { return m_has_deprecated_tiles; }

  /** Checks whether the level can be saved and does not contain
      obvious issues (currently: check if main sector and a spawn point
      named "main" is present) */
  void check_save_prerequisites(const std::function<void ()>& callback) const;

  void check_unsaved_changes(const std::function<void ()>& action);

  void autosave(float dt_sec);
  void remove_autosave_file();

  void pack_addon();

private:
  std::shared_ptr<Level> m_level;
  std::unique_ptr<World> m_world;

  std::string m_levelfile;
  std::string m_autosave_levelfile;
  
  bool m_temp_level;

  float m_time_since_last_save;
  bool m_save_temp_level;
  std::string m_save_request_filename;
  bool m_save_request_switch;
  
  TileSet* m_tileset;
  bool m_has_deprecated_tiles;

private:
  EditorProject(const EditorProject&) = delete;
  EditorProject& operator=(const EditorProject&) = delete;
};