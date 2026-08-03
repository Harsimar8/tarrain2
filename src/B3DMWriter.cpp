#include "B3DMWriter.h"
#include <fstream>
#include <vector>
#include <cstdint>
#include <iostream>

bool B3DMWriter::writeB3DM(
const std::string& glbFile,
const std::string& b3dmFile
)
{
std::ifstream in(glbFile, std::ios::binary);


if (!in)
    return false;

std::vector<char> glb(
    (std::istreambuf_iterator<char>(in)),
    std::istreambuf_iterator<char>()
);

in.close();

std::ofstream out(b3dmFile, std::ios::binary);

if (!out)
    return false;

const uint32_t version = 1;

std::string featureTable = "{\"BATCH_LENGTH\":0}";

while (featureTable.size() % 8 != 0)
    featureTable.push_back(' ');

const uint32_t featureJson = static_cast<uint32_t>(featureTable.size());
const uint32_t featureBin  = 0;
const uint32_t batchJson   = 0;
const uint32_t batchBin    = 0;

uint32_t byteLength =
    28 +
    featureJson +
    static_cast<uint32_t>(glb.size());

out.write("b3dm", 4);
out.write(reinterpret_cast<const char*>(&version), 4);
out.write(reinterpret_cast<const char*>(&byteLength), 4);
out.write(reinterpret_cast<const char*>(&featureJson), 4);
out.write(reinterpret_cast<const char*>(&featureBin), 4);
out.write(reinterpret_cast<const char*>(&batchJson), 4);
out.write(reinterpret_cast<const char*>(&batchBin), 4);

out.write(featureTable.data(), featureTable.size());
out.write(glb.data(), glb.size());

out.close();

std::cout << "\n========== STEP 3 : GLB -> B3DM ==========" << std::endl;
std::cout << "Input GLB        : " << glbFile << std::endl;
std::cout << "GLB size         : " << glb.size() << " bytes" << std::endl;
std::cout << "Feature table    : " << featureTable.size() << " bytes" << std::endl;
std::cout << "B3DM byteLength  : " << byteLength << std::endl;
std::cout << "Output B3DM      : " << b3dmFile << std::endl;
std::cout << "==========================================\n" << std::endl;

return true;


}
