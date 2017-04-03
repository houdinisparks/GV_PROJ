#ifndef BVH_H
#define BVH_H

#include "BVH.h"
#include "Joint.h"
#include <fstream>
#include <vecmath.h>
#include <string>
#include <sstream>
#include <iostream>

typedef BVH::MOTION MOTION;


std::string trim(const std::string& str)
{
	size_t first = str.find_first_not_of(' ');
	if (std::string::npos == first)
	{
		return str;
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

void BVH::load(const std::string& filename)
{
	std::fstream file;
	file.open(filename.c_str(), std::ios_base::in);

	if (file.is_open())
	{
		std::string line;

		while (file.good())
		{
			file >> line;
			if (trim(line) == "HIERARCHY")
				loadHierarchy(file);
			break;
		}

		file.close();
	}

	bvhToVertices(rootJoint, skeletalVertices, skeletalIndices);

	std::cout << "loaded finished" << std::endl;
}

void BVH::loadHierarchy(std::istream& stream)
{
	std::string tmp;
	std::cout << " loading heirarchy" << std::endl;

	while (stream.good())
	{
		stream >> tmp;

		if (trim(tmp) == "ROOT")
			rootJoint = loadJoint(stream);
		else if (trim(tmp) == "MOTION")
			loadMotion(stream);
	}

	std::cout << " finish heirarchy" << std::endl;

}

Joint* BVH::loadJoint(std::istream& stream, Joint* parent) //default value of parent == NULL
{
	std::cout << " loading Joint" << std::endl;

	Joint* joint = new Joint;
	joint->parent = parent;

	// load joint name
	std::string* name = new std::string;
	stream >> *name;
	joint->name = name->c_str();

	std::string tmp;

	// setting local matrix to identity
	joint->transform = Matrix4f::identity();

	static int _channel_start = 0;
	unsigned channel_order_index = 0;

	while (stream.good())
	{
		stream >> tmp;
		tmp = trim(tmp);
		std::cout << tmp << std::endl;
		// loading channels
		char c = tmp.at(0);
		if (c == 'X' || c == 'Y' || c == 'Z')
		{
			if (tmp == "Xposition")
			{
				joint->channels_order[channel_order_index++] = Xposition;
			}
			if (tmp == "Yposition")
			{
				joint->channels_order[channel_order_index++] = Yposition;
			}
			if (tmp == "Zposition")
			{
				joint->channels_order[channel_order_index++] = Zposition;
			}

			if (tmp == "Xrotation")
			{
				joint->channels_order[channel_order_index++] = Xrotation;
			}
			if (tmp == "Yrotation")
			{
				joint->channels_order[channel_order_index++] = Yrotation;
			}
			if (tmp == "Zrotation")
			{
				joint->channels_order[channel_order_index++] = Zrotation;
			}
		}

		if (tmp == "OFFSET")
		{
			// reading an offset values
			stream >> joint->offset.x
				>> joint->offset.y
				>> joint->offset.z;
		}
		else if (tmp == "CHANNELS")
		{
			// loading num of channels
			stream >> joint->num_channels;

			// adding to motiondata
			motionData.num_motion_channels += joint->num_channels;

			// increasing static counter of channel index starting motion section
			joint->channel_start = _channel_start;
			_channel_start += joint->num_channels;

			// creating array for channel order specification
			joint->channels_order = new short[joint->num_channels];

		}
		else if (tmp == "Joint")
		{
			// loading child joint and setting this as a parent
			Joint* tmp_joint = loadJoint(stream, joint);

			tmp_joint->parent = joint;
			joint->children.push_back(tmp_joint);
		}
		else if (tmp == "End")
		{
			// loading End Site joint
			stream >> tmp >> tmp; // Site {

			Joint* tmp_joint = new Joint;

			tmp_joint->parent = joint;
			tmp_joint->num_channels = 0;
			tmp_joint->name = "EndSite";
			joint->children.push_back(tmp_joint);

			stream >> tmp;
			if (tmp == "OFFSET")
				stream >> tmp_joint->offset.x
				>> tmp_joint->offset.y
				>> tmp_joint->offset.z;

			stream >> tmp;
		}
		else if (tmp == "}")
			return joint;

	}

	std::cout << " finish Joint" << std::endl;
}

void BVH::loadMotion(std::istream& stream)
{
	std::string tmp;
	std::cout << " loading Motion" << std::endl;

	while (stream.good())
	{
		stream >> tmp;

		if (trim(tmp) == "Frames:")
		{
			// loading frame number
			stream >> motionData.num_frames;
		}
		else if (trim(tmp) == "Frame")
		{
			// loading frame time
			float frame_time;
			stream >> tmp >> frame_time;

			int num_frames = motionData.num_frames;
			int num_channels = motionData.num_motion_channels;

			// creating motion data array
			motionData.data = new float[num_frames * num_channels];

			// foreach frame read and store floats
			for (int frame = 0; frame < num_frames; frame++)
			{
				for (int channel = 0; channel < num_channels; channel++)
				{
					// reading float
					float x;
					std::stringstream ss;
					stream >> tmp;
					ss << tmp;
					ss >> x;

					// calculating index for storage
					int index = frame * num_channels + channel;
					motionData.data[index] = x;
				}
			}
		}
	}
	std::cout << " finish Motion" << std::endl;

}

/**
Calculates Joint's local transformation matrix for
specified frame starting index
*/
static void moveJoint(Joint* joint, MOTION* motionData, int frame_starts_index)
{
	// we'll need index of motion data's array with start of this specific joint
	int start_index = frame_starts_index + joint->channel_start;

	// translate indetity matrix to this joint's offset parameters
	joint->transform = Matrix4f::translation(joint->offset.x,
		joint->offset.y,
		joint->offset.z);
		

	// here we transform joint's local matrix with each specified channel's values
	// which are read from motion data
	for (int i = 0; i < joint->num_channels; i++)
	{
		// channel alias
		const short& channel = joint->channels_order[i];

		// extract value from motion data
		float value = motionData->data[start_index + i];

		if (channel & Xposition)
		{
			joint->transform = joint->transform.translation(value , 0 , 0);
		}
		if (channel & Yposition)
		{
			joint->transform = joint->transform.translation(0, value, 0);
		}
		if (channel & Zposition)
		{
			joint->transform = joint->transform.translation(0, 0, value);
		}

		if (channel & Xrotation)
		{
			joint->transform = joint->transform.rotateX(value);
		}
		if (channel & Yrotation)
		{
			joint->transform = joint->transform.rotateY(value);
		}
		if (channel & Zrotation)
		{
			joint->transform = joint->transform.rotateZ(value);
		}
	}

	// then we apply parent's local transfomation matrix to this joint's LTM (local tr. mtx. :)
	if (joint->parent != NULL)
		joint->transform = joint->parent->transform * joint->transform;

	// when we have calculated parent's matrix do the same to all children
	for (auto& child : joint->children)
		moveJoint(child, motionData, frame_starts_index);
}

void BVH::moveTo(unsigned frame)
{
	// we calculate motion data's array start index for a frame
	unsigned start_index = frame * motionData.num_motion_channels;

	// recursively transform skeleton
	moveJoint(rootJoint, &motionData, start_index);
}

//for drawing the skeleton

void BVH::drawSkeleton(bool drawSkeleton , int frame = 1) {
	std::cout << "drawing skeleton" << std::endl;

	moveTo(frame);
	//bvhToVertices already loaded



}

void BVH::bvhToVertices(Joint* joint, std::vector<Vector4f>& vertices, std::vector<int>&   indices, int parentIndex = 0) {
	// vertex from current joint is in 4-th ROW (column-major ordering)
	Vector4f translatedVertex = joint->transform.getCol(4);

	// pushing current 
	vertices.push_back(translatedVertex);

	// avoid putting root twice
	int myindex = vertices.size() - 1;
	if (parentIndex != myindex)
	{
		indices.push_back(parentIndex);
		indices.push_back(myindex);
	}

	// foreach child same thing
	for (auto& child : joint->children)
		bvhToVertices(child, vertices, indices, myindex);
}

#endif