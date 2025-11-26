#include "SkyDomeMeshGenerator.h"
#include "VertexArray.h"
#include "MathUtils.h"
#include <vector>
#include <cmath>

namespace SkyDomeMeshGenerator
{

std::unique_ptr<VertexArray> CreateSkyDomeVAO(int slices, int stacks, float radius)
{
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<unsigned int> indices;

    for (int y = 0; y <= stacks; ++y)
    {
        float v = static_cast<float>(y) / stacks;
        float phi = v * Math::PiOver2; // π/2 まで：半球のみ

        for (int x = 0; x <= slices; ++x)
        {
            float u = static_cast<float>(x) / slices;
            float theta = u * Math::TwoPi;

            float xPos = radius * sinf(phi) * cosf(theta);
            float yPos = radius * cosf(phi); // Y+が上方向（従来と逆）
            float zPos = radius * sinf(phi) * sinf(theta);

            positions.push_back(xPos);
            positions.push_back(yPos);
            positions.push_back(zPos);

            Vector3 n = Vector3(xPos, yPos, zPos);
            n.Normalize();
            normals.push_back(n.x);
            normals.push_back(n.y);
            normals.push_back(n.z);

            texCoords.push_back(u);
            texCoords.push_back(1.0f - v);
        }
    }

    for (int y = 0; y < stacks; ++y)
    {
        for (int x = 0; x < slices; ++x)
        {
            int i0 = y * (slices + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (slices + 1);
            int i3 = i2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    return std::make_unique<VertexArray>(
        static_cast<unsigned int>(positions.size() / 3),
        positions.data(),
        normals.data(),
        texCoords.data(),
        static_cast<unsigned int>(indices.size()),
        indices.data()
    );
}

} // namespace SkyDomeMeshGenerator
/*
namespace SkyDomeMeshGenerator {

std::unique_ptr<class VertexArray> CreateSkyDomeVAO(int slices, int stacks, float radius)
{
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<unsigned int> indices;

    for (int y = 0; y <= stacks; ++y)
    {
        float v = static_cast<float>(y) / stacks;
        float phi = v * Math::PiOver2; // π/2 まで：半球のみ

        for (int x = 0; x <= slices; ++x)
        {
            float u = static_cast<float>(x) / slices;
            float theta = u * Math::TwoPi;

            float xPos = radius * sinf(phi) * cosf(theta);
            float yPos = radius * cosf(phi);
            float zPos = radius * sinf(phi) * sinf(theta);

            positions.push_back(xPos);
            positions.push_back(yPos);
            positions.push_back(zPos);

            Vector3 n = Vector3(xPos, yPos, zPos);
            n.Normalize();
            normals.push_back(n.x);
            normals.push_back(n.y);
            normals.push_back(n.z);

            texCoords.push_back(u);
            texCoords.push_back(1.0f - v);
        }
    }

    for (int y = 0; y < stacks -1; ++y)
    {
        for (int x = 0; x < slices; ++x)
        {
            int i0 = y * (slices + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (slices + 1);
            int i3 = i2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    return std::make_unique<VertexArray>(
        static_cast<unsigned int>(positions.size() / 3),
        positions.data(),
        normals.data(),
        texCoords.data(),
        static_cast<unsigned int>(indices.size()),
        indices.data()
    );
}
} // namespace SkyDomeMeshGenerator
*/
