//  SuperTux
//  Copyright (C) 2024 SuperTux Devel Team
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

#include <array>
#include <cmath>

#include "math/rectf.hpp"
#include "math/vector.hpp"

/**
 * A rotated rectangle represented by its center, half-width, half-height, and rotation angle.
 * The rotation is around the center of the rectangle.
 */
class RotatedRectf final
{
public:
  /** Construct a RotatedRectf from center, half-extents (half-width and half-height), and angle */
  RotatedRectf(const Vector& center, const Vector& half_extents, float angle) :
    m_center(center),
    m_half_extents(half_extents),
    m_angle(angle)
  {
  }

  /** Construct a RotatedRectf from an axis-aligned Rectf with zero rotation */
  explicit RotatedRectf(const Rectf& rect, float angle = 0.0f) :
    m_center(rect.get_middle()),
    m_half_extents(rect.get_width() / 2.0f, rect.get_height() / 2.0f),
    m_angle(angle)
  {
  }

  RotatedRectf(const RotatedRectf& other) = default;
  RotatedRectf& operator=(const RotatedRectf& other) = default;

  /** Get the center of the rectangle */
  const Vector& get_center() const { return m_center; }
  void set_center(const Vector& center) { m_center = center; }

  /** Get the half-extents (half-width, half-height) */
  const Vector& get_half_extents() const { return m_half_extents; }
  void set_half_extents(const Vector& half_extents) { m_half_extents = half_extents; }

  /** Get the rotation angle in radians */
  float get_angle() const { return m_angle; }
  void set_angle(float angle) { m_angle = angle; }

  /** Get the width of the rectangle */
  float get_width() const { return m_half_extents.x * 2.0f; }

  /** Get the height of the rectangle */
  float get_height() const { return m_half_extents.y * 2.0f; }

  /** Get the four corners of the rotated rectangle */
  std::array<Vector, 4> get_corners() const
  {
    const float cos_a = std::cos(m_angle);
    const float sin_a = std::sin(m_angle);

    // Local space corners (relative to center)
    const Vector corners_local[4] = {
      Vector(-m_half_extents.x, -m_half_extents.y),
      Vector( m_half_extents.x, -m_half_extents.y),
      Vector( m_half_extents.x,  m_half_extents.y),
      Vector(-m_half_extents.x,  m_half_extents.y)
    };

    std::array<Vector, 4> corners;
    for (int i = 0; i < 4; ++i) {
      // Rotate and translate to world space
      corners[i] = Vector(
        m_center.x + corners_local[i].x * cos_a - corners_local[i].y * sin_a,
        m_center.y + corners_local[i].x * sin_a + corners_local[i].y * cos_a
      );
    }
    return corners;
  }

  /** Get the two axes of the oriented bounding box (normalized) */
  std::array<Vector, 2> get_axes() const
  {
    const float cos_a = std::cos(m_angle);
    const float sin_a = std::sin(m_angle);
    return {
      Vector(cos_a, sin_a),   // X axis
      Vector(-sin_a, cos_a)   // Y axis
    };
  }

  /** Get the axis-aligned bounding box that contains this rotated rectangle */
  Rectf get_aabb() const
  {
    const auto corners = get_corners();
    float min_x = corners[0].x;
    float max_x = corners[0].x;
    float min_y = corners[0].y;
    float max_y = corners[0].y;
    
    for (int i = 1; i < 4; ++i) {
      min_x = std::min(min_x, corners[i].x);
      max_x = std::max(max_x, corners[i].x);
      min_y = std::min(min_y, corners[i].y);
      max_y = std::max(max_y, corners[i].y);
    }
    
    return Rectf(min_x, min_y, max_x, max_y);
  }

  /** Move the rotated rectangle by the given offset */
  void move(const Vector& offset) { m_center += offset; }

  /** Return a moved copy of this rotated rectangle */
  RotatedRectf moved(const Vector& offset) const
  {
    return RotatedRectf(m_center + offset, m_half_extents, m_angle);
  }

  /** Check if a point is inside the rotated rectangle */
  bool contains(const Vector& point) const
  {
    // Transform point to local space
    const float cos_a = std::cos(-m_angle);
    const float sin_a = std::sin(-m_angle);
    
    const Vector local_point(
      (point.x - m_center.x) * cos_a - (point.y - m_center.y) * sin_a,
      (point.x - m_center.x) * sin_a + (point.y - m_center.y) * cos_a
    );
    
    return std::abs(local_point.x) <= m_half_extents.x &&
           std::abs(local_point.y) <= m_half_extents.y;
  }

  /** Check if this rotated rectangle overlaps with another rotated rectangle
   *  Uses the Separating Axis Theorem (SAT) */
  bool overlaps(const RotatedRectf& other) const;

  /** Check if this rotated rectangle overlaps with an axis-aligned rectangle */
  bool overlaps(const Rectf& rect) const;

private:
  Vector m_center;
  Vector m_half_extents;
  float m_angle; // in radians
};

std::ostream& operator<<(std::ostream& out, const RotatedRectf& rect);
