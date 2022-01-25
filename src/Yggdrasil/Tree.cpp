#include "Yggdrasil/Tree.hpp"

#include <RaZ/Entity.hpp>
#include <RaZ/Data/Mesh.hpp>
#include <RaZ/Math/Quaternion.hpp>
#include <RaZ/Math/Transform.hpp>
#include <RaZ/Render/MeshRenderer.hpp>

#include <random>

namespace {

void createBranch(Raz::Mesh& mesh, const Raz::Vec3f& base, const Raz::Vec3f& direction, float length,
                  unsigned int level, Raz::Radiansf angle, std::mt19937& randGenerator) {
  static std::uniform_real_distribution<float> randDistrib(1.f, Raz::Pi<float>);

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
  const Raz::Vec3f branchTipPos     = base + direction * length * randDistrib(randGenerator);

  std::vector<Raz::Vertex>& vertices = mesh.getSubmeshes().front().getVertices();
  vertices.emplace_back(Raz::Vertex{ base - branchBaseStride });
  vertices.emplace_back(Raz::Vertex{ base + branchBaseStride });
  vertices.emplace_back(Raz::Vertex{ branchTipPos });

  std::vector<unsigned int>& indices = mesh.getSubmeshes().front().getTriangleIndices();
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

  const Raz::Mat3f rotation(Raz::Quaternion(angle * randDistrib(randGenerator), direction.cross(branchPerpDir)).computeMatrix());
  const float nextLength       = length * 0.66f;
  const unsigned int nextLevel = level - 1;

  const Raz::Vec3f nextDirection    = direction * rotation;
  const Raz::Vec3f nextInvDirection = rotation * direction;

  createBranch(mesh, branchTipPos, nextDirection, nextLength, nextLevel, angle, randGenerator);
  createBranch(mesh, branchTipPos, nextInvDirection, nextLength, nextLevel, angle, randGenerator);

  const Raz::Mat3f upRotation(Raz::Quaternionf(Raz::Degrees(90.f), Raz::Axis::Y).computeMatrix());

  createBranch(mesh, branchTipPos, nextDirection * upRotation, nextLength, nextLevel, angle, randGenerator);
  createBranch(mesh, branchTipPos, upRotation * nextDirection, nextLength, nextLevel, angle, randGenerator);
}

} // namespace

Tree::Tree(Raz::Entity& entity) : m_entity{ entity } {
  if (!m_entity.hasComponent<Raz::Transform>())
    m_entity.addComponent<Raz::Transform>();

  if (!m_entity.hasComponent<Raz::Mesh>())
    m_entity.addComponent<Raz::Mesh>();

  if (!m_entity.hasComponent<Raz::MeshRenderer>())
    m_entity.addComponent<Raz::MeshRenderer>();
}

void Tree::generate(unsigned int branchLevel, Raz::Radiansf branchAngle) {
  auto& mesh         = m_entity.getComponent<Raz::Mesh>();
  auto& meshRenderer = m_entity.getComponent<Raz::MeshRenderer>();

  mesh.getSubmeshes().resize(1);
  meshRenderer.getSubmeshRenderers().resize(1);

  std::vector<Raz::Vertex>& vertices = mesh.getSubmeshes().front().getVertices();
  vertices.clear();
  vertices.reserve(static_cast<std::size_t>(std::pow(2, branchLevel)) * 2);

  std::vector<unsigned int>& indices = mesh.getSubmeshes().front().getTriangleIndices();
  indices.clear();
  indices.reserve(vertices.capacity());

  std::mt19937 randGenerator(0);
  createBranch(mesh, Raz::Vec3f(0.f, -2.f, 0.f), Raz::Axis::Y, 2.f, branchLevel, branchAngle, randGenerator);

  meshRenderer.load(mesh);
}
