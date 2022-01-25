#include "Yggdrasil/Tree.hpp"

#include <RaZ/Application.hpp>
#include <RaZ/Math/Transform.hpp>
#include <RaZ/Render/RenderSystem.hpp>
#include <RaZ/Utils/Logger.hpp>

using namespace std::literals;
using namespace Raz::Literals;

int main() {
  ////////////////////
  // Initialization //
  ////////////////////

  Raz::Application app;
  Raz::World& world = app.addWorld(3);

  Raz::Logger::setLoggingLevel(Raz::LoggingLevel::ALL);

  ///////////////
  // Rendering //
  ///////////////

  auto& renderSystem = world.addSystem<Raz::RenderSystem>(1280u, 720u, "Yggdrasil", Raz::WindowSetting::DEFAULT, 2);

  Raz::RenderPass& geometryPass = renderSystem.getGeometryPass();
  geometryPass.getProgram().setShaders(Raz::VertexShader(RAZ_ROOT + "shaders/common.vert"s),
                                       Raz::FragmentShader(RAZ_ROOT + "shaders/cook-torrance.frag"s));

  Raz::Window& window = renderSystem.getWindow();
  window.setClearColor(0.75f, 0.75f, 0.75f);

#if !defined(USE_OPENGL_ES)
  // Allow wireframe toggling
  bool isWireframe = false;
  window.addKeyCallback(Raz::Keyboard::Z, [&isWireframe] (float /* deltaTime */) {
    isWireframe = !isWireframe;
    Raz::Renderer::setPolygonMode(Raz::FaceOrientation::FRONT_BACK, (isWireframe ? Raz::PolygonMode::LINE : Raz::PolygonMode::FILL));
  }, Raz::Input::ONCE);
#endif

  // Allowing to quit the application with the Escape key
  window.addKeyCallback(Raz::Keyboard::ESCAPE, [&app] (float /* deltaTime */) noexcept { app.quit(); });

  ///////////////////
  // Camera entity //
  ///////////////////

  Raz::Entity& camera = world.addEntity();
  auto& cameraComp    = camera.addComponent<Raz::Camera>(window.getWidth(), window.getHeight(), 45_deg, 0.1f, 1000.f);
  auto& cameraTrans   = camera.addComponent<Raz::Transform>(Raz::Vec3f(0.f, 5.f, 17.5f));

  //////////
  // Tree //
  //////////

  unsigned int branchLevel  = 10;
  Raz::Radiansf branchAngle = 20_deg;

  Tree tree(world.addEntity(), branchLevel, branchAngle);

  Raz::Renderer::disable(Raz::Capability::CULL);

  /////////////////////
  // Camera controls //
  /////////////////////

  float cameraSpeed = 1.f;
  window.addKeyCallback(Raz::Keyboard::LEFT_SHIFT,
                        [&cameraSpeed] (float /* deltaTime */) noexcept { cameraSpeed = 2.f; },
                        Raz::Input::ONCE,
                        [&cameraSpeed] () noexcept { cameraSpeed = 1.f; });
  window.addKeyCallback(Raz::Keyboard::SPACE, [&cameraTrans, &cameraSpeed] (float deltaTime) {
    cameraTrans.move(0.f, (10.f * deltaTime) * cameraSpeed, 0.f);
  });
  window.addKeyCallback(Raz::Keyboard::V, [&cameraTrans, &cameraSpeed] (float deltaTime) {
    cameraTrans.move(0.f, (-10.f * deltaTime) * cameraSpeed, 0.f);
  });
  window.addKeyCallback(Raz::Keyboard::W, [&cameraTrans, &cameraComp, &cameraSpeed] (float deltaTime) {
    const float moveVal = (-10.f * deltaTime) * cameraSpeed;

    cameraTrans.move(0.f, 0.f, moveVal);
    cameraComp.setOrthoBoundX(cameraComp.getOrthoBoundX() + moveVal);
    cameraComp.setOrthoBoundY(cameraComp.getOrthoBoundY() + moveVal);
  });
  window.addKeyCallback(Raz::Keyboard::S, [&cameraTrans, &cameraComp, &cameraSpeed] (float deltaTime) {
    const float moveVal = (10.f * deltaTime) * cameraSpeed;

    cameraTrans.move(0.f, 0.f, moveVal);
    cameraComp.setOrthoBoundX(cameraComp.getOrthoBoundX() + moveVal);
    cameraComp.setOrthoBoundY(cameraComp.getOrthoBoundY() + moveVal);
  });
  window.addKeyCallback(Raz::Keyboard::A, [&cameraTrans, &cameraSpeed] (float deltaTime) {
    cameraTrans.move((-10.f * deltaTime) * cameraSpeed, 0.f, 0.f);
  });
  window.addKeyCallback(Raz::Keyboard::D, [&cameraTrans, &cameraSpeed] (float deltaTime) {
    cameraTrans.move((10.f * deltaTime) * cameraSpeed, 0.f, 0.f);
  });

  window.addMouseScrollCallback([&cameraComp] (double /* xOffset */, double yOffset) {
    const float newFov = Raz::Degreesf(cameraComp.getFieldOfView()).value + static_cast<float>(-yOffset) * 2.f;
    cameraComp.setFieldOfView(Raz::Degreesf(std::clamp(newFov, 15.f, 90.f)));
  });

  bool cameraLocked = true; // To allow moving the camera using the mouse

  window.addMouseButtonCallback(Raz::Mouse::RIGHT_CLICK, [&cameraLocked, &window] (float) {
    cameraLocked = false;
    window.changeCursorState(Raz::Cursor::DISABLED);
  }, Raz::Input::ONCE, [&cameraLocked, &window] () {
    cameraLocked = true;
    window.changeCursorState(Raz::Cursor::NORMAL);
  });

  window.addMouseMoveCallback([&cameraLocked, &cameraTrans, &window] (double xMove, double yMove) {
    if (cameraLocked)
      return;

    // Dividing move by window size to scale between -1 and 1
    cameraTrans.rotate(-90_deg * yMove / window.getHeight(),
                       -90_deg * xMove / window.getWidth());
  });

  /////////////
  // Overlay //
  /////////////

  Raz::OverlayWindow& overlay = window.getOverlay().addWindow("Yggdrasil", Raz::Vec2f(-1.f));

  overlay.addLabel("Press WASD to fly the camera around,");
  overlay.addLabel("Space/V to go up/down,");
  overlay.addLabel("& Shift to move faster.");
  overlay.addLabel("Hold the right mouse button to rotate the camera.");

  overlay.addSeparator();

  overlay.addSlider("Branches level", [&branchLevel, &branchAngle, &tree] (float value) {
    branchLevel = static_cast<unsigned int>(value);
    tree.generate(branchLevel, branchAngle);
  }, 0.f, 10.f, static_cast<float>(branchLevel));

  overlay.addSlider("Branches angle", [&branchLevel, &branchAngle, &tree] (float value) {
    branchAngle = Raz::Degreesf(value);
    tree.generate(branchLevel, branchAngle);
  }, 5.f, 45.f, Raz::Degreesf(branchAngle).value);

  overlay.addSeparator();

  overlay.addFrameTime("Frame time: %.3f ms/frame");
  overlay.addFpsCounter("FPS: %.1f");

  //////////////////////////
  // Starting application //
  //////////////////////////

  app.run();

  return EXIT_SUCCESS;
}
