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

#include "editor/editor_project.hpp"

#include "supertux/level.hpp"
#include "supertux/world.hpp"
#include "supertux/tileset.hpp"

EditorProject::EditorProject() :
  m_levelloaded(false),
  m_enabled(true),
  m_temp_level(false),
  m_save_temp_level(false),
  m_time_since_last_save(0.f),
  m_has_deprecated_tiles(false)
{
}

void
EditorProject::set_level(std::unique_ptr<Level> level, bool reset)
{
  std::string sector_name = DEFAULT_SECTOR_NAME;
  Vector translation(0.0f, 0.0f);

  m_temp_level = (level == nullptr);

  if (!reset && m_sector) {
    translation = m_sector->get_camera().get_translation();
    sector_name = m_sector->get_name();
  }

  m_reload_request = false;
  m_enabled = true;

  if (reset) {
    m_toolbox_widget->get_tilebox().set_input_type(InputType::NONE);
  }

  m_levelloaded = true;

  if (level != nullptr)
  {
    // Reload level.
    m_level = std::move(level);
  }
  else
  {
    level_from_nothing();
  }

  if (reset) {
    m_tileset = TileManager::current()->get_tileset(m_level->get_tileset());
  }

  load_sector(sector_name);

  if (m_sector != nullptr)
  {
    m_sector->get_camera().set_mode(Camera::Mode::FREE);

    if (!reset) {
      m_sector->get_camera().set_translation(translation);
    }
  }

  m_layers_widget->refresh_sector_text();
  m_toolbox_widget->update_mouse_icon();
  m_overlay_widget->on_level_change();

  if (!reset) return;

  // Warn the user if any deprecated tiles are used throughout the level
  check_deprecated_tiles();
  if (m_has_deprecated_tiles)
  {
    std::string message = _("This level contains deprecated tiles.\nIt is strongly recommended to replace all deprecated tiles\nto avoid loss of compatibility in future versions.");
    if (!g_config->editor_show_deprecated_tiles)
      message += "\n\n" + _("Tip: Turn on \"Show Deprecated Tiles\" from the level editor menu.");

    Dialog::show_message(message);
  }
}

void
EditorProject::level_from_nothing()
{
	m_level = std::make_unique<Level>(false);
	m_level->m_name = "";
    m_level->m_license = LEVEL_DEFAULT_LICENSE;
	m_level->m_tileset = "images/tiles.strf";
	auto sector = SectorParser::from_nothing(*m_level);
	sector->set_name(DEFAULT_SECTOR_NAME);
	m_level->add_sector(std::move(sector));
	m_level->initialize();
	//m_reload_request = true;
}

bool
EditorProject::save_level(const std::string& filename, bool switch_file)
{
  if (m_temp_level && !m_save_temp_level)
  {
    MenuManager::instance().set_menu(MenuStorage::EDITOR_TEMP_SAVE_MENU);
    return false;
  }

  if (m_save_temp_level)
  {
    m_save_temp_level = false;
    m_temp_level = false;
    // Implied
    switch_file = true;
  }

  auto file = !filename.empty() ? filename : m_levelfile;

  if (switch_file)
    m_levelfile = filename;

  for (const auto& sector : m_level->m_sectors)
  {
    sector->on_editor_save();
  }
  m_level->save(m_world ? FileSystem::join(m_world->get_basedir(), file) : file);
  m_time_since_last_save = 0.f;
  remove_autosave_file();
  return true;
}

std::string
EditorProject::get_level_directory() const
{
  std::string basedir;
  if (m_world != nullptr)
  {
    basedir = m_world->get_basedir();
    if (basedir == "./")
    {
      basedir = PHYSFS_getRealDir(m_levelfile.c_str());
    }
  }
  else
  {
    basedir = FileSystem::dirname(m_levelfile);
  }
  return basedir;
}

const std::vector<Tilegroup>&
Editor::get_tilegroups() const
{
  return m_tileset->get_tilegroups();
}

void
EditorProject::change_tileset()
{
  m_tileset = TileManager::current()->get_tileset(m_level->get_tileset());
  m_toolbox_widget->get_tilebox().set_input_type(InputType::NONE);
  for (const auto& sector : m_level->m_sectors) {
    for (auto& tilemap : sector->get_objects_by_type<TileMap>()) {
      tilemap.set_tileset(m_tileset);
    }
  }
}

void
EditorProject::open_level_directory()
{
  if (m_temp_level)
    return;

  m_level->save(FileSystem::join(get_level_directory(), m_levelfile));
  auto path = FileSystem::join(PHYSFS_getWriteDir(), get_level_directory());
  FileSystem::open_path(path);
}

void
EditorProject::check_deprecated_tiles(bool focus)
{
  // Check for any deprecated tiles, used throughout the entire level
  m_has_deprecated_tiles = false;
  for (const auto& sector : m_level->get_sectors())
  {
    for (auto& tilemap : sector->get_objects_by_type<TileMap>())
    {
      int pos = -1;
      for (const uint32_t& tile_id : tilemap.get_tiles())
      {
        pos++;
        if (m_tileset->get(tile_id).is_deprecated())
        {
          // Focus on deprecated tile
          if (focus)
          {
            set_sector(sector.get());
            m_layers_widget->set_selected_tilemap(&tilemap);

            const int width = tilemap.get_width();
            m_sector->get_camera().set_translation_centered(Vector(pos % width, pos / width) * 32.f);
            keep_camera_in_bounds();
          }

          m_has_deprecated_tiles = true;
          return;
        }
      }
    }
  }
}

void
EditorProject::check_save_prerequisites(const std::function<void ()>& callback) const
{
  if (m_level->is_worldmap())
  {
    callback();
    return;
  }

  bool sector_valid = false, spawnpoint_valid = false;
  for (const auto& sector : m_level->m_sectors)
  {
    if (sector->get_name() == DEFAULT_SECTOR_NAME)
    {
      sector_valid = true;
      for (const auto& spawnpoint : sector->get_objects_by_type<SpawnPointMarker>())
      {
        if (spawnpoint.get_name() == DEFAULT_SPAWNPOINT_NAME)
        {
          spawnpoint_valid = true;
        }
      }
    }
  }

  if(sector_valid && spawnpoint_valid)
  {
    callback();
    return;
  }

  if (!sector_valid)
  {
    /*
    l10n: When translating this message, please keep "main" untranslated (the game expects the name of the sector to be "main").
    */
    Dialog::show_message(_("Couldn't find a sector with the name \"main\".\nPlease change the name of the sector where\nyou'd like the player to start to \"main\""));
  }
  else if (!spawnpoint_valid)
  {
    /*
    l10n: When translating this message, please keep "main" untranslated (the game expects the name of the spawnpoint to be "main").
    */
    Dialog::show_message(_("Couldn't find a spawnpoint with the name \"main\".\nPlease change the name of the spawnpoint where\nyou'd like the player to start to \"main\""));
  }
}

void
EditorProject::check_unsaved_changes(const std::function<void ()>& action)
{
  if (!m_levelloaded || m_temp_level)
  {
    action();
    return;
  }

  bool has_unsaved_changes = !g_config->editor_undo_tracking;
  if (!has_unsaved_changes)
  {
    for (const auto& sector : m_level->m_sectors)
    {
      if (sector->has_object_changes())
      {
        has_unsaved_changes = true;
        break;
      }
    }
  }

  if (has_unsaved_changes)
  {
    m_enabled = false;
    auto dialog = std::make_unique<Dialog>();
    dialog->set_text(g_config->editor_undo_tracking ? _("This level contains unsaved changes, do you want to save?") :
                                                      _("This level may contain unsaved changes, do you want to save?"));
    dialog->add_default_button(_("Yes"), [this, action] {
      check_save_prerequisites([this, action] {
        save_level();
        action();
        m_enabled = true;
      });
    });
    dialog->add_button(_("No"), [this, action] {
      action();
	  set_level(nullptr, true);
      m_enabled = true;
    });
    dialog->add_button(_("Cancel"), [this] {
      m_enabled = true;
    });
    MenuManager::instance().set_dialog(std::move(dialog));
  }
  else
  {
    action();
  }
}

void
EditorProject::remove_autosave_file()
{
  if (m_temp_level)
    return;

  // Clear the auto-save file.
  if (!m_autosave_levelfile.empty())
  {
    // Try to remove the test level using the PhysFS file system
    if (physfsutil::remove(m_autosave_levelfile) != 0)
    {
      // This file is not inside any PhysFS mounts,
      // try to remove this using normal file system
      // methods.
      FileSystem::remove(m_autosave_levelfile);
    }
  }
}

void
EditorProject::pack_addon()
{
  auto id = FileSystem::basename(get_world()->get_basedir());
  auto output_file_path = FileSystem::join(PHYSFS_getWriteDir(), "addons/" + id + ".zip");

  int version = 0;
  if (PHYSFS_exists(output_file_path.c_str()))
  {
    try
    {
      Partio::ZipFileReader zipold(output_file_path);
      auto info_file = zipold.Get_File(id + ".nfo");
      if (info_file)
      {
        auto info_stream = ReaderDocument::from_stream(*info_file);
        auto a = info_stream.get_root().get_mapping();
        a.get("version", version);
      }
    }
    catch(const std::exception& e)
    {
      log_warning << e.what() << std::endl;
    }
  }
  version++;

  Partio::ZipFileWriter zip(output_file_path);
  physfsutil::enumerate_files_recurse(get_world()->get_basedir(),
    [&zip](const std::string& full_path)
    {
      auto os = zip.Add_File(full_path);
      *os << std::ifstream(FileSystem::join(PHYSFS_getWriteDir(), full_path)).rdbuf();
      return false;
    });

  std::stringstream ss;
  Writer info(ss);

  info.start_list("supertux-addoninfo");
  {
    info.write("id", id);
    info.write("version", version);
    info.write("type", get_world()->get_type());

    info.write("title", get_world()->get_title());
    info.write("author", get_level()->get_author());
    info.write("license", get_level()->get_license());
  }
  info.end_list("supertux-addoninfo");

  *zip.Add_File(id + ".nfo") << ss.rdbuf();
}