#pragma once

#include <vector>
#include "vecmath.h"
#include "BVH.h"

class SkeletalModel_BVH {
public:
	SkeletalModel_BVH() {};
	~SkeletalModel_BVH() { };
	void load(const char *skeletonFile, const char *meshFile, const char *attachmentsFile); // load wireframe, mesh, wght
	void drawSkeleton(Matrix4f cameraMatrix, bool drawSkeleton);

	// This method should compute m_rootJoint and populate m_joints.
	void loadSkeleton(const std::string& filename);

	/** Loads BVH Joint Vertices into an array */
	void bvhToVertices(Joint * joint, std::vector<Vector4f>& vertices, std::vector<int>& indices, int parentIndex);


private:

	BVH bvh;
	std::vector<Vector3f> skeletalVertices;
	std::vector< Joint* > m_joints;
	Joint* m_rootJoint;


};