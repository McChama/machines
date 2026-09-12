#include <catch2/catch.hpp>

#include "render/internal/objmesh.hpp"
#include "system/pathname.hpp"

namespace
{
    SysPathName fixturePath(const char* name)
    {
        return SysPathName(string(TEST_FIXTURES_DIR) + "/" + name);
    }
}

TEST_CASE( "RenIObjMeshLoader parses a simple quad", "[objmesh]" )
{
    RenIObjMesh mesh;
    bool loaded = RenIObjMeshLoader::load(fixturePath("simple_quad.obj"), &mesh);

    REQUIRE( loaded );

    SECTION( "vertex count and attributes" )
    {
        // Each of the quad's 4 corners has a distinct position and uv, so no
        // deduplication collapses any of them together.
        REQUIRE( mesh.numVertices() == 4 );

        REQUIRE( mesh.position(0).x() == Approx(0.0) );
        REQUIRE( mesh.position(0).y() == Approx(0.0) );
        REQUIRE( mesh.position(0).z() == Approx(0.0) );

        REQUIRE( mesh.position(2).x() == Approx(1.0) );
        REQUIRE( mesh.position(2).y() == Approx(1.0) );
        REQUIRE( mesh.position(2).z() == Approx(0.0) );

        // Every corner shares the same explicit vn (0,0,1).
        for (size_t i = 0; i < mesh.numVertices(); ++i)
        {
            REQUIRE( mesh.normal(i).x() == Approx(0.0) );
            REQUIRE( mesh.normal(i).y() == Approx(0.0) );
            REQUIRE( mesh.normal(i).z() == Approx(1.0) );
        }

        // OBJ's V axis is bottom-up; RenIObjMeshLoader flips it (1-v) to match
        // this engine's top-down convention - so OBJ "vt 0 0" (bottom-left)
        // becomes uv (0,1), and OBJ "vt 1 1" (top-right) becomes uv (1,0).
        REQUIRE( mesh.uv(0).x() == Approx(0.0) );
        REQUIRE( mesh.uv(0).y() == Approx(1.0) );
        REQUIRE( mesh.uv(2).x() == Approx(1.0) );
        REQUIRE( mesh.uv(2).y() == Approx(0.0) );
    }

    SECTION( "material groups" )
    {
        REQUIRE( mesh.numMaterialGroups() == 1 );

        const RenIObjMaterialGroup& group = mesh.materialGroup(0);
        REQUIRE( group.materialName == "Fixture" );
        REQUIRE( group.diffuseTexture == "test_diffuse.png" );
        REQUIRE( group.diffuseR == Approx(0.8) );
        REQUIRE( group.diffuseG == Approx(0.2) );
        REQUIRE( group.diffuseB == Approx(0.2) );
    }

    SECTION( "triangulation and winding" )
    {
        const RenIObjMaterialGroup& group = mesh.materialGroup(0);
        REQUIRE( group.triangles.size() == 2 );

        // No axis flip is applied for OBJ (unlike buildFromGXMesh's .agt/.X
        // path), so face winding is preserved exactly as written: the two
        // faces "1/1/1 2/2/1 3/3/1" and "1/1/1 3/3/1 4/4/1" become 0-based
        // (0,1,2) and (0,2,3).
        REQUIRE( group.triangles[0].v0 == 0 );
        REQUIRE( group.triangles[0].v1 == 1 );
        REQUIRE( group.triangles[0].v2 == 2 );

        REQUIRE( group.triangles[1].v0 == 0 );
        REQUIRE( group.triangles[1].v1 == 2 );
        REQUIRE( group.triangles[1].v2 == 3 );
    }
}

TEST_CASE( "RenIObjMeshLoader fails cleanly on a missing file", "[objmesh]" )
{
    RenIObjMesh mesh;
    bool loaded = RenIObjMeshLoader::load(fixturePath("does_not_exist.obj"), &mesh);

    REQUIRE_FALSE( loaded );
}
