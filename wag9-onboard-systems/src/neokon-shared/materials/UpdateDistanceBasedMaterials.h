#pragma once

/**
 * Adds the material to the cache if the material's base material is
 * sleepers_distance_tex_switch.
 *
 * Does nothing if anything goes wrong.
 */
void maybeAddDistanceBasedMaterial(Unigine::MaterialPtr const& materialCandidate);

/**
 * Updates the switch distances of all collected and currently valid materials.
 */
void updateDistanceBasedMaterials();
