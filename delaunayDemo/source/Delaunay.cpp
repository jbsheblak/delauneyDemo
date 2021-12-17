

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

    struct SFloat2 { float x, y; };

    // --------------------------------------------------------------

    struct SFloat3
    {
        explicit SFloat3() = default;
        explicit SFloat3 (float _x, float _y, float _z) : x(_x) , y(_y) , z(_z) {}
        float x,y,z;
    };

    // --------------------------------------------------------------
    // 3x3 matrix determinant
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
    // check to see if 'q' is in the circle whose perimeter that touches (a,b,c)
    bool is_in_circle(SFloat2 const &a, SFloat2 const &b, SFloat2 const &c, SFloat2 const &q)
    {
        float const detWinding = circle_winding_det(a,b,c);
        float const detQuery   = circle_test_det(a,b,c,q);

        // q lies in the circle only if these two determinants have opposite signs
        return (detWinding * detQuery) < 0;
    }

    // --------------------------------------------------------------

    static constexpr uint32_t const skInvalidIndex = ~0u;

    // ==============================================================
    // SEdge
    // ==============================================================
    // represents a single edge in our triangle mesh.
    // always stored such that lower index is in v0 and higher index is in v1
    struct SEdge
    {
        explicit SEdge()
            : mV0(skInvalidIndex)
            , mV1(skInvalidIndex)
        {
        }

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

        bool operator == (SEdge const &other) const
        {
            return (mV0 == other.mV0) && (mV1 == other.mV1);
        }

        bool operator != (SEdge const &other) const
        {
            return !(*this == other);
        }

        uint32_t mV0;   ///< index of first vertex
        uint32_t mV1;   ///< index of second vertex
    };

    // ==============================================================
    // STriPair
    // ==============================================================
    // a pair of triangles that share an edge

    struct STriPair
    {
        explicit STriPair()
            : mTri0(skInvalidIndex)
            , mTri1(skInvalidIndex)
        {
        }

        // add the triangle to the pairing
        void Accumulate(uint32_t const tri)
        {
            if (mTri0 == skInvalidIndex)
            {
                mTri0 = tri;
            }
            else if (mTri1 == skInvalidIndex)
            {
                mTri1 = tri;
            }
            else
            {
                assert(false && "too many triangles!");
            }
        }

        // check if we have two paired triangles
        bool HasPairedTriangles() const
        {
            return (mTri0 != skInvalidIndex) && (mTri1 != skInvalidIndex);
        }

        // if our tri matches the oldIdx, replace it with the newIdx
        void ReplaceTri(uint32_t const oldIdx, uint32_t const newIdx)
        {
            if (mTri0 == oldIdx)
            {
                mTri0 = newIdx;
            }

            if (mTri1 == oldIdx)
            {
                mTri1 = newIdx;
            }
        }

        uint32_t mTri0; ///< index of the first triangle
        uint32_t mTri1; ///< index of the second triangle
    };

    typedef std::map<SEdge, STriPair> TEdgeTriMap;
    typedef std::map<SEdge, bool>     TEdgeMarkedMap;

    // ==============================================================
    // CTriEdges
    // ==============================================================
    // a set of edges formed from a triangle

    class CTriEdges
    {
    public:
        explicit CTriEdges(uint32_t const *pTriangleIndices)            
        {
            mEdges[0] = SEdge(pTriangleIndices[0], pTriangleIndices[1]);
            mEdges[1] = SEdge(pTriangleIndices[1], pTriangleIndices[2]);
            mEdges[2] = SEdge(pTriangleIndices[2], pTriangleIndices[0]);
        }

        bool HasEdge(SEdge const &e) const
        {
            for (SEdge const &ourEdge : mEdges)
            {
                if (e == ourEdge)
                {
                    return true;
                }   
            }

            return false;
        }

        SEdge mEdges[3];
    };

    // --------------------------------------------------------------
    // get the index of the triangle that is NOT included in the edge
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
        return skInvalidIndex;
    }

    // ==============================================================
    // CPositionAccess
    // ==============================================================
    // utility class for accessing positions from indices

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
            return reinterpret_cast<SFloat2 const*>(mpPositions8 + intptr_t(idx) * mPositionStride);
        }

    private:
        uint8_t const * mpPositions8;
        uint32_t        mPositionStride;
    };

    // --------------------------------------------------------------
    // check to see if the specified triangle has the proper winding
    bool has_correct_winding(uint32_t const *pTriIndices, CPositionAccess const &positions)
    {
        SFloat2 const * const pPos0 = positions.Get(pTriIndices[0]);
        SFloat2 const * const pPos1 = positions.Get(pTriIndices[1]);
        SFloat2 const * const pPos2 = positions.Get(pTriIndices[2]);

        return circle_winding_det(*pPos0, *pPos1, *pPos2) >= 0;
    }

    // --------------------------------------------------------------

    bool retriangulate_with_edge_flip(uint32_t * const pTriIndices, uint32_t const indexCount, void const * const pPositions, uint32_t const positionStride)
    {
        assert(((pTriIndices != nullptr) || (indexCount == 0)) && "invalid tri indices");
        assert(((pPositions != nullptr) && (positionStride > 0)) && "invalid position and stride");
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

        TEdgeTriMap edgeTriMap;
        TEdgeMarkedMap edgeMarkedMap;

        // build up the edge triangle mappings
        for (uint32_t idx = 0; idx < indexCount; idx += 3)
        {
            uint32_t const i0 = pTriIndices[idx+0];
            uint32_t const i1 = pTriIndices[idx+1];
            uint32_t const i2 = pTriIndices[idx+2];

            SEdge const e0(i0, i1);
            SEdge const e1(i1, i2);
            SEdge const e2(i2, i0);

            uint32_t const triIdx = idx/3;
            edgeTriMap[e0].Accumulate(triIdx);
            edgeTriMap[e1].Accumulate(triIdx);
            edgeTriMap[e2].Accumulate(triIdx);
        }

        // remove any edge mappings that don't have paired triangles.
        // these are border edges and we don't need them.
        for (auto itr = edgeTriMap.begin(); itr != edgeTriMap.end(); /*in loop*/)
        {
            if (!itr->second.HasPairedTriangles())
            {
                itr = edgeTriMap.erase(itr);
            }
            else
            {
                ++itr;
            }
        }

        // push all of the known edges into the 'to-process' bin and mark them
        typedef std::vector<SEdge const*> TEdgeProcessVec;
        TEdgeProcessVec edgesToProcess;
        edgesToProcess.reserve(edgeTriMap.size());
        for (auto itr = edgeTriMap.begin(); itr != edgeTriMap.end(); ++itr)
        {
            edgesToProcess.push_back(&itr->first);
            edgeMarkedMap[itr->first] = true;
        }

        // process edges until we hit them all
        while (!edgesToProcess.empty())
        {
            // pop edge from stack and unmark
            SEdge const &e = *edgesToProcess.back();
            edgesToProcess.pop_back();
            edgeMarkedMap[e] = false;

            // check if this edge is local Delaunay
            auto const edgeMapItr = edgeTriMap.find(e);
            assert((edgeMapItr != edgeTriMap.end()) && "invalid edge");
            STriPair const &triPair = edgeMapItr->second;
            assert(((triPair.mTri0 != skInvalidIndex) && (triPair.mTri1 != skInvalidIndex)) && "invalid tri pair");
            
            uint32_t * const pTriIndices0 = pTriIndices + intptr_t(triPair.mTri0) * 3;
            uint32_t * const pTriIndices1 = pTriIndices + intptr_t(triPair.mTri1) * 3;

            uint32_t const freeIndex0 = get_index_not_on_edge(pTriIndices0, e);
            uint32_t const freeIndex1 = get_index_not_on_edge(pTriIndices1, e);

            SFloat2 const * const pFree0Pos = positions.Get(freeIndex0);
            SFloat2 const * const pFree1Pos = positions.Get(freeIndex1);
            SFloat2 const * const pEdge0Pos = positions.Get(e.mV0);
            SFloat2 const * const pEdge1Pos = positions.Get(e.mV1);

            if (is_in_circle(*pFree0Pos, *pEdge0Pos, *pEdge1Pos, *pFree1Pos))
            {
                // flip the triangle

                CTriEdges const priorTriangle0(pTriIndices0);
                CTriEdges const priorTriangle1(pTriIndices1);

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

                // add the flipped edge to the triangle map
                SEdge const flippedEdge(freeIndex0, freeIndex1);
                {   
                    edgeTriMap[flippedEdge].Accumulate(triPair.mTri0);
                    edgeTriMap[flippedEdge].Accumulate(triPair.mTri1);
                    edgeMarkedMap[flippedEdge] = true;
                }

                CTriEdges const newTriangle0(newTri0);
                CTriEdges const newTriangle1(newTri1);

                // our flipped edge will cause new edge associations.
                // update the edge mappings and which edges to process
                {
                    auto const update_edges = [](TEdgeTriMap &edgeTriMap,
                                                 TEdgeMarkedMap &edgeMarkedMap,
                                                 TEdgeProcessVec &edgesToProcess,
                                                 SEdge const &edge,
                                                 uint32_t const priorTriIdx,
                                                 uint32_t const newTriIdx)
                    {
                        // look for the edge in the edge tri map
                        // if it doesn't exist, that means it was a border edge and we don't have to worry about it
                        auto const edgeMapItr = edgeTriMap.find(edge);
                        if (edgeMapItr != edgeTriMap.end())
                        {
                            // replace the old triIdx with the new triIdx
                            STriPair &triPair = edgeMapItr->second;
                            triPair.ReplaceTri(priorTriIdx, newTriIdx);

                            // if this edge hasn't already been marked, do that now and add to process list
                            auto markItr = edgeMarkedMap.find(edgeMapItr->first);
                            assert((markItr != edgeMarkedMap.end()) && "invalid edge mark itr");

                            // if unmarked, push onto sack and mark it
                            if (!markItr->second)
                            {
                                edgesToProcess.push_back(&edgeMapItr->first);
                                markItr->second = true;
                            }
                        }
                    };

                    for (SEdge const &edge : newTriangle0.mEdges)
                    {
                        if (!priorTriangle0.HasEdge(edge))
                        {
                            if (edge != flippedEdge)
                            {
                                update_edges(edgeTriMap, edgeMarkedMap, edgesToProcess, edge, triPair.mTri1 /*prior*/, triPair.mTri0 /*new*/);
                            }
                        }
                    }

                    for (SEdge const &edge : newTriangle1.mEdges)
                    {
                        if (!priorTriangle1.HasEdge(edge))
                        {
                            if (edge != flippedEdge)
                            {
                                update_edges(edgeTriMap, edgeMarkedMap, edgesToProcess, edge, triPair.mTri0 /*prior*/, triPair.mTri1 /*new*/);
                            }
                        }
                    }
                }

                // convert the triangle indices to the new indices
                memcpy(pTriIndices0, newTri0, sizeof(uint32_t) * 3);
                memcpy(pTriIndices1, newTri1, sizeof(uint32_t) * 3);
            }
        }

        return true;
    }

    // --------------------------------------------------------------
}