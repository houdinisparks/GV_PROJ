#ifndef BVH_H
#define BVH_H

#include "BVH.h"
#include "Joint.h"
#include <fstream>
#include <vecmath.h>
#include <vector>
#include <string>
#include <sstream>


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
}

void BVH::loadHierarchy(std::istream& stream)
{
	std::string tmp;

	while (stream.good())
	{
		stream >> tmp;

		if (trim(tmp) == "ROOT")
			rootJoint = loadJoint(stream);
		else if (trim(tmp) == "MOTION")
			loadMotion(stream);
	}
}

Joint* BVH::loadJoint(std::istream& stream, Joint* parent) //default value of parent == NULL
{
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
}

void BVH::loadMotion(std::istream& stream)
{
	std::string tmp;

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
}

#endif