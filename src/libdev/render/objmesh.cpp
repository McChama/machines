/*
 * O B J M E S H . C P P
 * Phase 1.5: OBJ mesh importer.
 */

#include "render/internal/ren_pch.hpp"	// NB: pre-compiled header must come 1st
#include "render/internal/objmesh.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>
#include "system/pathname.hpp"

size_t RenIObjMesh::addVertex(const MexPoint3d& pos, const MexVec3& normal, const MexPoint2d& uv)
{
    positions_.push_back(pos);
    normals_.push_back(normal);
    uvs_.push_back(uv);
    return positions_.size() - 1;
}

RenIObjMaterialGroup& RenIObjMesh::findOrCreateMaterialGroup(const string& materialName)
{
    for (size_t i = 0; i < materialGroups_.size(); ++i)
    {
        if (materialGroups_[i].materialName == materialName)
            return materialGroups_[i];
    }

    RenIObjMaterialGroup newGroup;
    newGroup.materialName = materialName;
    materialGroups_.push_back(newGroup);
    return materialGroups_[materialGroups_.size() - 1];
}

namespace
{
    // Raw material data parsed from a .mtl file, keyed by name (as declared
    // by `newmtl`).
    struct ObjMaterial
    {
        std::string diffuseTexture;
        float diffuseR = 1.0f, diffuseG = 1.0f, diffuseB = 1.0f;
    };

    // Identifies one unique (position, normal, uv) combination by the raw,
    // 0-based indices into the OBJ file's own v/vn/vt lists - mirrors the
    // role GXPolyVert3 plays for RenMesh::buildFromGXMesh's dedup map.
    struct ObjVertexKey
    {
        long posIndex, normalIndex, uvIndex;

        bool operator<(const ObjVertexKey& rhs) const
        {
            if (posIndex != rhs.posIndex) return posIndex < rhs.posIndex;
            if (normalIndex != rhs.normalIndex) return normalIndex < rhs.normalIndex;
            return uvIndex < rhs.uvIndex;
        }
    };

    // One `f` line's parsed face-vertex token, e.g. "12/4/7" -> (12,4,7).
    // A missing component (e.g. "12//7") is represented as 0 (OBJ indices
    // are 1-based, so 0 is otherwise unused) and resolved by the caller.
    struct ObjFaceVertex
    {
        long posIndex = 0, uvIndex = 0, normalIndex = 0;
    };

    bool parseFaceVertexToken(const std::string& token, ObjFaceVertex* out)
    {
        // Forms: "v", "v/vt", "v//vn", "v/vt/vn".
        size_t firstSlash = token.find('/');
        if (firstSlash == std::string::npos)
        {
            out->posIndex = std::atol(token.c_str());
            return out->posIndex != 0;
        }

        out->posIndex = std::atol(token.substr(0, firstSlash).c_str());
        if (out->posIndex == 0)
            return false;

        size_t secondSlash = token.find('/', firstSlash + 1);
        if (secondSlash == std::string::npos)
        {
            // "v/vt"
            out->uvIndex = std::atol(token.substr(firstSlash + 1).c_str());
            return true;
        }

        // "v/vt/vn" or "v//vn"
        if (secondSlash > firstSlash + 1)
            out->uvIndex = std::atol(token.substr(firstSlash + 1, secondSlash - firstSlash - 1).c_str());
        out->normalIndex = std::atol(token.substr(secondSlash + 1).c_str());
        return true;
    }

    // Resolves a possibly-negative-index-free OBJ index (1-based) against a
    // list of size n, returning a 0-based index. Only positive indices are
    // supported - see RenIObjMeshLoader's class comment.
    long resolveIndex(long objIndex)
    {
        return objIndex - 1;
    }

    bool parseMtlFile(const SysPathName& path, std::map<std::string, ObjMaterial>* outMaterials)
    {
        std::ifstream file(path.pathname().c_str());
        if (!file)
            return false;

        std::string currentName;
        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream ls(line);
            std::string keyword;
            ls >> keyword;

            if (keyword == "newmtl")
            {
                ls >> currentName;
                (*outMaterials)[currentName] = ObjMaterial();
            }
            else if (keyword == "map_Kd" && !currentName.empty())
            {
                std::string texName;
                ls >> texName;
                (*outMaterials)[currentName].diffuseTexture = texName;
            }
            else if (keyword == "Kd" && !currentName.empty())
            {
                float r = 1.0f, g = 1.0f, b = 1.0f;
                ls >> r >> g >> b;
                ObjMaterial& mat = (*outMaterials)[currentName];
                mat.diffuseR = r;
                mat.diffuseG = g;
                mat.diffuseB = b;
            }
        }
        return true;
    }
}

bool RenIObjMeshLoader::load(const SysPathName& path, RenIObjMesh* outMesh)
{
    PRE(outMesh);

    SysPathName withExt(path);
    withExt.extension("obj");

    std::ifstream file(withExt.pathname().c_str());
    if (!file)
        return false;

    std::vector<MexPoint3d> rawPositions;
    std::vector<MexVec3> rawNormals;
    std::vector<MexPoint2d> rawUVs;

    std::map<std::string, ObjMaterial> materials;
    std::string currentMaterialName;

    std::map<ObjVertexKey, size_t> vertexCache;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream ls(line);
        std::string keyword;
        ls >> keyword;

        if (keyword == "v")
        {
            double x = 0, y = 0, z = 0;
            ls >> x >> y >> z;
            rawPositions.push_back(MexPoint3d(x, y, z));
        }
        else if (keyword == "vn")
        {
            double x = 0, y = 0, z = 0;
            ls >> x >> y >> z;
            rawNormals.push_back(MexVec3(x, y, z));
        }
        else if (keyword == "vt")
        {
            double u = 0, v = 0;
            ls >> u >> v;
            // OBJ's V axis is bottom-up; this engine's is top-down - matches
            // the same 1-v flip RenMesh::buildFromGXMesh applies for .agt.
            rawUVs.push_back(MexPoint2d(u, 1.0 - v));
        }
        else if (keyword == "mtllib")
        {
            std::string mtlName;
            ls >> mtlName;

            SysPathName mtlPath(withExt.directory());
            mtlPath.combine(SysPathName(mtlName.c_str()));
            parseMtlFile(mtlPath, &materials);
        }
        else if (keyword == "usemtl")
        {
            ls >> currentMaterialName;
        }
        else if (keyword == "f")
        {
            std::vector<ObjFaceVertex> faceVertices;
            std::string token;
            while (ls >> token)
            {
                ObjFaceVertex fv;
                if (parseFaceVertexToken(token, &fv))
                    faceVertices.push_back(fv);
            }

            if (faceVertices.size() < 3)
                continue;

            // Compute a flat face normal up-front, used for any face-vertex
            // that doesn't specify one (uncommon for a Blender export, but
            // cheap to handle reasonably rather than defaulting to a fixed,
            // possibly-wrong direction).
            MexVec3 faceNormal(0, 0, 1);
            {
                long p0 = resolveIndex(faceVertices[0].posIndex);
                long p1 = resolveIndex(faceVertices[1].posIndex);
                long p2 = resolveIndex(faceVertices[2].posIndex);
                if (p0 >= 0 && p0 < (long)rawPositions.size() &&
                    p1 >= 0 && p1 < (long)rawPositions.size() &&
                    p2 >= 0 && p2 < (long)rawPositions.size())
                {
                    MexVec3 edge1(rawPositions[p0], rawPositions[p1]);
                    MexVec3 edge2(rawPositions[p0], rawPositions[p2]);
                    MexVec3 cross = MexVec3::crossProduct(edge1, edge2);
                    if (!cross.isZeroVector())
                    {
                        cross.makeUnitVector();
                        faceNormal = cross;
                    }
                }
            }

            // Resolve each face-vertex to a final (deduplicated) mesh vertex
            // index, mirroring RenMesh::buildFromGXMesh's IndexesMap.
            std::vector<size_t> finalIndices;
            for (size_t i = 0; i < faceVertices.size(); ++i)
            {
                const ObjFaceVertex& fv = faceVertices[i];
                ObjVertexKey key = { fv.posIndex, fv.normalIndex, fv.uvIndex };

                std::map<ObjVertexKey, size_t>::iterator it = vertexCache.find(key);
                if (it != vertexCache.end())
                {
                    finalIndices.push_back(it->second);
                    continue;
                }

                long posIdx = resolveIndex(fv.posIndex);
                if (posIdx < 0 || posIdx >= (long)rawPositions.size())
                    return false;
                MexPoint3d pos = rawPositions[posIdx];

                MexVec3 normal = faceNormal;
                if (fv.normalIndex != 0)
                {
                    long normIdx = resolveIndex(fv.normalIndex);
                    if (normIdx >= 0 && normIdx < (long)rawNormals.size())
                        normal = rawNormals[normIdx];
                }

                MexPoint2d uv(0, 0);
                if (fv.uvIndex != 0)
                {
                    long uvIdx = resolveIndex(fv.uvIndex);
                    if (uvIdx >= 0 && uvIdx < (long)rawUVs.size())
                        uv = rawUVs[uvIdx];
                }

                size_t newIndex = outMesh->addVertex(pos, normal, uv);
                vertexCache[key] = newIndex;
                finalIndices.push_back(newIndex);
            }

            RenIObjMaterialGroup& group = outMesh->findOrCreateMaterialGroup(currentMaterialName.c_str());
            std::map<std::string, ObjMaterial>::const_iterator matIt = materials.find(currentMaterialName);
            if (matIt != materials.end())
            {
                group.diffuseTexture = matIt->second.diffuseTexture.c_str();
                group.diffuseR = matIt->second.diffuseR;
                group.diffuseG = matIt->second.diffuseG;
                group.diffuseB = matIt->second.diffuseB;
            }

            // Fan-triangulate faces with more than 3 vertices.
            for (size_t i = 1; i + 1 < finalIndices.size(); ++i)
            {
                RenIObjTriangle tri;
                tri.v0 = finalIndices[0];
                tri.v1 = finalIndices[i];
                tri.v2 = finalIndices[i + 1];
                group.triangles.push_back(tri);
            }
        }
    }

    return outMesh->numVertices() > 0;
}

/* End OBJMESH.CPP ****************************************************/
