#include "Yggdrasil/Tree.hpp"

#include <RaZ/Math/Quaternion.hpp>
#include <RaZ/Render/Mesh.hpp>

namespace {

void createBranch(std::vector<Raz::Vertex>& vertices, std::vector<unsigned int>& indices,
                  const Raz::Vec3f& base, const Raz::Vec3f& direction, float length,
                  unsigned int level, Raz::Radiansf angle) {
  // Creating a new triangle forming the branch:
  //
  //        tip
  //         ^
  //        / \
  //       /_x_\
  //       ^^
  //    baseStride

  const Raz::Vec3f branchPerpDir    = direction.cross(Raz::Axis::Z).normalize();
  const Raz::Vec3f branchBaseStride = branchPerpDir * 0.02f;
  const Raz::Vec3f branchTipPos     = base + direction * length;

  vertices.emplace_back(Raz::Vertex{ base + branchBaseStride });
  vertices.emplace_back(Raz::Vertex{ base - branchBaseStride });
  vertices.emplace_back(Raz::Vertex{ branchTipPos });

  indices.emplace_back(vertices.size() - 3);
  indices.emplace_back(vertices.size() - 2);
  indices.emplace_back(vertices.size() - 1);

  // If we've reached the max branch level, end the recursion
  if (level == 0)
    return;

  // Splitting the branch in both directions according to the given angle:
  //
  //      \ /
  //       |
  //       |

  const Raz::Mat3f rotation(Raz::Quaternion(angle, direction.cross(branchPerpDir)).computeMatrix());
  const float nextLength       = length * 0.66f;
  const unsigned int nextLevel = level - 1;

  createBranch(vertices, indices, branchTipPos, direction * rotation, nextLength, nextLevel, angle);
  createBranch(vertices, indices, branchTipPos, direction * rotation.inverse(), nextLength, nextLevel, angle);
}

} // namespace

void Tree::generate(unsigned int branchLevel, Raz::Radiansf branchAngle) {
  std::vector<Raz::Vertex>& vertices = m_mesh.getSubmeshes().front().getVertices();
  vertices.clear();
  vertices.reserve(static_cast<std::size_t>(std::pow(2, branchLevel)) * 2);

  std::vector<unsigned int>& indices = m_mesh.getSubmeshes().front().getTriangleIndices();
  indices.clear();
  indices.reserve(vertices.capacity() * 3);

  createBranch(vertices, indices, Raz::Vec3f(0.f, -2.f, 0.f), Raz::Axis::Y, 2.f, branchLevel, branchAngle);

  m_mesh.load();
}
