#pragma once

#include "Mesh.h"
#include <vector>

namespace Primitives
{
    //functions that returns Mesh object , generate vertices and indices
    //Cube
    void GenerateCube(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices );

    //Plane
    void GeneratePlane(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices , int segments = 1 );

    
} // namespace Primitives
