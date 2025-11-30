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

#include "st_assert.hpp"
#include <cmath>

#include "math/rotated_rectf.hpp"

const float PI = 3.14159265358979323846f;

// Helper function to compare floating point values
bool approx_equal(float a, float b, float epsilon = 0.001f)
{
  return std::abs(a - b) < epsilon;
}

int main(void)
{
  // Test construct from Rectf
  {
    Rectf rect(100.0f, 100.0f, 200.0f, 200.0f);
    RotatedRectf rotated(rect);
    
    ST_ASSERT("construct_from_rectf center x", approx_equal(rotated.get_center().x, 150.0f));
    ST_ASSERT("construct_from_rectf center y", approx_equal(rotated.get_center().y, 150.0f));
    ST_ASSERT("construct_from_rectf half_extents x", approx_equal(rotated.get_half_extents().x, 50.0f));
    ST_ASSERT("construct_from_rectf half_extents y", approx_equal(rotated.get_half_extents().y, 50.0f));
    ST_ASSERT("construct_from_rectf angle", approx_equal(rotated.get_angle(), 0.0f));
  }

  // Test contains point with no rotation
  {
    RotatedRectf rect(Vector(100.0f, 100.0f), Vector(50.0f, 25.0f), 0.0f);
    
    ST_ASSERT("contains center point", rect.contains(Vector(100.0f, 100.0f)));
    ST_ASSERT("contains inside point", rect.contains(Vector(120.0f, 110.0f)));
    ST_ASSERT("does not contain outside point x", !rect.contains(Vector(200.0f, 100.0f)));
    ST_ASSERT("does not contain outside point y", !rect.contains(Vector(100.0f, 200.0f)));
  }

  // Test contains point with rotation
  {
    RotatedRectf rect(Vector(100.0f, 100.0f), Vector(50.0f, 50.0f), PI / 4.0f);
    
    ST_ASSERT("rotated contains center", rect.contains(Vector(100.0f, 100.0f)));
    ST_ASSERT("rotated contains diagonal point", rect.contains(Vector(100.0f, 140.0f)));
    ST_ASSERT("rotated does not contain corner", !rect.contains(Vector(160.0f, 160.0f)));
  }

  // Test get corners
  {
    RotatedRectf rect(Vector(100.0f, 100.0f), Vector(50.0f, 25.0f), 0.0f);
    auto corners = rect.get_corners();
    
    ST_ASSERT("corner 0 x", approx_equal(corners[0].x, 50.0f));
    ST_ASSERT("corner 0 y", approx_equal(corners[0].y, 75.0f));
    ST_ASSERT("corner 1 x", approx_equal(corners[1].x, 150.0f));
    ST_ASSERT("corner 1 y", approx_equal(corners[1].y, 75.0f));
    ST_ASSERT("corner 2 x", approx_equal(corners[2].x, 150.0f));
    ST_ASSERT("corner 2 y", approx_equal(corners[2].y, 125.0f));
    ST_ASSERT("corner 3 x", approx_equal(corners[3].x, 50.0f));
    ST_ASSERT("corner 3 y", approx_equal(corners[3].y, 125.0f));
  }

  // Test get AABB
  {
    RotatedRectf rect1(Vector(100.0f, 100.0f), Vector(50.0f, 25.0f), 0.0f);
    Rectf aabb1 = rect1.get_aabb();
    
    ST_ASSERT("aabb left", approx_equal(aabb1.get_left(), 50.0f));
    ST_ASSERT("aabb right", approx_equal(aabb1.get_right(), 150.0f));
    ST_ASSERT("aabb top", approx_equal(aabb1.get_top(), 75.0f));
    ST_ASSERT("aabb bottom", approx_equal(aabb1.get_bottom(), 125.0f));
    
    RotatedRectf rect2(Vector(100.0f, 100.0f), Vector(50.0f, 25.0f), PI / 4.0f);
    Rectf aabb2 = rect2.get_aabb();
    
    ST_ASSERT("rotated aabb is larger width", aabb2.get_width() > 100.0f);
    ST_ASSERT("rotated aabb is larger height", aabb2.get_height() > 50.0f);
  }

  // Test overlap with no rotation
  {
    RotatedRectf rect1(Vector(100.0f, 100.0f), Vector(50.0f, 50.0f), 0.0f);
    RotatedRectf rect2(Vector(140.0f, 140.0f), Vector(50.0f, 50.0f), 0.0f);
    RotatedRectf rect3(Vector(300.0f, 300.0f), Vector(50.0f, 50.0f), 0.0f);
    
    ST_ASSERT("overlapping rectangles 1->2", rect1.overlaps(rect2));
    ST_ASSERT("overlapping rectangles 2->1", rect2.overlaps(rect1));
    ST_ASSERT("non-overlapping rectangles 1->3", !rect1.overlaps(rect3));
    ST_ASSERT("non-overlapping rectangles 3->1", !rect3.overlaps(rect1));
  }

  // Test overlap with rotation
  {
    RotatedRectf rect1(Vector(100.0f, 100.0f), Vector(50.0f, 50.0f), 0.0f);
    RotatedRectf rect2(Vector(100.0f, 100.0f), Vector(50.0f, 50.0f), PI / 4.0f);
    
    ST_ASSERT("rotated overlapping 1->2", rect1.overlaps(rect2));
    ST_ASSERT("rotated overlapping 2->1", rect2.overlaps(rect1));
  }

  // Test overlap with AABB
  {
    RotatedRectf rotated(Vector(100.0f, 100.0f), Vector(50.0f, 50.0f), PI / 4.0f);
    
    Rectf aabb1(80.0f, 80.0f, 120.0f, 120.0f);
    ST_ASSERT("rotated overlaps aabb", rotated.overlaps(aabb1));
    
    Rectf aabb2(300.0f, 300.0f, 400.0f, 400.0f);
    ST_ASSERT("rotated does not overlap aabb", !rotated.overlaps(aabb2));
  }

  // Test moved
  {
    RotatedRectf rect(Vector(100.0f, 100.0f), Vector(50.0f, 50.0f), PI / 4.0f);
    RotatedRectf moved = rect.moved(Vector(50.0f, 25.0f));
    
    ST_ASSERT("moved center x", approx_equal(moved.get_center().x, 150.0f));
    ST_ASSERT("moved center y", approx_equal(moved.get_center().y, 125.0f));
    ST_ASSERT("original center x unchanged", approx_equal(rect.get_center().x, 100.0f));
    ST_ASSERT("original center y unchanged", approx_equal(rect.get_center().y, 100.0f));
  }

  return 0;
}

/* EOF */
