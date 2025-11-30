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

#include "math/rotated_rectf.hpp"

#include <algorithm>
#include <iostream>

namespace {

/** Project all corners of a rotated rectangle onto an axis and return min/max projections */
void project_on_axis(const std::array<Vector, 4>& corners, const Vector& axis, float& min_proj, float& max_proj)
{
  min_proj = glm::dot(corners[0], axis);
  max_proj = min_proj;
  
  for (int i = 1; i < 4; ++i) {
    float proj = glm::dot(corners[i], axis);
    min_proj = std::min(min_proj, proj);
    max_proj = std::max(max_proj, proj);
  }
}

/** Check if two projection ranges overlap */
bool projections_overlap(float min1, float max1, float min2, float max2)
{
  return !(max1 < min2 || max2 < min1);
}

} // namespace

bool
RotatedRectf::overlaps(const RotatedRectf& other) const
{
  // Get corners of both rectangles
  const auto corners1 = get_corners();
  const auto corners2 = other.get_corners();
  
  // Get the four axes to test (two from each rectangle)
  const auto axes1 = get_axes();
  const auto axes2 = other.get_axes();
  
  // Test all four axes
  std::array<Vector, 4> axes = { axes1[0], axes1[1], axes2[0], axes2[1] };
  
  for (const auto& axis : axes) {
    float min1, max1, min2, max2;
    project_on_axis(corners1, axis, min1, max1);
    project_on_axis(corners2, axis, min2, max2);
    
    if (!projections_overlap(min1, max1, min2, max2)) {
      return false; // Found a separating axis
    }
  }
  
  return true; // No separating axis found, rectangles overlap
}

bool
RotatedRectf::overlaps(const Rectf& rect) const
{
  // Convert axis-aligned rect to rotated rect with zero angle
  RotatedRectf other(rect, 0.0f);
  return overlaps(other);
}

std::ostream& operator<<(std::ostream& out, const RotatedRectf& rect)
{
  out << "RotatedRectf(center=" << rect.get_center()
      << ", half_extents=" << rect.get_half_extents()
      << ", angle=" << rect.get_angle() << ")";
  return out;
}
