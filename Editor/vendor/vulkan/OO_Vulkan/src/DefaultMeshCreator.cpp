#include "DefaultMeshCreator.h"

#include "MathCommon.h"

DefaultMesh CreateDefaultCubeMesh()
{
    DefaultMesh mesh;

    constexpr glm::vec3 redColor = glm::vec3{ 1.0f,0.0f,0.0f };
    constexpr glm::vec3 dirX = glm::vec3{ 1.0f,0.0f,0.0f };
    constexpr glm::vec3 dirY = glm::vec3{ 0.0f,1.0f,0.0f };
    constexpr glm::vec3 dirZ = glm::vec3{ 0.0f,0.0f,1.0f };

    // The default box mesh must have 6 faces (or rather 6 planes),
    // with the normals correctly pointing outwards relative to the planes.
    // This cube is also unit and normalized (in [-0.5,0.5] range)
    mesh.m_VertexBuffer =
    {
        // 
        oGFX::Vertex{ {-0.5, 0.5, 0.5}, dirZ, redColor, { 0.0f, 1.0f }, dirX },
        oGFX::Vertex{ { 0.5, 0.5, 0.5}, dirZ, redColor, { 1.0f, 1.0f }, dirX },
        oGFX::Vertex{ {-0.5,-0.5, 0.5}, dirZ, redColor, { 0.0f, 0.0f }, dirX },
        oGFX::Vertex{ { 0.5,-0.5, 0.5}, dirZ, redColor, { 1.0f, 0.0f }, dirX },
        //
        oGFX::Vertex{ { 0.5, 0.5,-0.5}, -dirZ, redColor, { 0.0f, 1.0f }, -dirX },
        oGFX::Vertex{ {-0.5, 0.5,-0.5}, -dirZ, redColor, { 1.0f, 1.0f }, -dirX },
        oGFX::Vertex{ { 0.5,-0.5,-0.5}, -dirZ, redColor, { 0.0f, 0.0f }, -dirX },
        oGFX::Vertex{ {-0.5,-0.5,-0.5}, -dirZ, redColor, { 1.0f, 0.0f }, -dirX },
        //
        oGFX::Vertex{ { 0.5, 0.5, 0.5}, dirX, redColor, { 0.0f, 1.0f }, -dirZ },
        oGFX::Vertex{ { 0.5, 0.5,-0.5}, dirX, redColor, { 1.0f, 1.0f }, -dirZ },
        oGFX::Vertex{ { 0.5,-0.5, 0.5}, dirX, redColor, { 0.0f, 0.0f }, -dirZ },
        oGFX::Vertex{ { 0.5,-0.5,-0.5}, dirX, redColor, { 1.0f, 0.0f }, -dirZ },
        //
        oGFX::Vertex{ {-0.5, 0.5,-0.5}, -dirX, redColor, { 0.0f, 1.0f }, dirZ },
        oGFX::Vertex{ {-0.5, 0.5, 0.5}, -dirX, redColor, { 1.0f, 1.0f }, dirZ },
        oGFX::Vertex{ {-0.5,-0.5,-0.5}, -dirX, redColor, { 0.0f, 0.0f }, dirZ },
        oGFX::Vertex{ {-0.5,-0.5, 0.5}, -dirX, redColor, { 1.0f, 0.0f }, dirZ },
        //
        oGFX::Vertex{ {-0.5, 0.5,-0.5}, dirY, redColor, { 0.0f, 1.0f }, dirX },
        oGFX::Vertex{ { 0.5, 0.5,-0.5}, dirY, redColor, { 1.0f, 1.0f }, dirX },
        oGFX::Vertex{ {-0.5, 0.5, 0.5}, dirY, redColor, { 0.0f, 0.0f }, dirX },
        oGFX::Vertex{ { 0.5, 0.5, 0.5}, dirY, redColor, { 1.0f, 0.0f }, dirX },
        //
        oGFX::Vertex{ {-0.5,-0.5, 0.5}, -dirY, redColor, { 0.0f, 1.0f }, dirX },
        oGFX::Vertex{ { 0.5,-0.5, 0.5}, -dirY, redColor, { 1.0f, 1.0f }, dirX },
        oGFX::Vertex{ {-0.5,-0.5,-0.5}, -dirY, redColor, { 0.0f, 0.0f }, dirX },
        oGFX::Vertex{ { 0.5,-0.5,-0.5}, -dirY, redColor, { 1.0f, 0.0f }, dirX }

        // Data here is dumped from my engine somewhere...
        // Putting all these here for sanity check
        //[0]	position=[-0.5  0.5 0.5] normal=[0 0 1] tangent=[1 0 0] bitangent=[0 1 0] uv=[0 1]
        //[1]	position=[ 0.5  0.5 0.5] normal=[0 0 1] tangent=[1 0 0] bitangent=[0 1 0] uv=[1 1]
        //[2]	position=[-0.5 -0.5 0.5] normal=[0 0 1] tangent=[1 0 0] bitangent=[0 1 0] uv=[0 0]
        //[3]	position=[ 0.5 -0.5 0.5] normal=[0 0 1] tangent=[1 0 0] bitangent=[0 1 0] uv=[1 0]
        //
        //[4]	position=[ 0.5  0.5 -0.5] normal=[0 0 -1] tangent=[-1 0 0] bitangent=[0 1 0] uv=[0 1]
        //[5]	position=[-0.5  0.5 -0.5] normal=[0 0 -1] tangent=[-1 0 0] bitangent=[0 1 0] uv=[1 1]
        //[6]	position=[ 0.5 -0.5 -0.5] normal=[0 0 -1] tangent=[-1 0 0] bitangent=[0 1 0] uv=[0 0]
        //[7]	position=[-0.5 -0.5 -0.5] normal=[0 0 -1] tangent=[-1 0 0] bitangent=[0 1 0] uv=[1 0]
        //
        //[8]	position=[0.5  0.5  0.5] normal=[1 0 0] tangent=[0 0 -1] bitangent=[0 1 0] uv=[0 1]
        //[9]	position=[0.5  0.5 -0.5] normal=[1 0 0] tangent=[0 0 -1] bitangent=[0 1 0] uv=[1 1]
        //[10]  position=[0.5 -0.5  0.5] normal=[1 0 0] tangent=[0 0 -1] bitangent=[0 1 0] uv=[0 0]
        //[11]  position=[0.5 -0.5 -0.5] normal=[1 0 0] tangent=[0 0 -1] bitangent=[0 1 0] uv=[1 0]
        //
        //[12]  position=[-0.5  0.5 -0.5] normal=[-1 0 0] tangent=[0 0 1] bitangent=[0 1 0] uv=[0 1]
        //[13]  position=[-0.5  0.5  0.5] normal=[-1 0 0] tangent=[0 0 1] bitangent=[0 1 0] uv=[1 1]
        //[14]  position=[-0.5 -0.5 -0.5] normal=[-1 0 0] tangent=[0 0 1] bitangent=[0 1 0] uv=[0 0]
        //[15]  position=[-0.5 -0.5  0.5] normal=[-1 0 0] tangent=[0 0 1] bitangent=[0 1 0] uv=[1 0]
        //
        //[16]  {position=[-0.5 0.5 -0.5] normal=[0 1 0] tangent=[1 0 0] bitangent=[0 0 -1] uv=[0 1]
        //[17]  {position=[ 0.5 0.5 -0.5] normal=[0 1 0] tangent=[1 0 0] bitangent=[0 0 -1] uv=[1 1]
        //[18]  {position=[-0.5 0.5  0.5] normal=[0 1 0] tangent=[1 0 0] bitangent=[0 0 -1] uv=[0 0]
        //[19]  {position=[ 0.5 0.5  0.5] normal=[0 1 0] tangent=[1 0 0] bitangent=[0 0 -1] uv=[1 0]
        //
        //[20]  {position=[-0.5 -0.5  0.5] normal=[0 -1 0] tangent=[1 0 0] bitangent=[0 0 1] uv=[0 1]
        //[21]  {position=[ 0.5 -0.5  0.5] normal=[0 -1 0] tangent=[1 0 0] bitangent=[0 0 1] uv=[1 1]
        //[22]  {position=[-0.5 -0.5 -0.5] normal=[0 -1 0] tangent=[1 0 0] bitangent=[0 0 1] uv=[0 0]
        //[23]  {position=[ 0.5 -0.5 -0.5] normal=[0 -1 0] tangent=[1 0 0] bitangent=[0 0 1] uv=[1 0]
    };

    mesh.m_IndexBuffer =
    {
        0,2,1,
        1,2,3,
        4,6,5,
        5,6,7,
        8,10,9,
        9,10,11,
        12,14,13,
        13,14,15,
        16,18,17,
        17,18,19,
        20,22,21,
        21,22,23
    };

    return mesh;
}

// We must be explicit by saying this is a XZ-plane, because a plane can mean anything, is it XY, XZ, or YZ...?
DefaultMesh CreateDefaultPlaneXZMesh()
{
    DefaultMesh mesh;

    mesh.m_VertexBuffer =
    {
        oGFX::Vertex{ {-0.5f, 0.0f ,-0.5f}, { 0.0f,1.0f,0.0f }, { 1.0f,0.0f,0.0f }, { 0.0f,0.0f } },
        oGFX::Vertex{ { 0.5f, 0.0f ,-0.5f}, { 0.0f,1.0f,0.0f }, { 1.0f,0.0f,0.0f }, { 1.0f,0.0f } },
        oGFX::Vertex{ { 0.5f, 0.0f , 0.5f}, { 0.0f,1.0f,0.0f }, { 1.0f,0.0f,0.0f }, { 1.0f,1.0f } },
        oGFX::Vertex{ {-0.5f, 0.0f , 0.5f}, { 0.0f,1.0f,0.0f }, { 1.0f,0.0f,0.0f }, { 0.0f,1.0f } },
    };

    mesh.m_IndexBuffer =
    {
        0,2,1,
        2,0,3
    };

    return mesh;
}


