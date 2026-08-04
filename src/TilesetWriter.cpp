#include "TilesetWriter.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>


bool TilesetWriter::writeTileset(

    const MeshBuilder& mesh,

    double originLat,

    double originLon,

    const std::string& filename

)
{

    const auto& vertices = mesh.getVertices();


    if(vertices.empty())
    {
        std::cout
        << "Mesh empty\n";

        return false;
    }



    double minX = vertices[0].x;
    double minY = vertices[0].y;
    double minZ = vertices[0].z;


    double maxX = vertices[0].x;
    double maxY = vertices[0].y;
    double maxZ = vertices[0].z;



    for(const auto& v : vertices)
    {

        minX = std::min(minX, (double)v.x);
        minY = std::min(minY, (double)v.y);
        minZ = std::min(minZ, (double)v.z);


        maxX = std::max(maxX, (double)v.x);
        maxY = std::max(maxY, (double)v.y);
        maxZ = std::max(maxZ, (double)v.z);

    }



    double centerX =
        (minX+maxX)*0.5;


    double centerY =
        (minY+maxY)*0.5;


    double centerZ =
        (minZ+maxZ)*0.5;



    double radius =
        sqrt(
            (maxX-minX)*(maxX-minX)
          + (maxY-minY)*(maxY-minY)
          + (maxZ-minZ)*(maxZ-minZ)
        );



    std::ofstream out(filename);


    if(!out)
        return false;



    out
    << std::fixed
    << std::setprecision(10);



    out <<
R"({
 "asset":{
   "version":"1.0"
 },

 "geometricError":500,

 "root":{

   "boundingVolume":{
      "box":[)";


    out
    << centerX << ","
    << centerY << ","
    << centerZ << ","

    << radius << ",0,0,0,"

    << radius << ",0,0,0,"

    << radius;


    out <<
R"(]
   },

   "geometricError":0,

   "refine":"ADD",

   "content":{
      "uri":"tile.b3dm"
   }

 }

})";



    out.close();



    std::cout
    << "\n========== TILESET ==========\n"
    << "Origin lat : "
    << originLat
    << "\nOrigin lon : "
    << originLon
    << "\nCenter : "
    << centerX << ", "
    << centerY << ", "
    << centerZ
    << "\nRadius : "
    << radius
    << "\nOutput : "
    << filename
    << "\n=============================\n";



    return true;

}