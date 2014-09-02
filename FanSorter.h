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


class FanSorter {
public:

    FanSorter(PwpFile &dumpFile);
    ~FanSorter();

    void    sort(CaeUnsGridModel &model, PWP_UINT32 origVertNdx,
                UInt32Array1 &fanCells, EdgeToUInt32Map &edgeToVertex);


private:

    void    walkLeft(PWP_UINT32 cellNdx, UInt32Array1::iterator &itCell,
                bool &isClosed);

    void    walkRight(PWP_UINT32 cellNdx, UInt32Array1::iterator &itCell,
                bool &isClosed);


private:
    PwpFile &           dumpFile_;
    EdgeToUInt32Map     ecMap_;
    UInt32ToEdgePair    cellFanEdges_;
};

#endif
