#ifndef BVH_H
#define BVH_H

#include "BVH.h"
#include "Joint.h"
#include <fstream>
#include <vecmath.h>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <cctype>
#include <future>


#define M_PI           3.14159265358979323846
#ifdef WIN32
#include "GL/freeglut.h"
#else
#include <GL/glut.h>
#endif

int count_joint = 0;
typedef BVH::MOTION MOTION;
BVH::BVH()
	: rootJoint(NULL)
{
	motionData.data = 0;
}

// trim from start
static inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}

float convertToRadians(float degrees) {
	return (M_PI * degrees) / 180.0f;

}
void BVH::load(const std::string& filename)
{
	std::fstream file;
	char buffer[8192];
	//file.rdbuf()->pubsetbuf(buffer, 8192);

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

	std::cout << "loaded finished" << std::endl;
}

bool invalidChar(char c)
{
	return !(c >= 0 && c < 128);
}
void stripUnicode(std::string & str)
{
	str.erase(remove_if(str.begin(), str.end(), invalidChar), str.end());
}

void BVH::loadHierarchy(std::istream& stream)
{
	std::string tmp;
	std::cout << " loading heirarchy" << std::endl;


	while (stream.good())
	{
		stream >> tmp;
		klog.l("hier") << tmp;

		if (trim(tmp) == "ROOT")
			rootJoint = loadJoint(stream);
		else if (trim(tmp) == "MOTION")
			loadMotion(stream);
	}
	

	//std::cout << " finish heirarchy" << std::endl;

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
	try {
		while (stream.good())
		{
			stream >> tmp;
			tmp = trim(tmp);

			// setting channels
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
			else if (tmp == "JOINT")
			{
				// loading child joint and setting this as a parent
				/*std::cout << "\tJoint";*/
					//std::cout << count_joint++ << std::endl;
				Joint* tmp_joint = this->loadJoint(stream, joint);

				tmp_joint->parent = joint;
				joint->children.push_back(tmp_joint);
			}
			else if (tmp == "End")
			{
				// End Site {
	
				stream >> tmp >> tmp;

				Joint* tmp_joint = new Joint;

				tmp_joint->parent = joint;
				tmp_joint->num_channels = 0;
				tmp_joint->name = "EndSite";
				joint->children.push_back(tmp_joint);

				//            allJoints.insert(tmp_joint);

				stream >> tmp;
				if (tmp == "OFFSET") {
	
					stream >> tmp_joint->offset.x
						>> tmp_joint->offset.y
						>> tmp_joint->offset.z;
				}

				// ucitavanje }
				stream >> tmp;
			}

			else if (tmp == "}") {
				//std::cout << "joint returned: " << joint->name << std::endl;
				count_joint += 1;

				return  joint;
			}
		}
	}

	catch (const std::runtime_error& re) {
		std::cerr << "Runtime error: " << re.what() << std::endl;
	}
	catch (const std::exception& ex) {
		std::cerr << "Error occurred: " << ex.what() << std::endl;
		}

	
	//std::cout << " finish Joint" << std::endl;

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
			stream >> motionData.num_frames;
		}
		else if (trim(tmp) == "Frame")
		{
			float frame_time;
			stream >> tmp >> frame_time;

			int num_frames = motionData.num_frames;
			int num_channels = motionData.num_motion_channels;

			klog.l() << "num frames:" << num_frames;
			klog.l() << "num channels:" << num_channels;

			motionData.data = new float[num_frames * num_channels];

			for (int frame = 0; frame < num_frames; frame++)
			{
				for (int channel = 0; channel < num_channels; channel++)
				{
					float x;
					std::stringstream ss;
					stream >> tmp;
					ss << tmp;
					ss >> x;

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
void BVH::moveJoint(Joint* joint, MOTION* motionData, int frame_starts_index)
{
	// we'll need index of motion data's array with start of this specific joint
	int start_index = frame_starts_index + joint->channel_start;

	// translate indetity matrix to this joint's offset parameters w.r.t parents
	joint->transform = Matrix4f::translation(joint->offset.x,
		joint->offset.y,
		joint->offset.z );

	//std::cout << joint->offset.x << " " << joint->offset.y << " " <<
	//	joint->offset.z << std::endl;



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
			joint->transform = joint->transform * Matrix4f::translation(Vector3f(value, 0, 0));
		}
		if (channel & Yposition)
		{
			joint->transform = joint->transform * Matrix4f::translation(Vector3f(0, value, 0)); 
		}
		if (channel & Zposition)
		{
			joint->transform = joint->transform * Matrix4f::translation(Vector3f(0, 0, value));
		}

		if (channel & Xrotation)
		{
			joint->transform = joint->transform * Matrix4f::rotateX(convertToRadians(value));
		}
		if (channel & Yrotation)
		{
			joint->transform = joint->transform * Matrix4f::rotateY(convertToRadians(value));
		}
		if (channel & Zrotation)
		{
			joint->transform = joint->transform * Matrix4f::rotateZ(convertToRadians(value));
		}
	}

	// then we apply parent's local transfomation matrix to this joint's LTM (local tr. mtx. :)
	if (joint->parent != NULL) {
		joint->transform = joint->parent->transform * joint->transform;
	}

	// when we have calculated parent's matrix do the same to all children
	for (auto& child : joint->children) {
		moveJoint(child, motionData, frame_starts_index);
	}
}

void BVH::moveTo(unsigned frame)
{
	// we calculate motion data's array start index for a frame
	unsigned start_index = frame * motionData.num_motion_channels;

	// recursively transform skeleton
	moveJoint(rootJoint, &motionData, start_index);


}

//for drawing the skeleton
void BVH::drawSkeleton(bool drawSkeleton, int frame = 0) {
	//std::cout << "drawing skeleton" << std::endl;

	frame = frame % motionData.num_frames;

	moveTo(frame);
	bvhToVertices(rootJoint, skeletalVertices, skeletalIndices,0);
	//std::cout << "no sv " << skeletalVertices.size() << std::endl; // 30
	//std::cout << "no si " << skeletalIndices.size() << std::endl; // 58

	for (int i = 0; i < skeletalIndices.size(); i += 2)
	{
		int parent_idx = skeletalIndices[i];
		int child_idx = skeletalIndices[i + 1];

		Vector3f parent = skeletalVertices[parent_idx].xyz();
		Vector3f child = skeletalVertices[child_idx].xyz();
		//std::cout << "parent: " ; parent.print();
		//std::cout << "child: "; child.print();

		glLineWidth(2.0f);
	
		glBegin(GL_LINES);
		glVertex3f(parent.x(), parent.y(), parent.z());
		glVertex3f(child.x(), child.y(), child.z());
		glEnd();

	}
	skeletalVertices.clear();
	skeletalIndices.clear();
}

void BVH::bvhToVertices(Joint* joint, std::vector<Vector4f>& vertices, std::vector<int>&   indices, int parentIndex = 0) {
	// vertex from current joint is in 4-th ROW (column-major ordering)
	Vector4f translatedVertex = joint->transform.getCol(3); //get the translation of the vertex
	//joint->transform.getCol(3).print();

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
	for (auto& child : joint->children) {
		bvhToVertices(child, vertices, indices, myindex);
	}
}

void BVH::printJoint(const Joint* const joint) const
{
	klog.l("joint") << "print joint" << joint->name << joint->channel_start;

	for (std::vector<Joint*>::const_iterator ct = joint->children.begin();
		ct != joint->children.end(); ++ct)
	{
		Joint* _tmp = *ct;

			printJoint(_tmp);
	}

}


void BVH::testOutput() const
{
	if (rootJoint == 0)
		return;

	klog.l() << "output";
	printJoint(rootJoint);

	klog.l() << "num frames: " << motionData.num_frames;
	klog.l() << "num motion channels: " << motionData.num_motion_channels;

	int num_frames = motionData.num_frames;
	int num_channels = motionData.num_motion_channels;


}

#endif