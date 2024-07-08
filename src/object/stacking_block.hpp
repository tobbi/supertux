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

#ifndef HEADER_SUPERTUX_OBJECT_STACKING_BLOCK_HPP
#define HEADER_SUPERTUX_OBJECT_STACKING_BLOCK_HPP

#include <memory>

#include "object/block.hpp"
#include "supertux/timer.hpp"

class StackingBlock final : public Block
{
public:
  StackingBlock(const ReaderMapping& mapping);
  ~StackingBlock() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  static std::string class_name() { return "stacking_block"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Stacking Block"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual GameObjectClasses get_class_types() const override { return Block::get_class_types().add(typeid(StackingBlock)); }

  virtual ObjectSettings get_settings() override;


private:
  bool m_is_stacking;
  float m_stack_delay;
  Timer m_stack_timer;
  int m_max_stack_count;
  int m_current_stack_count;

private:
  virtual void hit(Player& player) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

private:
  StackingBlock(const StackingBlock&) = delete;
  StackingBlock& operator=(const StackingBlock&) = delete;
};

#endif

/* EOF */
