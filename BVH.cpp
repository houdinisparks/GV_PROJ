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
//#include "GL/freeglut.h"
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

void BVH::load(const char* bvhfile, const char* meshfile, const char *attachfile) {
	loadBVH(bvhfile);
	loadMesh(meshfile);
	loadAttachments(attachfile);

	computeBindWorldToJointTransforms(rootJoint);
	precomputeVerticesForEachFrame();
	printJoint(rootJoint);
	//updateCurrentJointToWorldTransforms()//this is done in moveJoint step.;

}

void BVH::loadBVH(const std::string& filename)
{
	std::fstream file;
	char buffer[8192];


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
	klog.l("Joints") << "# of Joints: " << m_joints.size();
	std::cout << "loaded finished" << std::endl;
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




}
/*
FOR SKELETON DRAWING
*/
Joint* BVH::loadJoint(std::istream& stream, Joint* parent) //default value of parent == NULL
{
	std::cout << " loading Joint" << std::endl;

	Joint* joint = new Joint;
	joint->parent = parent;

	// load joint name
	std::string* name = new std::string;
	stream >> *name;
	joint->name = name->c_str();
	 m_joints.push_back(joint); //include all joints except end site

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
				m_joints.push_back(tmp_joint);

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

void BVH::computeBindWorldToJointTransforms(Joint *joint) {
	//assume frame 0 is binding pose.

	if (joint->parent != NULL) {

		joint->transform = joint->parent->transform * Matrix4f::translation(joint->offset.x,
			joint->offset.y,
			joint->offset.z);

		joint->bindWorldToJointTransform = joint->transform.inverse();

	}
	else {
		joint->transform = Matrix4f::translation(joint->offset.x,
			joint->offset.y,
			joint->offset.z);

		joint->bindWorldToJointTransform = joint->transform.inverse(); //0 0 0 offset
	}

	for (auto& child : joint->children) {
		computeBindWorldToJointTransforms(child);
	}
}
void BVH::drawSkeleton(bool drawSkeleton, int frame = 0) {
	//std::cout << "drawing skeleton" << std::endl;

	frame = frame % motionData.num_frames;

	moveTo(frame);
	bvhToVertices(rootJoint, skeletalVertices, skeletalIndices, 0);
	glBegin(GL_LINES);
	for (int i = 0; i < skeletalIndices.size(); i += 2)
	{
		int parent_idx = skeletalIndices[i];
		int child_idx = skeletalIndices[i + 1];

		Vector3f parent = skeletalVertices[parent_idx].xyz();
		Vector3f child = skeletalVertices[child_idx].xyz();


		glLineWidth(2.0f);
		glVertex3f(parent.x(), parent.y(), parent.z());
		glVertex3f(child.x(), child.y(), child.z());

	}
	glEnd();
	skeletalVertices.clear();
	skeletalIndices.clear();
}
int currentCompletion = -1;
void BVH::precomputeVerticesForEachFrame() {
	klog.l("Animation") << "Loading Animation...";
	for (float frame = 0; frame < motionData.num_frames; frame++)
	{
		int percentComplete = (int)((frame / motionData.num_frames) * 100);
		
		if (percentComplete % 25 == 0 && currentCompletion != percentComplete) {
			klog.l("Animation") << "Loading Frame: " << percentComplete << "% Complete";
			currentCompletion = percentComplete;
		}
		moveTo(frame);
		std::vector<Vector3f> verts_in_one_frame;

		for (int i = 0; i < m_mesh.vecv.size(); i++) //[vecv1 , vecv2 , vecv3 , ...]
		{
			Vector4f bind_vertex(m_mesh.vecv[i], 1.0f); //static
			Vector4f updated_vertex(0.0f);

			for (int j = 0; j < m_mesh.attachments[i].size(); j += 2) //[ vecv1:[jtidx1,wght1, jtidx2,wght2
			{
				//wT1B1-1p
				int jt_idx = m_mesh.attachments[i][j];

				float jt_weights = m_mesh.attachments[i][j + 1];

				Vector4f update_vertex = jt_weights *
					(m_joints[jt_idx]->transform *
						m_joints[jt_idx]->bindWorldToJointTransform * bind_vertex);

				updated_vertex = updated_vertex + update_vertex;

			}
	
			verts_in_one_frame.push_back(updated_vertex.xyz());
		}
		
		m_mesh_vertices_all_frames.push_back(verts_in_one_frame);
	}

	klog.l("Animation") << "Loading Animation...Finished";
}

/**
Calculates Joint's local transformation matrix for
specified frame starting index; frame starts from 0.
*/
void BVH::moveJoint(Joint* joint, MOTION* motionData, int frame_starts_index)
{
	// we'll need index of motion data's array with start of this specific joint
	int start_index = frame_starts_index + joint->channel_start;

	// translate indetity matrix to this joint's offset parameters w.r.t parents
	joint->transform = Matrix4f::translation(joint->offset.x,
		joint->offset.y,
		joint->offset.z );

	// here we transform joint's local matrix with each specified channel's values
	// which are read from motion data
	for (int i = 0; i < joint->num_channels; i++)
	{

		//motion data is w.r.t to bind pose coordinates

		// channel alias
		const short& channel = joint->channels_order[i];

		// extract value from motion data, each frame with respect to parent
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

//for drawing the mesh
void BVH::updateMesh(int frame) {

	//klog.l("Mesh") << "updating mesh";
	frame = frame % motionData.num_frames;
	m_mesh.currentVertices = m_mesh_vertices_all_frames[frame];
	

}

void BVH::drawMesh(bool drawMesh, int frame = 0) {
	frame = frame % motionData.num_frames;

	updateMesh(frame);
	m_mesh.draw();

	//some function to update the vertices

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

void BVH::loadMesh(const char *meshFile) {
	klog.l("Mesh") << "loading mesh";
	m_mesh.load(meshFile); //populates the faces, the currentVertices
	klog.l("Mesh") << "finish loading mesh";
}

void BVH::loadAttachments(const char *attachmentFile) {
	klog.l("Attachments") << "loading attachmehnts";
	m_mesh.loadAttachments(attachmentFile);
	klog.l("Attachments") << "finish loading attachmehnts";
}

//----------------------------------------------------------------------
//FOR DEBUGGING PURPOSES -----------------------------------------------
//-----------------------------------------------------------------------
void BVH::printJoint(Joint* joint) const
{
	klog.l("joint")  << joint->name << joint->channel_start;
	klog.l(joint->name) << "WtoJ \n";
	joint->bindWorldToJointTransform.print();


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
void BVH::init_mesh_collide(){
    m_mesh.initiate_collide();
}

bool BVH::check_mesh_collide(){
    return m_mesh.check_collide();
}
#endif
