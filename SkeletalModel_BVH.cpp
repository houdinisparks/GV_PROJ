#pragma once

#include "SkeletalModel_BVH.h"
#include "BVH.h"

using namespace std;
void SkeletalModel_BVH::loadSkeleton(const std::string& filename) {
	bvh.load(filename);
}

void SkeletalModel_BVH::drawSkeleton(Matrix4f cameraMatrix, bool drawSkeleton) {

}

