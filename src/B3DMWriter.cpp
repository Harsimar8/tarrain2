#include "B3DMWriter.h"

#include <fstream>
#include <vector>
#include <cstdint>
#include <iostream>


static void writePadding(
    std::ofstream& out,
    size_t size
)
{
    const char padding = ' ';

    for (size_t i = 0; i < size; i++)
    {
        out.write(&padding, 1);
    }
}


static size_t getPadding(
    size_t size
)
{
    size_t remainder = size % 8;

    if (remainder == 0)
        return 0;

    return 8 - remainder;
}



bool B3DMWriter::writeB3DM(
    const std::string& glbFile,
    const std::string& b3dmFile
)
{

    // -----------------------------
    // Read GLB
    // -----------------------------

    std::ifstream in(
        glbFile,
        std::ios::binary
    );


    if (!in)
    {
        std::cout << "Cannot open GLB\n";
        return false;
    }


    std::vector<char> glb(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );


    in.close();



    // -----------------------------
    // Feature Table
    // -----------------------------

    std::string featureJSON =
        "{\"BATCH_LENGTH\":0}";


    size_t featurePadding =
        getPadding(featureJSON.size());


    featureJSON.append(
        featurePadding,
        ' '
    );



    uint32_t featureJSONLength =
        static_cast<uint32_t>(
            featureJSON.size()
        );


    uint32_t featureBinaryLength = 0;

    uint32_t batchJSONLength = 0;

    uint32_t batchBinaryLength = 0;



    // -----------------------------
    // GLB alignment
    // -----------------------------

    size_t glbPadding =
        getPadding(glb.size());


    uint32_t byteLength =
        28
        +
        featureJSONLength
        +
        static_cast<uint32_t>(glb.size())
        +
        static_cast<uint32_t>(glbPadding);



    // -----------------------------
    // Write B3DM
    // -----------------------------

    std::ofstream out(
        b3dmFile,
        std::ios::binary
    );


    if (!out)
    {
        std::cout << "Cannot create B3DM\n";
        return false;
    }



    out.write(
        "b3dm",
        4
    );


    uint32_t version = 1;


    out.write(
        reinterpret_cast<char*>(&version),
        4
    );


    out.write(
        reinterpret_cast<char*>(&byteLength),
        4
    );


    out.write(
        reinterpret_cast<char*>(&featureJSONLength),
        4
    );


    out.write(
        reinterpret_cast<char*>(&featureBinaryLength),
        4
    );


    out.write(
        reinterpret_cast<char*>(&batchJSONLength),
        4
    );


    out.write(
        reinterpret_cast<char*>(&batchBinaryLength),
        4
    );



    // Feature JSON

    out.write(
        featureJSON.data(),
        featureJSON.size()
    );



    // GLB

    out.write(
        glb.data(),
        glb.size()
    );



    // GLB padding

    if(glbPadding > 0)
    {
        writePadding(
            out,
            glbPadding
        );
    }



    out.close();



   


    return true;
}