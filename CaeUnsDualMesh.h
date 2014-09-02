/****************************************************************************
 *
 * class CaeUnsDualMesh
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _CAEUNSDUALMESH_H_
#define _CAEUNSDUALMESH_H_

#include <map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "CaePlugin.h"
#include "CaeUnsGridModel.h"
#include "PluginTypes.h"


//***************************************************************************
//***************************************************************************
//***************************************************************************

class CaeUnsDualMesh : public CaeUnsPlugin, public CaeFaceStreamHandler {
public:

    CaeUnsDualMesh(CAEP_RTITEM *pRti, PWGM_HGRIDMODEL model,
        const CAEP_WRITEINFO *pWriteInfo);

    ~CaeUnsDualMesh();

    static bool create(CAEP_RTITEM &rti);
    static void destroy(CAEP_RTITEM &rti);

    const Vec3& centroid(const PWGM_ELEMDATA &ed) const;


private: // base class virtual methods

    virtual bool        beginExport();
    virtual PWP_BOOL    write();

    void    addElement(PWP_UINT32 elemNdx, const PWGM_ELEMDATA &ed);
    void    writeVertex(PWP_UINT32 elemNdx, const Vec3 &v);
    bool    writePolys();
    void    writePoly(PWP_UINT32 origVertNdx, const UInt32Array1 &cellIndices);

    // face streaming handlers
    virtual PWP_UINT32 streamBegin(const PWGM_BEGINSTREAM_DATA &data);
    virtual PWP_UINT32 streamFace(const PWGM_FACESTREAM_DATA &data);
    virtual PWP_UINT32 streamEnd(const PWGM_ENDSTREAM_DATA &data);


private:

    //! The debug dump file
    PwpFile                 dumpFile_;

    //! Maps each original vertNdx to the cells that touch it.
    UInt32UInt32Array1MMap  vertToCells_;

    //! Maps boundary cell index to its boundary edge index.
    UInt32UInt32Array1Map   cellToEdge_;

    // Maps an edge to its dual mesh vertex index.
    EdgeToUInt32Map         edgeToVertex_;

    //! The set of boundary vertex indices
    UInt32Set               bndryVerts_;

    //! Array of boundary edges.
    EdgeArray1              edges_;

    //! Number of centroid vertices
    PWP_UINT32              numCentroids_;

    //! Number of boundary mid point vertices
    PWP_UINT32              numMids_;
};

#endif // _CAEUNSDUALMESH_H_
