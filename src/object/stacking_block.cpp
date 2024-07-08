//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "object/stacking_block.hpp"

#include <algorithm>

#include "editor/editor.hpp"
#include "object/player.hpp"
#include "object/camera.hpp"
#include "supertux/info_box_line.hpp"
#include "supertux/sector.hpp"
#include "supertux/timer.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

StackingBlock::StackingBlock(const ReaderMapping& mapping) :
  Block(mapping, "images/objects/bonus_block/infoblock.sprite"),
  m_is_stacking(),
  m_stack_delay(),
  m_stack_timer(),
  m_max_stack_count(),
  m_current_stack_count(),
  m_last_block_position(get_pos())
{
}

StackingBlock::~StackingBlock()
{
}

ObjectSettings
StackingBlock::get_settings()
{
  ObjectSettings result = Block::get_settings();
  return result;
}

void
StackingBlock::hit(Player& player)
{
  start_bounce(&player);
}

HitResponse
StackingBlock::collision(GameObject& other, const CollisionHit& hit_)
{
  if(!m_is_stacking)
  {
    m_is_stacking = true;
    m_stack_timer.start(m_stack_delay);
  }
  return Block::collision(other, hit_);
}

void
StackingBlock::update(float dt_sec)
{
  if(m_stack_timer.check())
  {
    if(m_current_stack_count < m_max_stack_count)
    {
      m_current_stack_count++;
      //Sector::get().add<SpecialRiser>(get_pos(), std::make_unique<Trampoline>(get_pos(), Trampoline::STATIONARY), true);
      log_warning << "Next stack" << std::endl;
      m_stack_timer.start(m_stack_delay);
      m_last_block_position += Vector(0, -32);
    }
  }
  Block::update(dt_sec);
}

void
StackingBlock::draw(DrawingContext& context)
{
  Block::draw(context);
}

/* EOF */
