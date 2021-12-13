

#include "Delaunay.h"
#include <assert.h>
#include <map>
#include <vector>
#include <algorithm>

// =================================================================
// NDelaunay
// =================================================================

namespace NDelaunay
{
    // --------------------------------------------------------------

    struct SFloat3
    {
        explicit SFloat3() = default;
        explicit SFloat3 (float _x, float _y, float _z) : x(_x) , y(_y) , z(_z) {}
        float x,y,z;
    };

    // --------------------------------------------------------------

    float det3(SFloat3 const &a, SFloat3 const &b, SFloat3 const &c)
    {
        //     | a.x      a.y     a.z   |
        // det | b.x      b.y     b.z   |
        //     | c.x      c.y     c.z   |

        return a.x * (b.y * c.z - c.y * b.z)
              -b.x * (a.y * c.z - c.y * a.z)
              +c.x * (a.y * b.z - b.y * a.z);
    }

    // --------------------------------------------------------------
    // get the orientation of the triangle formed by the three points
    // by looking at the determinate of 3x3 matrix formed by them
    // (kind of similar to the cross product of a 2 vector into 3 space
    float circle_winding_det(SFloat2 const &a, SFloat2 const &b, SFloat2 const &c)
    {
        //     | 1      a.x     a.y   |
        // det | 1      b.x     b.y   |
        //     | 1      c.x     c.y   |

        return (b.x * c.y - c.x * b.y ) -
               (a.x * c.y - c.x * a.y ) +
               (a.x * b.y - b.x * a.y );
    }

    // --------------------------------------------------------------
    /// q lies in the circle (a,b,c) only if q+ lies below the plane
    float circle_test_det(SFloat2 const &a, SFloat2 const &b, SFloat2 const &c, SFloat2 const &q)
    {
        //     | 1      a.x     a.y     (a.x*a.x + a.y*a.y)  |
        // det | 1      b.x     b.y     (b.x*b.x + b.y*b.y)  |
        //     | 1      c.x     c.y     (c.x*c.x + c.y*c.y)  |
        //     | 1      q.x     q.y     (q.x*q.x + q.y*q.y)  |

        float const a2 = a.x * a.x + a.y * a.y;
        float const b2 = b.x * b.x + b.y * b.y;
        float const c2 = c.x * c.x + c.y * c.y;
        float const q2 = q.x * q.x + q.y * q.y;

        float const d0 = det3(SFloat3(b.x, b.y, b2),
                              SFloat3(c.x, c.y, c2),
                              SFloat3(q.x, q.y, q2));

        float const d1 = det3(SFloat3(a.x, a.y, a2),
                              SFloat3(c.x, c.y, c2),
                              SFloat3(q.x, q.y, q2));

        float const d2 = det3(SFloat3(a.x, a.y, a2),
                              SFloat3(b.x, b.y, b2),
                              SFloat3(q.x, q.y, q2));

        float const d3 = det3(SFloat3(a.x, a.y, a2),
                              SFloat3(b.x, b.y, b2),
                              SFloat3(c.x, c.y, c2));

        return (d0 - d1 + d2 - d3);
    }

    // --------------------------------------------------------------

    bool is_in_circle(SFloat2 const &a, SFloat2 const &b, SFloat2 const &c, SFloat2 const &q)
    {
        float const detWinding = circle_winding_det(a,b,c);
        float const detQuery   = circle_test_det(a,b,c,q);

        // q lies in the circle only if these two determinants have opposite signs
        return (detWinding * detQuery) < 0;
    }

    struct SEdge
    {
        explicit SEdge(uint32_t const v0, uint32_t const v1)
            : mV0(v0 < v1 ? v0 : v1)
            , mV1(v0 > v1 ? v0 : v1)
        {
        }

        bool operator < (SEdge const &other) const
        {
            if (mV0 == other.mV0)
            {
                return mV1 < other.mV1;
            }

            return mV0 < other.mV0;
        }

        uint32_t mV0;
        uint32_t mV1;
    };

    struct SEdgeMeta
    {
        explicit SEdgeMeta(SEdge const &e, bool const marked)
            : mEdge(e)
            , mMarked(marked)
        {
        }

        SEdge   mEdge;
        bool    mMarked;
    };

    static constexpr uint32_t const skInvalidTriIndex = ~0u;

    struct STriPair
    {
        explicit STriPair()
            : mTri0(skInvalidTriIndex)
            , mTri1(skInvalidTriIndex)
        {
        }

        void Accumulate(uint32_t const tri)
        {
            if (mTri0 == skInvalidTriIndex)
            {
                mTri0 = tri;
            }
            else if (mTri1 == skInvalidTriIndex)
            {
                mTri1 = tri;
            }
            else
            {
                assert(false && "too many triangles!");
            }
        }

        bool HasPairedTriangles() const
        {
            return (mTri0 != skInvalidTriIndex) && (mTri1 != skInvalidTriIndex);
        }

        uint32_t OtherTriangle(uint32_t const triIdx) const
        {
            assert(((triIdx == mTri0) || (triIdx == mTri1)) && "doesn't match either tri");
            return (mTri0 == triIdx) ? mTri1 : mTri0;
        }

        uint32_t mTri0;
        uint32_t mTri1;
    };

    typedef std::map<SEdge, STriPair> TEdgeMap;

    uint32_t get_index_not_on_edge(uint32_t const * const pTriIndices, SEdge const &edge)
    {
        for (int i = 0; i < 3; ++i)
        {
            if ( (pTriIndices[i] != edge.mV0) && (pTriIndices[i] != edge.mV1) )
            {
                return pTriIndices[i];
            }
        }

        assert(false && "failed to find unique index");
        return skInvalidTriIndex;
    }

    class CPositionAccess
    {
    public:
        explicit CPositionAccess(void const *pPositions, uint32_t const positionStride)
            : mpPositions8(reinterpret_cast<uint8_t const*>(pPositions))
            , mPositionStride(positionStride)
        {
        }

        SFloat2 const * Get(uint32_t const idx) const
        {
            return reinterpret_cast<SFloat2 const*>(mpPositions8 + idx * mPositionStride);
        }

    private:
        uint8_t const * mpPositions8;
        uint32_t        mPositionStride;
    };

    bool has_correct_winding(uint32_t const *pTriIndices, CPositionAccess const &positions)
    {
        SFloat2 const * const pPos0 = positions.Get(pTriIndices[0]);
        SFloat2 const * const pPos1 = positions.Get(pTriIndices[1]);
        SFloat2 const * const pPos2 = positions.Get(pTriIndices[2]);

        return circle_winding_det(*pPos0, *pPos1, *pPos2) >= 0;
    }

    bool doit(uint32_t *pTriIndices, uint32_t const indexCount, void const * pPositions, uint32_t const positionStride)
    {
        assert((indexCount % 3) == 0 && "invalid index count");
        CPositionAccess positions(pPositions, positionStride);

        // ensure the triangles have a consistent winding
        for (uint32_t idx = 0; idx < indexCount; idx += 3)
        {
            if (!has_correct_winding(pTriIndices+idx, positions))
            {
                std::swap(pTriIndices[idx+1], pTriIndices[idx+2]);
            }
        }

        TEdgeMap edgeMap;
        for (uint32_t idx = 0; idx < indexCount; idx += 3)
        {
            uint32_t const i0 = pTriIndices[idx+0];
            uint32_t const i1 = pTriIndices[idx+1];
            uint32_t const i2 = pTriIndices[idx+2];

            SEdge const e0(i0, i1);
            SEdge const e1(i1, i2);
            SEdge const e2(i2, i0);

            uint32_t const triIdx = idx/3;
            edgeMap[e0].Accumulate(triIdx);
            edgeMap[e1].Accumulate(triIdx);
            edgeMap[e2].Accumulate(triIdx);
        }

        // remove any edge mappings that don't have paired triangles
        for (auto itr = edgeMap.begin(); itr != edgeMap.end(); /*in loop*/)
        {
            if (!itr->second.HasPairedTriangles())
            {
                itr = edgeMap.erase(itr);
            }
            else
            {
                ++itr;
            }
        }

        std::vector<SEdgeMeta> edges;
        edges.reserve(edgeMap.size());
        for (auto itr = edgeMap.begin(); itr != edgeMap.end(); ++itr)
        {
            edges.push_back(SEdgeMeta(itr->first, true /*marked*/));
        }

        std::vector<uint32_t> edgesToProcess;
        edgesToProcess.reserve(edges.size());
        for (uint32_t i = 0; i < edges.size(); ++i)
        {
            edgesToProcess.push_back(i);
        }

        while (!edgesToProcess.empty())
        {
            SEdgeMeta *pMeta = &edges[edgesToProcess.back()];
            edgesToProcess.pop_back();

            // check if this edge is local Delaunay
            SEdge const &e = pMeta->mEdge;
            
            auto const edgeMapItr = edgeMap.find(e);
            assert((edgeMapItr != edgeMap.end()) && "invalid edge");
            STriPair const &triPair = edgeMapItr->second;
            assert(((triPair.mTri0 != skInvalidTriIndex) && (triPair.mTri1 != skInvalidTriIndex)) && "invalid tri pair");
            
            uint32_t * pTriIndices0 = pTriIndices + triPair.mTri0 * 3;
            uint32_t * pTriIndices1 = pTriIndices + triPair.mTri1 * 3;

            uint32_t const freeIndex0 = get_index_not_on_edge(pTriIndices0, e);
            uint32_t const freeIndex1 = get_index_not_on_edge(pTriIndices1, e);

            SFloat2 const * const pFree0Pos = positions.Get(freeIndex0);
            SFloat2 const * const pFree1Pos = positions.Get(freeIndex1);
            SFloat2 const * const pEdge0Pos = positions.Get(e.mV0);
            SFloat2 const * const pEdge1Pos = positions.Get(e.mV1);

            bool const isFree1PosInCircle = is_in_circle(*pFree0Pos, *pEdge0Pos, *pEdge1Pos, *pFree1Pos);
            if (isFree1PosInCircle)
            {
                // flip the triangle

                uint32_t newTri0 [] = {freeIndex0, freeIndex1, e.mV0};
                if (!has_correct_winding(newTri0, positions))
                {
                    std::swap(newTri0[1], newTri0[2]);
                }

                uint32_t newTri1 [] = {freeIndex0, freeIndex1, e.mV1};
                if (!has_correct_winding(newTri1, positions))
                {
                    std::swap(newTri1[1], newTri1[2]);
                }

                memcpy(pTriIndices0, newTri0, sizeof(uint32_t) * 3);
                memcpy(pTriIndices1, newTri1, sizeof(uint32_t) * 3);

                // THIS PROBABLY WILL BREAK ALL THE OTHER NEIGHBOR TRIANGLES!!!!!

                /*edgeMap[SEdge(newTri


                SEdge const newTri0_edge0(freeIndex0, e.mV1);
                SEdge const newTri0_edge1(e.mV0, e.mV0);
                SEdge const newTri0_edge2(freeIndex0, e.mV0);*/


                

            }
        }

        return true;
    }

    // --------------------------------------------------------------
}