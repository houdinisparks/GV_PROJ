#include "SkeletalModel.h"

#include <FL/Fl.H>
#include <vector>
#include <algorithm>
#include <iterator>
#include <set>
#include <iostream>
#include <sstream>



using namespace std;

//forward declarations;
void traverseJointHeirarchy(Joint* curren_joint, MatrixStack& matrixstack);
void traverseJointHeirarchy_v2(MatrixStack& matrixstack, Joint * curren_joint);
void traverseJointHeirarchy_v3(MatrixStack& matrixstack, Joint * curren_joint);
void traverseBoneHeirarchy(Joint* current_joint, MatrixStack& matrixstack);

void SkeletalModel::load(const char *skeletonFile, const char *meshFile, const char *attachmentsFile)
{
	loadSkeleton(skeletonFile);

	m_mesh.load(meshFile);
	m_mesh.loadAttachments(attachmentsFile, m_joints.size());

	computeBindWorldToJointTransforms();
	updateCurrentJointToWorldTransforms();
}

void SkeletalModel::draw(Matrix4f cameraMatrix, bool skeletonVisible)
{
	// draw() gets called whenever a redraw is required
	// (after an update() occurs, when the camera moves, the window is resized, etc)

	m_matrixStack.clear();
	m_matrixStack.push(cameraMatrix);

	if (skeletonVisible)
	{
		drawJoints();
		
		drawSkeleton();
	}
	else
	{
		// Clear out any weird matrix we may have been using for drawing the bones and revert to the camera matrix.
		glLoadMatrixf(m_matrixStack.top());

		// Tell the mesh to draw itself.
		m_mesh.draw();
	}
}



// s is a reference
//function to split the string
template<typename Out>
void split(const std::string &s, char delim, Out result) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}


std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, back_inserter(elems));
	return elems;
}



void SkeletalModel::loadSkeleton(const char* filename)
{
	// Load the skeleton from file here.
	// each line := <field 1 float> <field 2 float> <field 3 float> <field 4>
	// each line is a joint, which you should load a pointer to
	// field 1 - 3 give the joints translation relative to parent joint

	//load each joint as as a pointer. since its a pointer, we need to 
	// to initialize it with the "new" keyword to allocate space in memory for this object
	cout << "parsing file..." << endl;

	//cannot use cin here since there are multiple files. use ifstream.
	ifstream infile;
	infile.open(filename);

	string line;
	while (getline(infile, line)) {

		Joint* joint = new Joint;

		if (line != "") {
			vector<string> fields = split(line, ' '); // [f1,f2,f3,f4]
			cout << "spliiting line " << fields[0] << " " << fields[1] << " " << fields[2] << endl;
			joint->transform = Matrix4f::translation(stof(fields[0]), stof(fields[1]), stof(fields[2]));

			try {
				if (stoi(fields[3]) == -1) {
					m_joints.push_back(joint);
				}
				else {
					m_joints.push_back(joint);
					m_joints[stoi(fields[3])]->children.push_back(joint);
				}

			}
			catch (const out_of_range& oor) {
				cout << "Node's parent is out of range!";
			}


			//add to parent if applicable
			//f4 is the index of the parent. root node is -1
		}
	}

	cout << "parsed finish. number of joints: " << m_joints.size() << endl;
	m_rootJoint = m_joints[0];
	//all joints


}

//global varialbes






void SkeletalModel::drawJoints()
{
	// Draw a sphere at each joint. You will need to add a recursive helper function to traverse the joint hierarchy.
	// recursive function outputs the joint transform

	// We recommend using glutSolidSphere( 0.025f, 12, 12 )
	// to draw a sphere of reasonable size.

	m_matrixStack.push(m_joints[0]->transform);
	glutSolidSphere(0.025f, 12, 12);
	cout << "drawing sphere" << endl;
	int children = m_joints[0]->children.size();
	for (int children_idx = 0; children_idx < children; children_idx++) {

		traverseJointHeirarchy(m_joints[0]->children[children_idx], m_matrixStack); //traverse down each and draws at each join

	}
	m_matrixStack.pop();
	
	// You are *not* permitted to use the OpenGL matrix stack commands
	// (glPushMatrix, glPopMatrix, glMultMatrix).
	// You should use your MatrixStack class
	// and use glLoadMatrix() before your drawing call.

}
void SkeletalModel::drawSkeleton()
{
	// Draw boxes between the joints. You will need to add a recursive helper function to traverse the joint hierarchy.
	cout << "drawing skeleton"<<endl;
	
	m_matrixStack.push(m_rootJoint->transform);

	int children = m_rootJoint->children.size();
	for (int children_idx = 0; children_idx < children; children_idx++) {

		traverseBoneHeirarchy(m_joints[0]->children[children_idx], m_matrixStack); //traverse down each and draws at each join
	}
	
	m_matrixStack.pop();
	
}

void traverseBoneHeirarchy(Joint* current_joint, MatrixStack& matrixstack) {
	
	//you must first push the transform in the matrix stack --> draw --> pop --> push next children.
	//these transfomations are not part of the heirarchy.

	matrixstack.push(current_joint->transform);

	for (vector<Joint*>::iterator it = current_joint->children.begin();
	it != current_joint->children.end();
		it++) {

		traverseBoneHeirarchy(*it, matrixstack);

	}

	//the box/bones will be drawing from parent to current_joint. therefore the matrixstack.top()
	//have to correspond to the parent
	matrixstack.pop(); //therefore need to pop first.

	// result_vect = translate_matrix <dot> vector https://en.wikipedia.org/wiki/Translation_(geometry)
	Matrix4f translate_z = Matrix4f::translation(0, 0, 0.5f); //translate by 05

	float distance = Vector3f(current_joint->transform(0, 3),
		current_joint->transform(1, 3),
		current_joint->transform(2, 3)).abs(); //distance from parent's joint, 3 is last col

	Matrix4f scale_smaller = Matrix4f::scaling(1.0f / 20, 1.0f / 20, distance);

	//remember the translation matrix is relative to the parent's coordinate frame!
	//the transformations are trnslations relative to parents joint
	Vector3f z = Vector3f(current_joint->transform(0, 3), current_joint->transform(1, 3), current_joint->transform(2, 3)).normalized();
	Vector3f rnd(0.0f, 0.0f, 1.0f);
	Vector3f y = Vector3f::cross(z, rnd).normalized();
	Vector3f x = Vector3f::cross(y, z).normalized();

	//rotate z to align
	Matrix4f rotation = Matrix4f::identity();
	rotation.setSubmatrix3x3(0, 0, Matrix3f(x, y, z));

	matrixstack.push(rotation);
	matrixstack.push(scale_smaller);
	matrixstack.push(translate_z);
	

	cout << "drawing cube" << endl;

	glutSolidCube(1.0f);

	//this cube will be automataically transformed based on the matrixstack, as glLoadMatrix is also called.

	matrixstack.pop();
	matrixstack.pop();
	matrixstack.pop();

	return;


}

// improvements that can be made. use functions as argument to traverse joint and implement
// different logic

void traverseJointHeirarchy(Joint* curren_joint, MatrixStack& matrixstack) {

	matrixstack.push(curren_joint->transform);
	//glLoadMatrixf;  dont have to call, already called in matrixstack.push() method
	cout << "drawing sphere" << endl;
	
	glutSolidSphere(0.025f, 12, 12);

	//check if no more children
	if (curren_joint->children.size() == 0) {
		//go back up
		matrixstack.pop();
		return;
	}
	else {
		for (vector<Joint*>::iterator it = curren_joint->children.begin();
		it != curren_joint->children.end();
			it++) {

			traverseJointHeirarchy(*it, matrixstack);

		}
		//go back up
		matrixstack.pop();
		return;
	}
}

void SkeletalModel::setJointTransform(int jointIndex, float rX, float rY, float rZ)
{
	// Set the rotation part of the joint's transformation matrix based on the passed in Euler angles.
	m_joints[jointIndex]->transform.setSubmatrix3x3(0, 0,
		((Matrix4f::rotateX(rX) * Matrix4f::rotateY(rY) * Matrix4f::rotateZ(rZ)).getSubmatrix3x3(0, 0)));
		

}


void SkeletalModel::computeBindWorldToJointTransforms()
{
	// 2.3.1. Implement this method to compute a per-joint transform from
	// world-space to joint space in the BIND POSE.
	//
	// Note that this needs to be computed only once since there is only
	// a single bind pose.
	//
	// This method should update each joint's bindWorldToJointTransform.
	// You will need to add a recursive helper function to traverse the joint hierarchy.
	
	cout << "computing bind world to joint transformations" <<endl;
	
	m_matrixStack.push(m_rootJoint->transform);
	m_rootJoint->bindWorldToJointTransform = m_matrixStack.top().inverse();
	int children = m_rootJoint->children.size();
	for (int children_idx = 0; children_idx < children; children_idx++) {

		traverseJointHeirarchy_v2(m_matrixStack, m_joints[0]->children[children_idx]); //traverse down each and draws at each join
	}
	m_matrixStack.pop();
}

void traverseJointHeirarchy_v2(MatrixStack& matrixstack, Joint* curren_joint) {

	matrixstack.push(curren_joint->transform);
	curren_joint->bindWorldToJointTransform = matrixstack.top().inverse(); // world space to joint local space

	//glLoadMatrixf;  dont have to call, already called in matrixstack.push() method
	cout << "computing bind transformation v2" << endl;

	//check if no more children
	if (curren_joint->children.size() == 0) {
		//go back up
		matrixstack.pop();
		return;
	}
	else {
		for (vector<Joint*>::iterator it = curren_joint->children.begin();
		it != curren_joint->children.end();
			it++) {

			traverseJointHeirarchy_v2(matrixstack , *it);

		}
		//go back up
		matrixstack.pop();
		return;
	}

}


void SkeletalModel::updateCurrentJointToWorldTransforms()
{
	// 2.3.2. Implement this method to compute a per-joint transform from
	// joint space to world space in the CURRENT POSE.
	//
	// The current pose is defined by the rotations you've applied to the
	// joints and hence needs to be *updated* every time the joint angles change.
	//
	// This method should update each joint's bindWorldToJointTransform.
	// You will need to add a recursive helper function to traverse the joint hierarchy.
	cout << "computing current joint to world transformations" << endl;
	m_matrixStack.clear();
	m_matrixStack.push(m_rootJoint->transform);
	m_rootJoint->currentJointToWorldTransform = m_matrixStack.top();

	int children = m_rootJoint->children.size();
	for (int children_idx = 0; children_idx < children; children_idx++) {

		traverseJointHeirarchy_v3(m_matrixStack, m_joints[0]->children[children_idx]); //traverse down each and draws at each join
	}

	m_matrixStack.pop();

}

void traverseJointHeirarchy_v3(MatrixStack& matrixstack, Joint* curren_joint) {

	matrixstack.push(curren_joint->transform);
	curren_joint->currentJointToWorldTransform = matrixstack.top(); // joint space to world space

	cout << "updating transformation v3" << endl;

	//check if no more children
	if (curren_joint->children.size() == 0) {
		//go back up
		matrixstack.pop();
		return;
	}
	else {
		for (vector<Joint*>::iterator it = curren_joint->children.begin();
		it != curren_joint->children.end();
			it++) {

			traverseJointHeirarchy_v3(matrixstack, *it);

		}
		//go back up
		matrixstack.pop();
		return;
	}

}

void SkeletalModel::updateMesh()
{
	// 2.3.2. This is the core of SSD.
	// Implement this method to update the vertices of the mesh
	// given the current state of the skeleton.
	// You will need both the bind pose world --> joint transforms.
	// and the current joint --> world transforms.
	
	//for each vertex (in world space bind pose), do transformations for each joint.
	cout << "# of current vertices: " << m_mesh.currentVertices.size();
	cout << "# of bind vertices: " << m_mesh.bindVertices.size();

	cout << "updating mesh" << endl;
	for (int i = 0; i < m_mesh.bindVertices.size(); i++)
	{
		Vector4f bind_vertex(m_mesh.bindVertices[i] , 1.0f); //static
		Vector4f updated_vertex(0.0f);

		//world --> joint transforms
		for (int j = 0; j < m_mesh.attachments[i].size(); j++)
		{
			//wT1B1-1p
			//Vector4f intm_joint = (m_joints[j+1] -> bindWorldToJointTransform) * bind_vertex; //B1-1*p
			//Vector4f update_vertex = m_mesh.attachments[i][j] *
			//	((m_joints[j + 1]->currentJointToWorldTransform) * intm_joint);
			
			Vector4f update_vertex = m_mesh.attachments[i][j] *
				(m_joints[j + 1]->currentJointToWorldTransform *
					m_joints[j + 1]->bindWorldToJointTransform * bind_vertex);
			
			updated_vertex = updated_vertex + update_vertex;
			
		}

		m_mesh.currentVertices[i] = updated_vertex.xyz();
	}
	

}

