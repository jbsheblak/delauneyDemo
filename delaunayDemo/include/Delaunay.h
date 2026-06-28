
#pragma once
#include <stdint.h>

// =================================================================
// NDelaunay
// =================================================================

namespace NDelaunay
{
   /// Perform a Delaunay triangularization on the specified input mesh.
   /// @param[in] pTriIndices An array of triangle indices that are 'indexCount' long.
   /// @param[in] indexCount The number of indices in pTriIndices to examine.
   /// @param[in] pPositions A non-null array of float2 positions.
   /// @param[in] positionStride A byte stride from one float2 instance to the start of another.
   /// @return true if the triangularization was successful, false otherwise.
   bool retriangulate_with_edge_flip(uint32_t * const pTriIndices, uint32_t const indexCount, void const * const pPositions, uint32_t const positionStride);
} // namespace NDelaunay