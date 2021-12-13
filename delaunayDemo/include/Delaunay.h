
#pragma once
#include <stdint.h>

// =================================================================
// NDelaunay
// =================================================================

namespace NDelaunay
{
    struct SFloat2 { float x, y; };

    bool is_in_circle(SFloat2 const &a, SFloat2 const &b, SFloat2 const &c, SFloat2 const &q);

    bool retriangulate(uint32_t *pTriIndices, uint32_t const indexCount, void const * pPositions, uint32_t const positionStride);
}