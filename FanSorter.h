/****************************************************************************
 *
 * class FanSorter
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _FANSORTER_H_
#define _FANSORTER_H_

#include "PluginTypes.h"
#include "PwpFile.h"


//***************************************************************************
//***************************************************************************
//***************************************************************************

class FanCell {
public:
    FanCell(PWP_UINT32 cellNdx = PWP_UINT32_UNDEF, PWP_UINT32 *indices = 0) :
        cellNdx_(cellNdx)
    {
        setIndices(indices);
    }


    ~FanCell(){}


    void
    setIndices(PWP_UINT32 *indices)
    {
        if (0 != indices) {
            indices_[0] = indices[0];
            indices_[1] = indices[1];
            indices_[2] = indices[2];
        }
        else {
            indices_[0] = indices_[1] = indices_[2] = PWP_UINT32_UNDEF;
        }
    }


    inline Edge
    edge(PWP_UINT32 ndx, bool fwd = true) const
    {
        Edge ret;
        switch(ndx) {
        case 0:
            ret[0] = indices_[fwd ? 0 : 1];
            ret[1] = indices_[fwd ? 1 : 0];
            break;
        case 1:
            ret[0] = indices_[fwd ? 1 : 2];
            ret[1] = indices_[fwd ? 2 : 1];
            break;
        case 2:
            ret[0] = indices_[fwd ? 2 : 0];
            ret[1] = indices_[fwd ? 0 : 2];
            break;
        }
        return ret;
    }


    inline Edge
    leftEdge(bool fwd = true) const
    {
        return edge(2, fwd);
    }


    inline Edge
    rightEdge(bool fwd = true) const
    {
        return edge(0, fwd);
    }

    PWP_UINT32 cellNdx_;
    PWP_UINT32 indices_[3];
};
typedef std::vector<FanCell>    FanCellArray1;


//***************************************************************************
//***************************************************************************
//***************************************************************************

class FanSorter {
public:

    FanSorter(PwpFile &dumpFile, const EdgeToUInt32Map &hardGceEdgeToDualVert);
    ~FanSorter();

    void        run(CaeUnsGridModel &model, PWP_UINT32 gceVertNdx,
                    const UInt32Array1 &fanCells, UInt32Array2 &fans);


private:

    bool    findHardEdge(Edge edge, EdgeToUInt32Map::const_iterator &it);

    bool    run2(FanCellArray1 &fanCells, UInt32Array1 &runLength);

    bool    sortFanCellRange(FanCellArray1 &fanCells,
                FanCellArray1::iterator itBegin,
                FanCellArray1::iterator &itRngRight,
                FanCellArray1::iterator &itRngLeft);


private:
    PwpFile &               dumpFile_;
    const EdgeToUInt32Map & hardGceEdgeToDualVert_;
};

#endif
