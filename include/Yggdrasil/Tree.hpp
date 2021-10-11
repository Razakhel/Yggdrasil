#pragma once

#ifndef YGGDRASIL_TREE_HPP
#define YGGDRASIL_TREE_HPP

#include <RaZ/Math/Angle.hpp>

namespace Raz { class Entity; }

class Tree {
public:
  explicit Tree(Raz::Entity& entity);
  Tree(Raz::Entity& entity, unsigned int branchLevel, Raz::Radiansf branchAngle) : Tree(entity) { generate(branchLevel, branchAngle); }

  void generate(unsigned int branchLevel, Raz::Radiansf branchAngle);

private:
  Raz::Entity& m_entity;
};

#endif // YGGDRASIL_TREE_HPP
