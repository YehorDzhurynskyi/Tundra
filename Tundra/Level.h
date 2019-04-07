#pragma once

namespace Level
{

using S_DensityChunk = std::array<vec2f, 7>;
using M_DensityChunk = std::array<vec2f, 12>;
using L_DensityChunk = std::array<vec2f, 17>;

extern const std::array<S_DensityChunk, 1> S_ChunkSelection;
extern const std::array<M_DensityChunk, 1> M_ChunkSelection;
extern const std::array<L_DensityChunk, 1> L_ChunkSelection;

}
