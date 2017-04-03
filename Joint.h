#ifndef JOINT_H
#define JOINT_H

#include <vector>
#include <vecmath.h>

#define Xposition 0x01
#define Yposition 0x02
#define Zposition 0x04
#define Zrotation 0x10
#define Xrotation 0x20
#define Yrotation 0x40


typedef struct OFFSET
{
	float x, y, z;
} OFFSET;

struct Joint
{
	Matrix4f transform; // transform relative to its parent
	std::vector< Joint* > children; // list of children

	// This matrix transforms world space into joint space for the initial ("bind") configuration of the joints.
	Matrix4f bindWorldToJointTransform;

	// This matrix maps joint space into world space for the *current* configuration of the joints.
	Matrix4f currentJointToWorldTransform;

	//FOR BVH IMPLEMENTATION

	const char* name = NULL;        // joint name
	Joint* parent = NULL;           // joint parent
	OFFSET offset;                  // offset data
	unsigned int num_channels = 0;  // num of channels joint has
	short* channels_order = NULL;   // ordered list of channels
	unsigned int channel_start = 0; // index of joint's channel data in motion array

};

#endif
