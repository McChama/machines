/*
 * O B J M E S H . H P P
 * Phase 1.5: OBJ mesh importer.
 */

#ifndef _RENDER_OBJMESH_HPP
#define _RENDER_OBJMESH_HPP

#include "base/base.hpp"
#include "ctl/vector.hpp"
#include "stdlib/string.hpp"
#include "mathex/point3d.hpp"
#include "mathex/vec3.hpp"
#include "mathex/point2d.hpp"

class SysPathName;

// One triangle, referring into RenIObjMesh's own vertex arrays (see
// RenIObjMesh::addVertex - always a real, already-deduplicated vertex, unlike
// a raw OBJ face's v/vt/vn indices).
struct RenIObjTriangle
{
    size_t v0, v1, v2;
};

// One group of triangles sharing a material (parsed from the OBJ's `usemtl`
// directives and the companion .mtl file's `map_Kd`/`Kd`).
struct RenIObjMaterialGroup
{
    string materialName;
    string diffuseTexture;   // from map_Kd; empty if the material has none
    float diffuseR = 1.0f, diffuseG = 1.0f, diffuseB = 1.0f;   // from Kd; white if absent
    ctl_vector<RenIObjTriangle> triangles;
};

// A parsed, render-ready mesh: already in this engine's Y-up render-space
// convention (unlike GXMesh/XFile::Mesh, which are Z-up and need the axis
// flip RenMesh::buildFromGXMesh/buildFromXMesh perform - a Blender-exported
// OBJ is already Y-up, so no such flip is needed here), and already
// deduplicated (each entry is a distinct position+normal+uv combination).
// Produced by RenIObjMeshLoader::load(), consumed by
// RenMesh::buildFromObjMesh(). Deliberately has no GL/RenDevice dependency so
// it can be unit-tested in isolation - see src/tests/test_objmesh.cpp.
class RenIObjMesh
{
public:
    // Always appends a new vertex and returns its index; the caller (the
    // loader) is responsible for deduplication - see RenIObjMeshLoader::load.
    size_t addVertex(const MexPoint3d& pos, const MexVec3& normal, const MexPoint2d& uv);

    size_t numVertices() const                 { return positions_.size(); }
    const MexPoint3d& position(size_t i) const { return positions_[i]; }
    const MexVec3& normal(size_t i) const      { return normals_[i]; }
    const MexPoint2d& uv(size_t i) const       { return uvs_[i]; }

    size_t numMaterialGroups() const                       { return materialGroups_.size(); }
    RenIObjMaterialGroup& materialGroup(size_t i)          { return materialGroups_[i]; }
    const RenIObjMaterialGroup& materialGroup(size_t i) const { return materialGroups_[i]; }

    // Returns the index of the group for this material name, creating a new
    // (empty) one if this is the first triangle using it.
    RenIObjMaterialGroup& findOrCreateMaterialGroup(const string& materialName);

private:
    ctl_vector<MexPoint3d> positions_;
    ctl_vector<MexVec3>    normals_;
    ctl_vector<MexPoint2d> uvs_;
    ctl_vector<RenIObjMaterialGroup> materialGroups_;
};

// Parses path (with a .obj extension) and, if it references one via mtllib,
// the companion .mtl file in the same directory, into an already-deduplicated
// RenIObjMesh. Supports the `f v`, `f v/vt`, `f v//vn` and `f v/vt/vn` face
// forms (triangulating faces with more than 3 vertices as a fan) and 1-based
// positive indices only (relative/negative OBJ indices are not supported).
// Returns false (outMesh left unspecified) if the file can't be read/parsed.
class RenIObjMeshLoader
{
public:
    static bool load(const SysPathName& path, RenIObjMesh* outMesh);
};

#endif

/* End OBJMESH.HPP ****************************************************/
