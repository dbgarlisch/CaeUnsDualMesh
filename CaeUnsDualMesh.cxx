/****************************************************************************
 *
 * class CaeUnsDualMesh
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#include <cmath>
#include <cstring>

#include "CaeUnsGridModel.h"
#include "CaeUnsDualMesh.h"
#include "FanSorter.h"
#include "PluginTypes.h"

const char *attrDebugDump           = "DebugDump";


//***************************************************************************
//***************************************************************************
//***************************************************************************

static void
projectPtToLineSeg(const Vec3 &pt, const Vec3 &v0, const Vec3 &v1, Vec3 &projPt)
{
    // project pt onto line segment (v0, v1)
    const Vec3::value_type lenSq = (v1 - v0).length_squared();
    if (lenSq < 1.0e-8) {
        // v0 == v1
        projPt = v0;
    }
    else {
        // Given line parameterized as v0 + t (v1 - v0). Find projection of
        // point pt onto the line.
        //    t = [(pt-v0) dot (v1-v0)] / lenSq
        const Vec3::value_type t = cml::dot(pt - v0, v1 - v0) / lenSq;
        if (t < 0.0) {
            projPt = v0; // Beyond the 'v0' end of the segment
        }
        else if (t > 1.0) {
            projPt = v1;  // Beyond the 'v1' end of the segment
        }
        else {
            projPt = v0 + t * (v1 - v0);  // Falls on the segment
        }
    }
}


//***************************************************************************
//***************************************************************************
//***************************************************************************

CaeUnsDualMesh::CaeUnsDualMesh(CAEP_RTITEM *pRti, PWGM_HGRIDMODEL
        model, const CAEP_WRITEINFO *pWriteInfo) :
    CaeUnsPlugin(pRti, model, pWriteInfo),
    dumpFile_(),
    gceVertToGceCells_(),
    gceVertToHardGceEdges_(),
    hardGceEdgeToDualVert_(),
    hardGceVerts_(),
    hardGceEdges_(),
    numCentroids_(0),
    numBndryMids_(0),
    numCnxnMids_(0)
{
}


CaeUnsDualMesh::~CaeUnsDualMesh()
{
}


bool
CaeUnsDualMesh::beginExport()
{
    /*! The dualGrid's point list will contain all cell centers followed by
        all boundary mid points. Hence, for:
          n = 0..NumCells-1, dualVertex = cellNdx
          n = NumCells..NumDualPoints, dualVertex = bndryNdx + NumCells
        We do not need to store the XYZ values in a map. They can be computed
        on the fly when writing the vertex list to disk.
    */
    PWP_BOOL doDump;
    model_.getAttribute(attrDebugDump, doDump);
    char filename[512];
    strcpy(filename, writeInfo_.fileDest);
    strcat(filename, ".dump");
    if (!doDump) {
        // remove dump file if it exists from previous run
        pwpFileDelete(filename);
    }
    else {
        dumpFile_.open(filename, pwpWrite | pwpAscii);
        sendInfoMsg("debug dump file:", 0);
        sendInfoMsg(filename, 0);
        if (!dumpFile_.isOpen()) {
            sendErrorMsg("debug dump file open failed!", 0);
        }
    }
    setProgressMajorSteps(3);
    return true;
}


PWP_BOOL
CaeUnsDualMesh::write()
{
    writeGceVertices();
    numCentroids_ = model_.elementCount();
    bool ret = progressBeginStep(numCentroids_);
    if (ret) {
        rtFile_.write(numCentroids_, "\n", "# Element centroid points ");
        PWGM_ELEMDATA ed;
        PWP_UINT32 dualNdx = 0;
        CaeUnsElement elem(model_);
        while (elem.data(ed)) {
            writeVertex(dualNdx++, centroid(ed));
            addElement(elem.index(), ed);
            if (!progressIncrement()) {
                ret = false;
                break;
            }
            ++elem;
        }
        ret = progressEndStep() && ret;
    }
    if (ret) {
        // PWGM_FACEORDER_BOUNDARYONLY
        ret = model_.streamFaces(PWGM_FACEORDER_BOUNDARYFIRST, *this) &&
                writePolys();
    }
    return ret;
}


void
CaeUnsDualMesh::addElement(PWP_UINT32 elemNdx, const PWGM_ELEMDATA &ed)
{
    for (PWP_UINT32 ii = 0; ii < ed.vertCnt; ++ii) {
        gceVertToGceCells_.insert(UInt32UInt32Array1MMap::value_type(
            ed.index[ii], elemNdx));
    }
}


void
CaeUnsDualMesh::writeVertex(PWP_UINT32 dualNdx, const Vec3 &v)
{
    rtFile_.write("vertex ");
    rtFile_.write((dualNdx < numCentroids_) ? "E " : "B ");
    rtFile_.write(dualNdx, " { ");
    rtFile_.write(v[0], " ");
    rtFile_.write(v[1], " ");
    rtFile_.write(v[2], " }\n");
}


void
CaeUnsDualMesh::writeGceVertices()
{
    PWGM_VERTDATA d;
    CaeUnsVertex v(model_);
    while (v.isValid()) {
        v.dataMod(d);
        rtFile_.write("gceVertex ");
        rtFile_.write(d.i, " { ");
        rtFile_.write(d.x, " ");
        rtFile_.write(d.y, " ");
        rtFile_.write(d.z, " }\n");
        ++v;
    }
}


bool
CaeUnsDualMesh::writePolys()
{
    bool ret = progressBeginStep(model_.vertexCount());
    if (ret && !gceVertToGceCells_.empty()) {
        std::pair<UInt32UInt32Array1MMap::iterator,
            UInt32UInt32Array1MMap::iterator> rng;
        UInt32Array1 fanCells;
        // The do/while loop expects rng.second to be the "end" of the previous
        // loop. Must set rng.second for first pass.
        rng.second = gceVertToGceCells_.begin();
        do {
            // Grab range using key from rng.second. After call, rng.first is
            // the FIRST multimap item with key. rng.second is item just after
            // LAST multimap item with key (first item in next range).
            rng = gceVertToGceCells_.equal_range(rng.second->first);
            // Presize the fanCells vector.
            fanCells.resize(std::distance(rng.first, rng.second));
            // Load the fanCells vector with subrange's cell indices. These
            // cells surround gce vertex with index == rng.first->first.
            UInt32UInt32Array1MMap::iterator it = rng.first;
            for (size_t ii = 0; it != rng.second; ++it, ++ii) {
                // cache gce cell index
                fanCells[ii] = it->second;
            }
            // Sort cell indices in radial order around gce vertex. Multiple
            // fans are possible if hard edges are encountered.
            FanSorter sorter(dumpFile_, hardGceEdgeToDualVert_);
            UInt32Array2 fans;
            sorter.run(model_, rng.first->first, fanCells, fans);
            UInt32Array2::const_iterator itFan = fans.begin();
            for (; itFan != fans.end(); ++itFan) {
                writePoly(rng.first->first, *itFan);
            }
            if (!progressIncrement()) {
                ret = false;
                break;
            }
        } while (gceVertToGceCells_.end() != rng.second);
        // Debug dump hardGceEdgeToDualVert_ info
        if (dumpFile_.isOpen()) {
            dumpFile_.write("# hardGceEdgeToDualVert_\n");
            EdgeToUInt32Map::const_iterator it =
                hardGceEdgeToDualVert_.begin();
            for (; it != hardGceEdgeToDualVert_.end(); ++it) {
                dumpFile_.write(it->first[0], " ", "#    edge { ");
                dumpFile_.write(it->first[1], " } ");
                dumpFile_.write(it->second, "\n", "vert=");
            }
        }
    }
    return progressEndStep() && ret;
}


void
CaeUnsDualMesh::writePoly(PWP_UINT32 gceVertNdx, const UInt32Array1 &fanCells)
{
    if (hardGceVerts_.end() == hardGceVerts_.find(gceVertNdx)) {
        // interior vertex. Cells form a full, 360 deg polygon
        rtFile_.write("poly I { ");
    }
    else {
        // boundary vertex. Cells form a partial, <360 deg polygon
        rtFile_.write("poly B { ");
    }
    UInt32Array1::const_iterator it = fanCells.begin();
    for (; it != fanCells.end(); ++it) {
        rtFile_.write(*it, " ");
    }
    rtFile_.write("}\n");
}


PWP_UINT32
CaeUnsDualMesh::streamBegin(const PWGM_BEGINSTREAM_DATA &data)
{
    numBndryMids_ = data.numBoundaryFaces;
    hardGceEdges_.reserve(numBndryMids_);
    return rtFile_.write(data.numBoundaryFaces, "\n",
        "# boundary mid points ") && progressBeginStep(numBndryMids_);
}


PWP_UINT32
CaeUnsDualMesh::streamFace(const PWGM_FACESTREAM_DATA &data)
{
    bool ret = false;
    switch (data.type) {
    case PWGM_FACETYPE_BOUNDARY:
        ret = handleBndryFace(data);
        break;
    case PWGM_FACETYPE_INTERIOR:
        ret = true; // ignore interior faces
        break;
    case PWGM_FACETYPE_CONNECTION:
        ret = handleCnxnFace(data);
        break;
    }
    return progressIncrement() && ret;
}


PWP_UINT32
CaeUnsDualMesh::streamEnd(const PWGM_ENDSTREAM_DATA &data)
{
    return progressEndStep() && data.ok;
}


bool
CaeUnsDualMesh::handleBndryFace(const PWGM_FACESTREAM_DATA &data)
{
    PWP_UINT32 dualNdx = data.face + numCentroids_;
    addHardEdge(dualNdx, data.elemData);
    // Project the boundary cell's centroid onto the edge.
    bool ret = false;
    Vec3 pt;
    if (projectCellCentroidToEdge(data.owner.cellIndex, data.elemData, pt)) {
        writeVertex(dualNdx, pt);
        ret = true;
    }
    return ret;
}


bool
CaeUnsDualMesh::handleCnxnFace(const PWGM_FACESTREAM_DATA &data)
{
    PWP_UINT32 dualNdx = numCnxnMids_ + numBndryMids_ + numCentroids_;
    addHardEdge(dualNdx, data.elemData);
    // Project the connection owner/neighbor cell centroids onto the edge and
    // average them. If the cells are highly skewed, this will produce poor
    // results becasue the projection is clipped to the edge endpoints.
    bool ret = false;
    Vec3 pt0;
    Vec3 pt1;
    if (projectCellCentroidToEdge(data.owner.cellIndex, data.elemData, pt0) &&
            projectCellCentroidToEdge(data.neighborCellIndex, data.elemData, pt1)) {
        writeVertex(dualNdx, (pt0 += pt1) /= 2.0);
        ret = true;
    }
    ++numCnxnMids_;
    return ret;
}


void
CaeUnsDualMesh::addHardEdge(PWP_UINT32 dualNdx, const PWGM_ELEMDATA &elemData)
{
    Edge edge(elemData.index[0], elemData.index[1]);
    hardGceEdgeToDualVert_.insert(EdgeToUInt32Map::value_type(edge, dualNdx));
    hardGceVerts_.insert(elemData.index[0]);
    hardGceVerts_.insert(elemData.index[1]);
    gceVertToHardGceEdges_.insert(UInt32UInt32Array1MMap::value_type(
        elemData.index[0], PWP_UINT32(hardGceEdges_.size())));
    gceVertToHardGceEdges_.insert(UInt32UInt32Array1MMap::value_type(
        elemData.index[1], PWP_UINT32(hardGceEdges_.size())));
    hardGceEdges_.push_back(edge);
}


bool
CaeUnsDualMesh::projectCellCentroidToEdge(PWP_UINT32 cellNdx,
    const PWGM_ELEMDATA &edgeElemData, Vec3 &edgePt)
{
    // Project the boundary cell's centroid onto the edge.
    bool ret = false;
    PWGM_ELEMDATA cellElemData;
    Vec3 v0;
    Vec3 v1;
    if (CaeUnsElement(model_, cellNdx).data(cellElemData) &&
            getCoord(edgeElemData.index[0], v0) &&
            getCoord(edgeElemData.index[1], v1)) {
        projectPtToLineSeg(centroid(cellElemData), v0, v1, edgePt);
        ret = true;
    }
    return ret;
}


const Vec3&
CaeUnsDualMesh::centroid(const PWGM_ELEMDATA &ed) const
{
    static Vec3 ret;
    CaeUnsVertex v(model_, ed.index[0]);
    PWGM_VERTDATA vd;
    if (v.dataMod(vd)) {
        ret.set(vd.x, vd.y, vd.z);
        for (PWP_UINT32 ii = 1; ii < ed.vertCnt; ++ii) {
            if (v.moveTo(model_, ed.index[ii]).dataMod(vd)) {
                ret += Vec3(vd.x, vd.y, vd.z);
            }
            else {
                ret.zero();
                break;
            }
        }
        ret /= ed.vertCnt;
    }
    else {
        ret.zero();
    }
    return ret;
}


bool
CaeUnsDualMesh::getCoord(PWP_UINT32 ndx, Vec3& v) const
{
    PWGM_VERTDATA vd;
    bool ret = CaeUnsVertex(model_, ndx).dataMod(vd);
    if (ret) {
        v.set(vd.x, vd.y, vd.z);
    }
    return ret;
}


bool
CaeUnsDualMesh::create(CAEP_RTITEM &rti)
{
    (void)rti.BCCnt; // silence unused arg warning
    return publishBoolValueDef(rti, attrDebugDump, "no",
        "Generate a debug dump file?", "no|yes");
}


void
CaeUnsDualMesh::destroy(CAEP_RTITEM &rti)
{
    (void)rti.BCCnt; // silence unused arg warning
}
