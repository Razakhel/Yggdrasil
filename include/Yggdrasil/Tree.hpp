#pragma once

#ifndef YGGDRASIL_TREE_HPP
#define YGGDRASIL_TREE_HPP

#include <RaZ/Math/Angle.hpp>

namespace Raz { class Mesh; }

class Tree {
public:
  explicit Tree(Raz::Mesh& mesh) : m_mesh{ mesh } {}
  Tree(Raz::Mesh& mesh, unsigned int branchLevel, Raz::Radiansf branchAngle) : Tree(mesh) { generate(branchLevel, branchAngle); }

  void generate(unsigned int branchLevel, Raz::Radiansf branchAngle);

private:
  Raz::Mesh& m_mesh;
};

#endif // YGGDRASIL_TREE_HPP
