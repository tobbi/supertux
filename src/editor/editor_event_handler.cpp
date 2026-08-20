
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

#include "editor/editor_event_handler.hpp"

void
EditorEventHandler::on_event(const SDL_Event& ev)
{
  auto editor = Editor::current();
  if (!editor->has_focus())
    return;

  for(const auto& control : m_controls)
    if (control->event(ev))
      return;

  try
  {
    if (ev.type == SDL_EVENT_MOUSE_MOTION)
    {
      m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(ev.motion.x, ev.motion.y);

      // If properties sidebar controls are active and the mouse is hovering over the sidebar,
      // do not propagate mouse motion to the editor or its widgets.
      if (!m_controls.empty() && Rectf(0, 32.0f, 200.0f, SCREEN_HEIGHT - 32.0f).contains(m_mouse_pos))
        return;
    }
    else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
      switch (ev.button.button)
      {
        case SDL_BUTTON_X1:
          editor->undo();
          break;
        case SDL_BUTTON_X2:
          editor->redo();
          break;
      }
    } else {
      // If properties sidebar controls are active and the mouse is hovering over the sidebar,
      // do not propagate mouse events to the editor or its widgets.
      if (!m_controls.empty() &&
          (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
           ev.type == SDL_EVENT_MOUSE_BUTTON_UP ||
           ev.type == SDL_EVENT_MOUSE_WHEEL) &&
          Rectf(0, 32.0f, 200.0f, SCREEN_HEIGHT - 32.0f).contains(m_mouse_pos))
      {
        return;
      }

      if (ev.type == SDL_EVENT_KEY_DOWN)
      {
        m_ctrl_pressed = ev.key.mod & SDL_KMOD_CTRL;
        m_shift_pressed = ev.key.mod & SDL_KMOD_SHIFT;
        m_alt_pressed = ev.key.mod & SDL_KMOD_ALT;

        if (m_ctrl_pressed)
          m_scroll_speed = 16.0f;
        else if (ev.key.mod & SDL_KMOD_RSHIFT)
          m_scroll_speed = 96.0f;

        if (ev.key.key == SDLK_F6)
        {
          Compositor::s_render_lighting = !Compositor::s_render_lighting;
          return;
        }
        else if (m_ctrl_pressed)
        {
          switch (ev.key.key)
          {
            case SDLK_T:
              if (m_shift_pressed && m_alt_pressed)
              {
                test_level(m_last_test_pos);
                break;
              }

              if (m_shift_pressed)
                m_last_test_pos = std::pair<std::string, Vector>(get_sector()->get_name(), m_overlay_widget->get_sector_pos());
              else
                m_last_test_pos = std::nullopt;

              test_level(m_last_test_pos);
              break;
            case SDLK_S:
              save_level();
              break;
            case SDLK_Z:
              undo();
              break;
            case SDLK_Y:
              redo();
              break;
            case SDLK_H:
              m_show_draggables = !m_show_draggables;
              if (!m_show_draggables)
                m_show_draggables_hint.start(6.7f);
              break;
            case SDLK_X:
              m_toolbar_widget->toggle_tile_object_mode();
              break;
            case SDLK_PAGEUP:
              m_toolbox_widget->switch_current_group(-1);
              break;
            case SDLK_PAGEDOWN:
              m_toolbox_widget->switch_current_group(1);
              break;
            case SDLK_PLUS: // Zoom in
            case SDLK_EQUALS:
            case SDLK_KP_PLUS:
              m_key_zoomed = true;
              m_new_scale = m_sector->get_camera().get_current_scale() + CAMERA_ZOOM_SENSITIVITY;
              break;
            case SDLK_MINUS: // Zoom out
            case SDLK_KP_MINUS:
              m_key_zoomed = true;
              m_new_scale = m_sector->get_camera().get_current_scale() - CAMERA_ZOOM_SENSITIVITY;
              break;
            case SDLK_D: // Reset zoom
              m_new_scale = 1.f;
              break;
            default:
              break;
          }
        }
      }
      else if (ev.type == SDL_EVENT_KEY_UP)
      {
        m_ctrl_pressed = ev.key.mod & SDL_KMOD_CTRL;
        m_shift_pressed = ev.key.mod & SDL_KMOD_SHIFT;
        m_alt_pressed = ev.key.mod & SDL_KMOD_ALT;

        if (!m_ctrl_pressed && !(ev.key.mod & SDL_KMOD_RSHIFT))
          m_scroll_speed = 32.0f;
      }
      else if (ev.type == SDL_EVENT_PEN_BUTTON_DOWN)
      {
        m_pen_down = true;
      }
      else if (ev.type == SDL_EVENT_PEN_BUTTON_UP)
      {
        m_pen_down = false;
      }
      else if (ev.type == SDL_EVENT_MOUSE_WHEEL && !m_toolbox_widget->has_mouse_focus() && !m_layers_widget->has_mouse_focus())
      {
#if SDL_VERSION_ATLEAST(3, 2, 12)
        float wheel_x = g_config->precise_scrolling ? ev.wheel.x : ev.wheel.integer_x;
        float wheel_y = g_config->precise_scrolling ? ev.wheel.y : ev.wheel.integer_y;
#else
        float wheel_x = ev.wheel.x;
        float wheel_y = ev.wheel.y;
#endif
        if (g_config->invert_wheel_x) wheel_x *= -1.f;
        if (g_config->invert_wheel_y) wheel_y *= -1.f;
        // Scroll or zoom with mouse wheel, if the mouse is not over the toolbox.
        // The toolbox does scrolling independently from the main area.
        if (m_ctrl_pressed)
          m_new_scale = m_sector->get_camera().get_current_scale() + static_cast<float>(wheel_y) * CAMERA_ZOOM_SENSITIVITY;
        else
          scroll({ static_cast<float>((m_shift_pressed ? wheel_y * (g_config->editor_invert_shift_scroll ? -1 : 1) : wheel_x) * 40),
                   static_cast<float>((m_shift_pressed ? wheel_x : wheel_y) * -40) });
      }
    }

    BIND_SECTOR(*m_sector);

    if (m_toolbar_widget->event(ev))
      return;

    for (const auto& widget : m_widgets)
      if (widget->event(ev))
        break;
  }
  catch(const std::exception& err)
  {
    log_warning << "error while processing Editor::event(): " << err.what() << std::endl;
  }
}