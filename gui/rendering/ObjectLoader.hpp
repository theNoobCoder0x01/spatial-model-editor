//
// Created by acaramizaru on 6/30/23.
//

#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh/Surface_mesh.h>

#include <fstream>
#include <iostream>
#include <string>

#include "ObjectInfo.hpp"

namespace rendering {

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point;

typedef CGAL::Surface_mesh<Point> SMesh;

class ObjectLoader {
public:
  static ObjectInfo Load(const std::string &filename);
  static ObjectInfo Load(const SMesh &);
  static SMesh LoadMesh(const std::string &filename);
};

} // namespace rendering
