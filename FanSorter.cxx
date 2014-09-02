/****************************************************************************
 *
 * class FanSorter
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#include <algorithm>

#include "apiPWP.h"
#include "CaeUnsGridModel.h"
#include "FanSorter.h"
#include "PluginTypes.h"
#include "PwpFile.h"


FanSorter::FanSorter(PwpFile &dumpFile) :
    dumpFile_(dumpFile),
    ecMap_(),
    cellFanEdges_()
{
}


FanSorter::~FanSorter()
{
}


// macros to make code more readable
#define itNextCellNdx       itNext->second
#define cellLeftEdge(it)    it->second.first
#define cellRightEdge(it)   it->second.second

#if defined(WINDOWS) && _MSC_VER < 1600
#pragma message ("Using compatibility functions for VS2008.")

namespace std {

    template<class IType1, class IType2>
    IType2 move_backward(IType1 first, IType1 last, IType2 result)
    {
        while (last != first) {
            *(--result) = *(--last);
        }
        return result;
    }
}

#endif


void
FanSorter::sort(CaeUnsGridModel &model, PWP_UINT32 origVertNdx,
    UInt32Array1 &fanCells, EdgeToUInt32Map &edgeToVertex)
{
    if (dumpFile_.isOpen()) {
        dumpFile_.write(origVertNdx, "\n", "\n# FanSorter::sort origVertNdx=");
    }
    /*
        fanCells is in an unspecified order. Each cell has a right-handed
        winding order that contains origVertNdx. For example, cellD = (5,4,0).
        We need to arrange cells around origVertNdx into a right-handed
        cell-fan D-C-B-A. If origVertNdx is a boundary vertex, the cell-fan
        arrangement will be open, similar to diagram below. If origVertNdx is an
        interior vertex, the cell-fan will be closed.

                2----3----4
               / \ B | C / \       0 = origVertNdx
              /   \  |  /   \      edge(0,3) = cellB's right fan-edge
             /  A  \ | /  D  \     edge(3,0) = cellC's left fan-edge
            /       \|/       \    cellA     = (0,5,4)
           1---------0---------5   
    */
    PWGM_ELEMDATA ed;
    EdgeToUInt32Map::value_type ecmVal;
    EdgePair edgePair;
    // Build EdgeToUInt32Map. This will be used to radially order the cells about
    // origVertNdx.
    UInt32Array1::const_iterator itNdx = fanCells.begin();
    for (; itNdx != fanCells.end(); ++itNdx) {
        const PWP_UINT32 cellNdx = *itNdx;
        // Preload map value. cellNdx will not change for rest of loop.
        ecmVal.second = cellNdx;
        if (!CaeUnsElement(model, cellNdx).data(ed)) {
            // very bad! exception?
            continue;
        }
        for (PWP_UINT32 ii = 0; ii < ed.vertCnt; ++ii) {
            if (ed.index[ii] == origVertNdx) {
                // rotate vertices to make origVertNdx first
                std::rotate(ed.index, ed.index + ii, ed.index + ed.vertCnt);
                // ecmVal.first is const! Cast to a non-const ref so we can
                // update the working edge data before adding it to ecMap_.
                // This approach is assumed to be more efficient than
                // constructing EdgeToUInt32Map::value_type temp vals for every
                // call to insert().
                Edge &ecmEdge = const_cast<Edge&>(ecmVal.first);
                // Cell edges stored in ecMap_ in reverse direction because that
                // is the direction for which the neighbor cells will be
                // searching.

                // Reverse edge from origVertNdx to ii+1 vert index.
                ecmEdge[0] = ed.index[1];
                ecmEdge[1] = origVertNdx;
                ecMap_.insert(ecmVal);
                // store cell's right fan-edge (forward direction).
                edgePair.second[0] = ecmEdge[1];
                edgePair.second[1] = ecmEdge[0];

                // Reverse edge from last vert index to origVertNdx.
                ecmEdge[0] = origVertNdx;
                ecmEdge[1] = ed.index[ed.vertCnt - 1];
                ecMap_.insert(ecmVal);
                // store cell's left fan-edge (forward direction).
                edgePair.first[0] = ecmEdge[1];
                edgePair.first[1] = ecmEdge[0];
                cellFanEdges_.insert(UInt32ToEdgePair::value_type(cellNdx, edgePair));
                break;
            }
        }
        if (dumpFile_.isOpen()) {
            dumpFile_.write(cellNdx, " { ", "#    cell ndx=");
            for (PWP_UINT32 ii = 0; ii < ed.vertCnt; ++ii) {
                dumpFile_.write(ed.index[ii], " ");
            }
            dumpFile_.write(edgePair.first[0], " ", " } / leftEdge { ");
            dumpFile_.write(edgePair.first[1], " }  rightEdge { ");
            dumpFile_.write(edgePair.second[0], " ");
            dumpFile_.write(edgePair.second[1], " }\n");
        }
    }

    if (dumpFile_.isOpen()) {
        dumpFile_.write("# ecMap_\n");
        EdgeToUInt32Map::const_iterator it = ecMap_.begin();
        for (; it != ecMap_.end(); ++it) {
            dumpFile_.write(it->first[0], " ", "#    edge { ");
            dumpFile_.write(it->first[1], " } ");
            dumpFile_.write(it->second, "\n", "cell=");
        }
        dumpFile_.write("# edgeToVertex\n");
        it = edgeToVertex.begin();
        for (; it != edgeToVertex.end(); ++it) {
            dumpFile_.write(it->first[0], " ", "#    edge { ");
            dumpFile_.write(it->first[1], " } ");
            dumpFile_.write(it->second, "\n", "vert=");
        }
    }

    // Sort the cells by walking their right edges.
    bool isClosed = false;
    // Arbitrarily start fan at first cell in fanCells.
    PWP_UINT32 startCellNdx = fanCells.at(0);
    // The majority of fans will be closed (origVertNdx is interior). Walking
    // the left cell-edges first will be the fastest. The over-head of
    // walkRight() is only be need for open fans (origVertNdx is boundary).
    UInt32Array1::iterator itCell = fanCells.begin();
    // walkLeft() will assign fan ordered cell indices starting at ++itCell.
    walkLeft(startCellNdx, ++itCell, isClosed);
    if (!isClosed) {
        // Only need walkRight() if startCellNdx was in the middle of the fan
        if (fanCells.end() != itCell) {
            // Shift left cells to end of array to make room for right cells
            itCell = std::move_backward(fanCells.begin(), itCell, fanCells.end());
            // Fan is not closed. Finish the sort by walking the remaining cells
            // by their right edges.
            walkRight(startCellNdx, itCell, isClosed);
        }
        // The fan does not include the boundary dual vertices
        UInt32ToEdgePair::const_iterator itCellEdgePair;
        EdgeToUInt32Map::const_iterator itEdgeVert;
        // append left-most edge vertex
        itCellEdgePair = cellFanEdges_.find(fanCells.back());
        if (cellFanEdges_.end() != itCellEdgePair) {
            itEdgeVert = edgeToVertex.find(cellLeftEdge(itCellEdgePair));
            if (edgeToVertex.end() != itEdgeVert) {
                fanCells.insert(fanCells.end(), itEdgeVert->second);
            }
        }
        // prepend right-most edge vertex
        itCellEdgePair = cellFanEdges_.find(fanCells.front());
        if (cellFanEdges_.end() != itCellEdgePair) {
            itEdgeVert = edgeToVertex.find(cellRightEdge(itCellEdgePair));
            if (edgeToVertex.end() != itEdgeVert) {
                fanCells.insert(fanCells.begin(), itEdgeVert->second);
            }
        }
    }
}


void
FanSorter::walkLeft(PWP_UINT32 cellNdx,
    UInt32Array1::iterator &itCell, bool &isClosed)
{
    isClosed = false;
    EdgeToUInt32Map::const_iterator itNext;
    UInt32ToEdgePair::const_iterator itCellEdgePair =
        cellFanEdges_.find(cellNdx);
    if (cellFanEdges_.end() != itCellEdgePair) {
        itNext = ecMap_.find(cellLeftEdge(itCellEdgePair));
        while (ecMap_.end() != itNext) {
            if (cellNdx == itNextCellNdx) {
                // We have wrapped back to starting cellNdx!
                isClosed = true;
                break;
            }
            *itCell++ = itNextCellNdx;
            itCellEdgePair = cellFanEdges_.find(itNextCellNdx);
            if (cellFanEdges_.end() == itCellEdgePair) {
                break;
            }
            itNext = ecMap_.find(cellLeftEdge(itCellEdgePair));
        }
    }
}


void
FanSorter::walkRight(PWP_UINT32 cellNdx,
    UInt32Array1::iterator &itCell, bool &isClosed)
{
    isClosed = false;
    EdgeToUInt32Map::const_iterator itNext;
    UInt32ToEdgePair::const_iterator itCellEdgePair = cellFanEdges_.find(cellNdx);
    if (cellFanEdges_.end() != itCellEdgePair) {
        itNext = ecMap_.find(cellRightEdge(itCellEdgePair));
        while (ecMap_.end() != itNext) {
            if (cellNdx == itNextCellNdx) {
                // We have wrapped back to starting cellNdx!
                isClosed = true;
                break;
            }
            *(--itCell) = itNextCellNdx;
            itCellEdgePair = cellFanEdges_.find(itNextCellNdx);
            if (cellFanEdges_.end() == itCellEdgePair) {
                break;
            }
            itNext = ecMap_.find(cellRightEdge(itCellEdgePair));
        }
    }
}

#undef itNextCellNdx
#undef cellLeftEdge
#undef cellRightEdge
