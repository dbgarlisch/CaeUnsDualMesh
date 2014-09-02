/****************************************************************************
 *
 * CAEP Plugin example - PwCaeGridWrite implementation
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

extern "C" {

#include "apiCAEP.h"
#include "apiCAEPUtils.h"
#include "apiGridModel.h"
#include "apiPWP.h"
#include "runtimeWrite.h"
#include "pwpPlatform.h"

} /* extern "C" */

#include "CaeUnsDualMesh.h"


/**************************************/
PWP_BOOL
runtimeWrite(CAEP_RTITEM *pRti, PWGM_HGRIDMODEL model,
    const CAEP_WRITEINFO *pWriteInfo)
{
    CaeUnsDualMesh plugin(pRti, model, pWriteInfo);
    return plugin.run();
}


PWP_BOOL
runtimeCreate(CAEP_RTITEM *pRti)
{
    return CaeUnsDualMesh::create(*pRti);
}


PWP_VOID
runtimeDestroy(CAEP_RTITEM *pRti)
{
    return CaeUnsDualMesh::destroy(*pRti);
}
