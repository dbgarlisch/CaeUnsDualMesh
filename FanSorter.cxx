/****************************************************************************
 *
 * class FanSorter
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

//#include <algorithm>

#include "apiPWP.h"
#include "CaeUnsGridModel.h"
#include "FanSorter.h"
#include "PluginTypes.h"
#include "PwpFile.h"


FanSorter::FanSorter(PwpFile &dumpFile,
        const EdgeToUInt32Map &hardGceEdgeToDualVert,
        const UInt32ToUInt32Map &hardGceVertToDualVert) :
    dumpFile_(dumpFile),
    hardGceEdgeToDualVert_(hardGceEdgeToDualVert),
    hardGceVertToDualVert_(hardGceVertToDualVert)
{
}


FanSorter::~FanSorter()
{
}


void
FanSorter::run(CaeUnsGridModel &model, PWP_UINT32 gceVertNdx,
    const UInt32Array1 &fanCells, UInt32Array2 &fans)
{
    if (dumpFile_.isOpen()) {
        dumpFile_.write(gceVertNdx, "\n", "\n# FanSorter::run gceVertNdx=");
    }
    /*  fanCells is in an unspecified order. Each cell has a right-handed
        winding order that contains gceVertNdx. For example, cellD = (5,4,0).
        We need to arrange cells around gceVertNdx into a right-handed
        cell-fan D-C-B-A. If gceVertNdx is a boundary vertex, the cell-fan
        arrangement will be open, similar to diagram below. If gceVertNdx is an
        interior vertex, the cell-fan will be closed.

                2----3----4
               / \ B | C / \       0 = gceVertNdx
              /   \  |  /   \      edge(0,3) = cellB's right fan-edge
             /  A  \ | /  D  \     edge(3,0) = cellC's left fan-edge
            /       \|/       \    cellA     = (0,5,4)
           1---------0---------5   
    */
    PWGM_ELEMDATA ed;
    FanCellArray1 fanCellArr;
    fanCellArr.reserve(fanCells.size());
    UInt32Array1::const_iterator itNdx = fanCells.begin();
    for (; itNdx != fanCells.end(); ++itNdx) {
        const PWP_UINT32 cellNdx = *itNdx;
        if (!CaeUnsElement(model, cellNdx).data(ed)) {
            // very bad! exception?
            continue;
        }
        // Load fanCellArr for processing below
        for (PWP_UINT32 ii = 0; ii < ed.vertCnt; ++ii) {
            if (ed.index[ii] == gceVertNdx) {
                // rotate cell vertices to make gceVertNdx first
                std::rotate(ed.index, ed.index + ii, ed.index + ed.vertCnt);
                fanCellArr.push_back(FanCell(cellNdx, ed.index));
                // all done with this fan cell
                break;
            }
        }
        if (dumpFile_.isOpen()) {
            const FanCell &c = fanCellArr.back();
            dumpFile_.write(c.cellNdx_, " { ", "#    cell ndx=");
            for (PWP_UINT32 ii = 0; ii < 3; ++ii) {
                dumpFile_.write(c.indices_[ii], " ");
            }
            Edge e = c.leftEdge();
            dumpFile_.write(e[0], " ", " } / leftEdge { ");
            dumpFile_.write(e[1], " }  rightEdge { ");
            e = c.rightEdge();
            dumpFile_.write(e[0], " ");
            dumpFile_.write(e[1], " }\n");
        }
    }

    // Sort the cells by walking their right edges.
    UInt32Array1 runLength;
    bool isClosed = run2(fanCellArr, runLength);

    // If gceVertNdx was exported, we need to include it in the polygon 
    PWP_UINT32 gceVertDualNdx = 0;
    bool includeGceVertNdx = false;
    UInt32ToUInt32Map::const_iterator it =
        hardGceVertToDualVert_.find(gceVertNdx);
    if (hardGceVertToDualVert_.end() != it) {
        includeGceVertNdx = true;
        gceVertDualNdx = it->second;
    }

    // build return array
    EdgeToUInt32Map::const_iterator itEdgeVert;
    FanCellArray1::const_iterator itFanCell = fanCellArr.begin();
    FanCellArray1::const_iterator itLastFanCell;
    UInt32Array1::iterator itRunLength = runLength.begin();
    for (; itRunLength != runLength.end(); ++itRunLength) {
        UInt32Array1 fan;
        if (!isClosed) {
            // cacheThis for below
            itLastFanCell = itFanCell + (*itRunLength - 1);
            // add right hard edge vertex
            if (findHardEdge(itFanCell->rightEdge(), itEdgeVert)) {
                fan.push_back(itEdgeVert->second);
            }
            else {
                fail("Could not find right hard edge");
            }
        }
        // Add cell centroid indices
        for (PWP_UINT32 n = 0; n < *itRunLength; ++n) {
            fan.push_back(itFanCell->cellNdx_);
            ++itFanCell;
        }
        if (!isClosed) {
            // add left hard edge vertex
            if (findHardEdge(itLastFanCell->leftEdge(), itEdgeVert)) {
                fan.push_back(itEdgeVert->second);
            }
            else {
                fail("Could not find left hard edge");
            }
            if (includeGceVertNdx) {
                fan.push_back(gceVertDualNdx);
            }
            *itRunLength = PWP_UINT32(fan.size());
        }
        fans.push_back(fan);
    }
}


bool
FanSorter::findHardEdge(Edge edge, EdgeToUInt32Map::const_iterator &it)
{
    it = hardGceEdgeToDualVert_.find(edge);
    if (hardGceEdgeToDualVert_.end() == it) {
        std::swap(edge[0], edge[1]);
        it = hardGceEdgeToDualVert_.find(edge);
    }
    return hardGceEdgeToDualVert_.end() != it;
}


bool
FanSorter::run2(FanCellArray1 &fanCells, UInt32Array1 &runLength)
{
    bool ret = false;
    runLength.clear();
    FanCellArray1::iterator itRngRight;
    FanCellArray1::iterator itRngLeft;
    FanCellArray1::iterator itBegin = fanCells.begin();
    while (fanCells.end() != itBegin) {
        ret = sortFanCellRange(fanCells, itBegin, itRngRight, itRngLeft);
        runLength.push_back(PWP_UINT32(std::distance(itRngRight, itRngLeft)));
        itBegin = itRngLeft;
    }
    return ret;
}


bool
FanSorter::sortFanCellRange(FanCellArray1 &fanCells,
    FanCellArray1::iterator itBegin, FanCellArray1::iterator &itRngRight,
    FanCellArray1::iterator &itRngLeft)
{
    bool ret = false;
    // Init range to only include itBegin.
    itRngRight = itBegin;
    itRngLeft = itBegin + 1;
    FanCellArray1::iterator itPivot = itRngRight;
    bool foundNext = true;
    FanCellArray1::iterator itCandidate = itRngLeft;
    EdgeToUInt32Map::const_iterator itHardEdges = hardGceEdgeToDualVert_.end();
    while ((fanCells.end() != itCandidate) && foundNext) {
        foundNext = false;
        while (fanCells.end() != itCandidate) {
            if (itHardEdges !=
                    hardGceEdgeToDualVert_.find(itPivot->leftEdge())) {
                // left edge is hard, can't walk across it!
                break;
            }
            // Is itPivot's left edge == reverse of candidate's right edge?
            if (itPivot->leftEdge() == itCandidate->rightEdge(false)) {
                // Make itCandidate the next leftmost FanCell in range.
                if (itRngLeft != itCandidate) {
                    std::swap(*itRngLeft, *itCandidate);
                }
                // Make itPivot ref the candidate just added. We want to match
                // to this one now.
                itPivot = itRngLeft;
                // Reset itCandidate for next pass
                itCandidate = ++itRngLeft;
                foundNext = true;
                break;
            }
            // Did not find match, try next FanCell
            ++itCandidate;
        }
    }
    // foundNext will only be true if all FanCells matched above. Is fan closed?
    if (foundNext) {
        ret = (itPivot->leftEdge() == itRngRight->rightEdge(false));
    }
    // Match remaining FanCells against right edge
    foundNext = true;
    itPivot = itRngRight;
    itCandidate = itRngLeft;
    while ((fanCells.end() != itCandidate) && foundNext) {
        foundNext = false;
        while (fanCells.end() != itCandidate) {
            Edge pivotRightEdge(itPivot->rightEdge());
            if (itHardEdges != hardGceEdgeToDualVert_.find(pivotRightEdge)) {
                // right edge is hard, can't walk across it!
                break;
            }
            // Is right edge of range's rightmost fanCell == reverse of
            // candidate's left edge?
            if (pivotRightEdge == itCandidate->leftEdge(false)) {
                // Edges match! Make itCandidate the start of range.
                FanCell tmp = *itCandidate;
                fanCells.erase(itCandidate);
                // Insert tmp before itPivot and make itPivot ref tmp
                itPivot = fanCells.insert(itPivot, tmp);
                // range shifted to left one more FanCell
                itCandidate = ++itRngLeft;
                foundNext = true;
                break;
            }
            ++itCandidate;
        }
    }
    return ret;
}
