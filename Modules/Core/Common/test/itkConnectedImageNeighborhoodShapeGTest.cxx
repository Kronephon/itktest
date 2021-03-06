/*=========================================================================
*
*  Copyright Insight Software Consortium
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/

// First include the header file to be tested:
#include "itkConnectedImageNeighborhoodShape.h"

#include "itkImageNeighborhoodOffsets.h"

#include "itkOffset.h"
#include "itkSize.h"

#include <climits> // For INT_MAX and SIZE_MAX.
#include <vector>

#include <gtest/gtest.h>

namespace
{
  template <unsigned int VImageDimension,
    std::size_t VMaximumCityblockDistance,
    bool VIncludeCenterPixel,
    std::size_t VExpectedNumberOfOffsets>
  void Assert_GetNumberOfOffsets_returns_expected_number()
  {
    using ShapeType = itk::Experimental::ConnectedImageNeighborhoodShape<VImageDimension>;

    // Test GetNumberOfOffsets() on a 'constexpr shape', at compile-time:
    constexpr ShapeType constexprShape(VMaximumCityblockDistance, VIncludeCenterPixel);
    static_assert(constexprShape.GetNumberOfOffsets() == VExpectedNumberOfOffsets,
      "Checked ConnectedImageNeighborhoodShape::GetNumberOfOffsets().");

    // Test GetNumberOfOffsets() on a non-const shape, at run-time:
    ShapeType nonConstShape = constexprShape;
    ASSERT_EQ(nonConstShape.GetNumberOfOffsets(), VExpectedNumberOfOffsets);
  }


  template <unsigned int VImageDimension,
    std::size_t VMaximumCityblockDistance,
    std::size_t VExpectedNumberOfOffsetsExcludingCenterPixel>
  void Assert_GetNumberOfOffsets_returns_expected_number()
  {
    // Test GetNumberOfOffsets() for both VIncludeCenterPixel = false and
    // VIncludeCenterPixel = true:
    Assert_GetNumberOfOffsets_returns_expected_number<VImageDimension,
      VMaximumCityblockDistance, false, VExpectedNumberOfOffsetsExcludingCenterPixel>();
    Assert_GetNumberOfOffsets_returns_expected_number<VImageDimension,
      VMaximumCityblockDistance, true, VExpectedNumberOfOffsetsExcludingCenterPixel + 1>();
  }


  // Asserts that GenerateImageNeighborhoodOffsets(shape) returns the expected
  // result for a shape with the specified ImageDimension and MaximumCityblockDistance,
  template <unsigned int VImageDimension, std::size_t VMaximumCityblockDistance>
  void Assert_GenerateImageNeighborhoodOffsets_returns_expected_offsets_excluding_center_pixel(
    const std::vector<itk::Offset<VImageDimension>>& expectedOffsets)
  {
    using ShapeType =
      itk::Experimental::ConnectedImageNeighborhoodShape<VImageDimension>;

    const bool includeCenterPixel = false;
    const ShapeType shape{ VMaximumCityblockDistance, includeCenterPixel };

    ASSERT_EQ(GenerateImageNeighborhoodOffsets(shape), expectedOffsets);
  }

  template <unsigned int VImageDimension>
  void Assert_The_middle_offset_is_all_zero_when_center_pixel_is_included()
  {
    using ShapeType =
      itk::Experimental::ConnectedImageNeighborhoodShape<VImageDimension>;
    using OffsetType = itk::Offset<VImageDimension>;

    const bool includeCenterPixel = true;
    const OffsetType allZeroOffset{ {} };

    for (unsigned int maximumCityblockDistance = 0; maximumCityblockDistance < VImageDimension; ++maximumCityblockDistance)
    {
      const ShapeType shape{ maximumCityblockDistance, includeCenterPixel };
      const std::size_t numberOfOffsets = shape.GetNumberOfOffsets();
      const std::vector<OffsetType> offsets = GenerateImageNeighborhoodOffsets(shape);

      ASSERT_FALSE(offsets.empty());
      ASSERT_TRUE((offsets.size() % 2) == 1);

      const OffsetType middleOffset = offsets[(numberOfOffsets - 1) / 2];
      ASSERT_EQ(middleOffset, allZeroOffset);
    }

  }

} // namespace


TEST(ConnectedImageNeighborhoodShape, GetNumberOfOffsets_returns_expected_value)
{
  // 0-dimensional:
  Assert_GetNumberOfOffsets_returns_expected_number<0, 0, 0>();
  Assert_GetNumberOfOffsets_returns_expected_number<0, SIZE_MAX, 0>();

  // 1-dimensional:
  Assert_GetNumberOfOffsets_returns_expected_number<1, 0, 0>();
  Assert_GetNumberOfOffsets_returns_expected_number<1, 1, 2>();
  Assert_GetNumberOfOffsets_returns_expected_number<1, SIZE_MAX, 2>();

  // 2-dimensional (0-connected, 4-connected, 8-connected):
  Assert_GetNumberOfOffsets_returns_expected_number<2, 0, 0>();
  Assert_GetNumberOfOffsets_returns_expected_number<2, 1, 4>();
  Assert_GetNumberOfOffsets_returns_expected_number<2, 2, 8>();
  Assert_GetNumberOfOffsets_returns_expected_number<2, SIZE_MAX, 8>();

  // 3-dimensional (0-connected, 6-connected, 18-connected, 26-connected):
  Assert_GetNumberOfOffsets_returns_expected_number<3, 0, 0>();
  Assert_GetNumberOfOffsets_returns_expected_number<3, 1, 6>();
  Assert_GetNumberOfOffsets_returns_expected_number<3, 2, 18>();
  Assert_GetNumberOfOffsets_returns_expected_number<3, 3, 26>();
  Assert_GetNumberOfOffsets_returns_expected_number<3, SIZE_MAX, 26>();

  // INT_MAX-dimensional:
  Assert_GetNumberOfOffsets_returns_expected_number<INT_MAX, 0, 0>();
  Assert_GetNumberOfOffsets_returns_expected_number<INT_MAX, 1, 2U * INT_MAX>();
}


TEST(ConnectedImageNeighborhoodShape, GenerateImageNeighborhoodOffsets_returns_expected_offsets)
{
  Assert_GenerateImageNeighborhoodOffsets_returns_expected_offsets_excluding_center_pixel<1, 1>(
  {
    {{-1}}, {{1}}
  });

  Assert_GenerateImageNeighborhoodOffsets_returns_expected_offsets_excluding_center_pixel<2, 1>(
  {
    {{0, -1}}, {{-1, 0}}, {{1, 0}}, {{0, 1}}
  });

  Assert_GenerateImageNeighborhoodOffsets_returns_expected_offsets_excluding_center_pixel<2, 2>(
  {
    {{-1, -1}}, {{0, -1}}, {{1, -1}}, {{-1, 0}},
    {{1, 0}}, {{-1, 1}}, {{0, 1}}, {{1, 1}}
  });

  Assert_GenerateImageNeighborhoodOffsets_returns_expected_offsets_excluding_center_pixel<3, 1>(
  {
    {{0, 0, -1}}, {{0, -1, 0}}, {{-1, 0, 0}},
    {{1, 0, 0}}, {{0, 1, 0}}, {{0, 0, 1}}
  });

  Assert_GenerateImageNeighborhoodOffsets_returns_expected_offsets_excluding_center_pixel<3, 2>(
  {
    {{0, -1, -1}}, {{-1, 0, -1}}, {{0, 0, -1}},
    {{1, 0, -1}}, {{0, 1, -1}}, {{-1, -1, 0}},
    {{0, -1, 0}}, {{1, -1, 0}}, {{-1, 0, 0}},
    {{1, 0, 0}}, {{-1, 1, 0}}, {{0, 1, 0}},
    {{1, 1, 0}}, {{0, -1, 1}}, {{-1, 0, 1}},
    {{0, 0, 1}}, {{1, 0, 1}}, {{0, 1, 1}}
  });

  Assert_GenerateImageNeighborhoodOffsets_returns_expected_offsets_excluding_center_pixel<3, 3>(
  {
    {{-1, -1, -1}}, {{0, -1, -1}},  {{1, -1, -1}},  {{-1, 0, -1}},
    {{0, 0, -1}}, {{1, 0, -1}}, {{-1, 1, -1}}, {{0, 1, -1}},
    {{1, 1, -1}}, {{-1, -1, 0}}, {{0, -1, 0}}, {{1, -1, 0}},
    {{-1, 0, 0}}, {{1, 0, 0}}, {{-1, 1, 0}}, {{0, 1, 0}},
    {{1, 1, 0}}, {{-1, -1, 1}}, {{0, -1, 1}}, {{1, -1, 1}},
    {{-1, 0, 1}}, {{0, 0, 1}}, {{1, 0, 1}}, {{-1, 1, 1}},
    {{0, 1, 1}}, {{1, 1, 1}}
  });
}


TEST(ConnectedImageNeighborhoodShape, The_middle_offset_is_all_zero_when_center_pixel_is_included)
{
  Assert_The_middle_offset_is_all_zero_when_center_pixel_is_included<1>();
  Assert_The_middle_offset_is_all_zero_when_center_pixel_is_included<2>();
  Assert_The_middle_offset_is_all_zero_when_center_pixel_is_included<3>();
}
