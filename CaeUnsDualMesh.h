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


private: // base class virtual methods

    virtual bool        beginExport();
    virtual PWP_BOOL    write();

    void    addElement(PWP_UINT32 elemNdx, const PWGM_ELEMDATA &ed);
    void    writeVertex(PWP_UINT32 elemNdx, const Vec3 &v);
    void    writeGceVertices();
    bool    writePolys();
    void    writePoly(PWP_UINT32 gceVertNdx, const UInt32Array1 &cellIndices);

    // face streaming handlers
    virtual PWP_UINT32 streamBegin(const PWGM_BEGINSTREAM_DATA &data);
    virtual PWP_UINT32 streamFace(const PWGM_FACESTREAM_DATA &data);
    virtual PWP_UINT32 streamEnd(const PWGM_ENDSTREAM_DATA &data);

    bool        handleBndryFace(const PWGM_FACESTREAM_DATA &data);
    bool        handleCnxnFace(const PWGM_FACESTREAM_DATA &data);
    void        addHardEdge(PWP_UINT32 dualNdx, const PWGM_ELEMDATA &elemData);
    bool        projectCellCentroidToEdge(PWP_UINT32 cellNdx,
                    const PWGM_ELEMDATA &edgeElemData, Vec3 &edgePt);
    const Vec3& centroid(const PWGM_ELEMDATA &ed) const;
    bool        getCoord(PWP_UINT32 ndx, Vec3& v) const;

private:

    //! The debug dump file
    PwpFile                 dumpFile_;

    //! Maps a gce vertNdx to the gce cells that touch it.
    UInt32UInt32Array1MMap  gceVertToGceCells_;

    //! Maps a gce vertNdx to hardGceEdges_ indices that touch it
    UInt32UInt32Array1MMap  gceVertToHardGceEdges_;

    // Maps a gce edge to its dual mesh vertex index.
    EdgeToUInt32Map         hardGceEdgeToDualVert_;

    //! The set of boundary/connection gce vertex indices
    UInt32Set               hardGceVerts_;

    //! Array of boundary/connection gce edges.
    EdgeArray1              hardGceEdges_;

    //! Number of gce cell centroid vertices
    PWP_UINT32              numCentroids_;

    //! Number of boundary gce edge mid point vertices
    PWP_UINT32              numBndryMids_;

    //! Number of connection gce edge mid point vertices
    PWP_UINT32              numCnxnMids_;
};

#endif // _CAEUNSDUALMESH_H_
