#ifndef PHYSICSLAYERS_H_
#define PHYSICSLAYERS_H_

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace Layers {
inline constexpr JPH::ObjectLayer NON_MOVING = 0;
inline constexpr JPH::ObjectLayer MOVING = 1;
inline constexpr JPH::uint NUM_LAYERS = 2;
} // namespace Layers

namespace BroadPhaseLayers {
inline constexpr JPH::BroadPhaseLayer NON_MOVING(0);
inline constexpr JPH::BroadPhaseLayer MOVING(1);
inline constexpr JPH::uint NUM_LAYERS = 2;
} // namespace BroadPhaseLayers

class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
{
public:
  bool ShouldCollide(JPH::ObjectLayer inObject1,
                     JPH::ObjectLayer inObject2) const override
  {
    switch (inObject1) {
      case Layers::NON_MOVING:
        return inObject2 == Layers::MOVING;
      case Layers::MOVING:
        return true;
      default:
        return false;
    }
  }
};

class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
  BroadPhaseLayerInterfaceImpl()
  {
    m_objectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    m_objectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
  }

  JPH::uint GetNumBroadPhaseLayers() const override
  {
    return BroadPhaseLayers::NUM_LAYERS;
  }

  JPH::BroadPhaseLayer GetBroadPhaseLayer(
    JPH::ObjectLayer inLayer) const override
  {
    return m_objectToBroadPhase[inLayer];
  }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  const char* GetBroadPhaseLayerName(
    JPH::BroadPhaseLayer inLayer) const override
  {
    switch ((JPH::BroadPhaseLayer::Type)inLayer) {
      case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
        return "NON_MOVING";
      case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
        return "MOVING";
      default:
        return "INVALID";
    }
  }
#endif

private:
  JPH::BroadPhaseLayer m_objectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl final
  : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
  bool ShouldCollide(JPH::ObjectLayer inLayer1,
                     JPH::BroadPhaseLayer inLayer2) const override
  {
    switch (inLayer1) {
      case Layers::NON_MOVING:
        return inLayer2 == BroadPhaseLayers::MOVING;
      case Layers::MOVING:
        return true;
      default:
        return false;
    }
  }
};

#endif // PHYSICSLAYERS_H_
