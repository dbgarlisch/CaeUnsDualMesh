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

CaeUnsDualMesh::CaeUnsDualMesh(CAEP_RTITEM *pRti, PWGM_HGRIDMODEL
        model, const CAEP_WRITEINFO *pWriteInfo) :
    CaeUnsPlugin(pRti, model, pWriteInfo),
    dumpFile_(),
    vertToCells_(),
    cellToEdge_(),
    edgeToVertex_(),
    bndryVerts_(),
    edges_(),
    numCentroids_(0),
    numMids_(0)
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
        ret = model_.streamFaces(PWGM_FACEORDER_BOUNDARYONLY, *this) &&
                writePolys();
    }
    return ret;
}


void
CaeUnsDualMesh::addElement(PWP_UINT32 elemNdx, const PWGM_ELEMDATA &ed)
{
    UInt32UInt32Array1MMap::iterator it;
    for (PWP_UINT32 ii = 0; ii < ed.vertCnt; ++ii) {
        vertToCells_.insert(UInt32UInt32Array1MMap::value_type(
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


bool
CaeUnsDualMesh::writePolys()
{
    bool ret = progressBeginStep(model_.vertexCount());
    if (ret && !vertToCells_.empty()) {
        std::pair<UInt32UInt32Array1MMap::iterator,
            UInt32UInt32Array1MMap::iterator> rng;
        UInt32Array1 fanCells;
        // set rng.second for first pass key
        rng.second = vertToCells_.begin();
        do {
            // grab subsort range using key from rng.second
            rng = vertToCells_.equal_range(rng.second->first);
            // rng.first is FIRST multimap item with key.
            // rng.second is item just after LAST multimap item with key.
            // Dual mesh poly built around this vertex.
            //PWP_UINT32 vertNdx = rng.first->first;
            // Load a vector with subrange's cell indices. These cells surround
            // vertNdx.
            fanCells.resize(std::distance(rng.first, rng.second));
            UInt32UInt32Array1MMap::iterator it = rng.first;
            for (size_t ii = 0; it != rng.second; ++it, ++ii) {
                fanCells[ii] = it->second;
            }
            // Sort cell indices in radial order around vertNdx.
            FanSorter sorter(dumpFile_);
            sorter.sort(model_, rng.first->first, fanCells, edgeToVertex_);
            writePoly(rng.first->first, fanCells);
            if (!progressIncrement()) {
                ret = false;
                break;
            }
        }
        while (vertToCells_.end() != rng.second);
    }
    return progressEndStep() && ret;
}


void
CaeUnsDualMesh::writePoly(PWP_UINT32 origVertNdx,
    const UInt32Array1 &fanCells)
{
    if (bndryVerts_.end() == bndryVerts_.find(origVertNdx)) {
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
    numMids_ = data.numBoundaryFaces;
    edges_.reserve(numMids_);
    return rtFile_.write(data.numBoundaryFaces, "\n",
        "# boundary mid points ") && progressBeginStep(numMids_);
}


static void
project(const Vec3 &v0, const Vec3 &v1, Vec3 &pt)
{
    const Vec3::value_type lenSq = (v1 - v0).length_squared();
    if (lenSq < 1.0e-8) {
        // v0 == v1
        pt = v0;
    }
    else {
        // Given line parameterized as v0 + t (v1 - v0). Find projection of
        // point pt onto the line.
        //    t = [(pt-v0) dot (v1-v0)] / lenSq
        const Vec3::value_type t = cml::dot(pt - v0, v1 - v0) / lenSq;
        if (t < 0.0) {
            pt = v0; // Beyond the 'v0' end of the segment
        }
        else if (t > 1.0) {
            pt = v1;  // Beyond the 'v1' end of the segment
        }
        else {
            pt = v0 + t * (v1 - v0);  // Projection falls on the segment
        }
    }
}


static bool
getCoord(CaeUnsGridModel &model, PWP_UINT32 ndx, Vec3& v)
{
    PWGM_VERTDATA vd;
    bool ret = CaeUnsVertex(model, ndx).dataMod(vd);
    if (ret) {
        v.set(vd.x, vd.y, vd.z);
    }
    return ret;
}


PWP_UINT32
CaeUnsDualMesh::streamFace(const PWGM_FACESTREAM_DATA &data)
{
    Edge edge(data.elemData.index[0], data.elemData.index[1]);
    PWP_UINT32 dualNdx = data.face + numCentroids_;
    cellToEdge_.insert(UInt32UInt32Array1Map::value_type(
        data.owner.cellIndex, data.face));
    edgeToVertex_.insert(EdgeToUInt32Map::value_type(edge, dualNdx));
    bndryVerts_.insert(data.elemData.index[0]);
    bndryVerts_.insert(data.elemData.index[1]);
    edges_.push_back(edge);

    // Project the boundary cell's centroid onto the edge.
    PWP_UINT32 ret = 0;
    PWGM_ELEMDATA ed;
    Vec3 v0;
    Vec3 v1;
    if (CaeUnsElement(model_, data.owner.cellIndex).data(ed) &&
            getCoord(model_, data.elemData.index[0], v0) &&
            getCoord(model_, data.elemData.index[1], v1)) {
        Vec3 pt = centroid(ed);
        project(v0, v1, pt);
        writeVertex(dualNdx, pt);
        ret = 1;
    }
    return progressIncrement() && ret;
}


PWP_UINT32
CaeUnsDualMesh::streamEnd(const PWGM_ENDSTREAM_DATA &data)
{
    return progressEndStep() && data.ok;
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
        }
        ret /= ed.vertCnt;
    }
    return ret;
}
