#pragma once

#include "Joint.h"
#include "Logger.h"
#include "Mesh.h"


class BVH
{
public:
	void load(const char * bvhfile, const char * meshfile, const char * attachfile);
	void loadBVH(const std::string & filename);
	void loadHierarchy(std::istream& stream);
	Joint* loadJoint(std::istream& stream, Joint* parent = NULL);

	void loadMotion(std::istream& stream);

	void computeBindWorldToJointTransforms(Joint *rootJoint);

	typedef struct HIERARCHY
	{
		Joint* rootJoint;
		int num_channels;
	} HIERARCHY;

	typedef struct MOTION
	{
		unsigned int num_frames;              // number of frames
		unsigned int num_motion_channels = 0; // number of motion channels
		float* data = NULL;                   // motion float data array
		unsigned* joint_channel_offsets;      // number of channels from beggining of hierarchy for i-th joint
	} MOTION;

	typedef struct
	{
		unsigned int num_hierarchies;
		unsigned int num_total_joints;
		unsigned int num_motion_channels = 0;
		unsigned int num_frames;

	} BVH_STATISTICS;

	BVH();
	~BVH() {};

	// loading

	void printJoint( Joint*  joint) const;

	void moveJoint(Joint * joint, MOTION * motionData, int frame_starts_index);

	/** Loads motion data from a frame into local matrices */
	void moveTo(unsigned frame) ;

	const Joint* getRootJoint() const { return rootJoint; }
	unsigned getNumFrames() const { return motionData.num_frames; }


	void updateMesh();

	void drawMesh(bool drawMesh, int frame);

	// Drawing the Skeleton
	void bvhToVertices(Joint * joint, std::vector<Vector4f>& vertices, std::vector<int>& indices, int parentIndex);
	void loadMesh(const char * meshFile);
	void loadAttachments(const char * attachmentFile);
	void testOutput() const;
	void drawSkeleton( bool drawSkeleton, int frame );

    void init_mesh_collide();
    bool check_mesh_collide();


private:
	std::vector<Vector4f> skeletalVertices;
	std::vector<int> skeletalIndices;
	Joint* rootJoint;
	std::vector<Joint*> m_joints;
	MOTION motionData;
	Mesh m_mesh;
};
