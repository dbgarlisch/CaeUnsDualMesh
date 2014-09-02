/****************************************************************************
 *
 * Pointwise Plugin utility functions
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _RTCAEPSUPPORTDATA_H_
#define _RTCAEPSUPPORTDATA_H_

/*! \cond */

/*------------------------------------*/
/* CaeUnsDualMesh format item setup data */
/*------------------------------------*/
CAEP_BCINFO CaeUnsDualMeshBCInfo[] = {
    { "inlet",      1 },
    { "outlet",     2 },
    { "wall",       3 },
};
/*------------------------------------*/
CAEP_VCINFO CaeUnsDualMeshVCInfo[] = {
    { "fluid",  11 },
    { "plasma", 12 },
    { "solid",  13 },
};
/*------------------------------------*/
const char *CaeUnsDualMeshFileExt[] = {
    "glf"
};

/*! \endcond */

#endif /* _RTCAEPSUPPORTDATA_H_ */
