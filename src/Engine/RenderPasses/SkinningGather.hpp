#ifndef SKINNINGGATHER_H_
#define SKINNINGGATHER_H_

class ECSManager;

// One entry of the per-instance vertex buffer (binding 1). Shared by
// GeometryPass and ShadowPass so the CPU layout matches the pipeline's
// vertex attributes.
//
// jointOffset travels per instance rather than as a uniform so several
// skinned entities, each with its own skeleton, can be drawn in one
// instanced call. It is a float to avoid adding an integer vertex format;
// the values are small enough that the float->int cast in the shader is
// exact.
struct InstanceData
{
  glm::mat4 modelMatrix;
  float jointOffset;
};

// Where one skinned (entity, node) pair's joint matrices live in the shared
// "jointMats" data texture for the current frame.
struct SkinnedNodeOffset
{
  Entity entity;
  u32 nodeIdx;
  u32 jointOffset; // First texel row of this skeleton's matrices.
};

// Packs every skinned entity's joint matrices into the shared jointMats
// texture in one upload, returning the row offset assigned to each
// (entity, node).
//
// Memoized per frame: ShadowPass and GeometryPass both call this, but only
// the first call does work.
const std::vector<SkinnedNodeOffset>&
gatherAndUploadFrameSkinning(ECSManager& eManager);

// Returns 0 when absent — non-skinned nodes never consult this.
u32
findJointOffset(const std::vector<SkinnedNodeOffset>& frameSkinning,
                Entity entity,
                u32 nodeIdx);

// Clears the per-frame memoization guard. Call once per frame before any
// pass that draws skinned entities.
void
resetFrameSkinningGather();

#endif // SKINNINGGATHER_H_
