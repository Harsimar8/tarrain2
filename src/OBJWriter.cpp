#include "OBJWriter.h"

#include <fstream>

bool OBJWriter::writeOBJ(
const MeshBuilder& mesh,
const std::string& filename
)
{
std::ofstream out(filename);


if (!out.is_open())
    return false;

const auto& vertices = mesh.getVertices();
const auto& triangles = mesh.getTriangles();

for (const auto& v : vertices)
{
    out << "v "
    << v.x << " "
    << v.z << " "
    << -v.y
    << "\n";
}

out << "\n";

for (const auto& t : triangles)
{
    out << "f "
        << (t.a + 1) << " "
        << (t.b + 1) << " "
        << (t.c + 1)
        << "\n";
}

return true;


}
