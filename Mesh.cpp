#include "Mesh.h"
#include <iterator>
#include <vector>
#include "Joint.h"

using namespace std;

namespace anonymous
{

	template<typename Out>
	void split(const std::string &s, char delim, Out result) {
		stringstream ss;
		ss.str(s);
		string item;
		while (getline(ss, item, delim)) {
			*(result++) = item;
		}
	}


	vector<string> split(const string &s, char delim) {
		vector<string> elems;
		split(s, delim, back_inserter(elems));
		return elems;
	}

}

void Mesh::load(const char* filename)
{
	// 2.1.1. load() should populate bindVertices, currentVertices, and faces

	// Add your code here.
	ifstream infile;
	infile.open(filename);

	string line;
	while (getline(infile, line)) {
		if (line != "") {
			vector<string> temp_stor = anonymous::split(line, ' ');
			string type = temp_stor[0];

			if (type == "v") {
				bindVertices.push_back(Vector3f(atof(temp_stor[1].c_str()),
					atof(temp_stor[2].c_str()),
					atof(temp_stor[3].c_str())));
			}

			else if (type == "f") {
				faces.push_back(Tuple3u(stoi(temp_stor[1]), stoi(temp_stor[2]), stoi(temp_stor[3])));

			}
		}
	}

	//cout << "number of faces: " << faces.size();
	//cout << "  number of vertices: " << bindVertices.size() << endl;
	// make a copy of the bind vertices as the current vertices
	currentVertices = bindVertices;
}



void Mesh::draw()
{
	// Since these meshes don't have normals
	// be sure to generate a normal per triangle.
	// Notice that since we have per-triangle normals
	// rather than the analytical normals from
	// assignment 1, the appearance is "faceted".

	//take reference from assignment 0 on how to draw the triangles  a/b/c d/e/f g/h/i.
	//cout << "drawing.. mesh" << endl;
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < faces.size(); i++)
	{
		//int a,d,g are the indices of the vertices.
		int a = faces[i][0];
		int d = faces[i][1];
		int g = faces[i][2];

		Vector3f a_vertex = currentVertices[a - 1]; //-1 beause indexes start from 0
		Vector3f d_vertex = currentVertices[d - 1];
		Vector3f g_vertex = currentVertices[g - 1];
		Vector3f normal = Vector3f::cross(d_vertex - a_vertex, g_vertex - a_vertex).normalized();

		//int b,e,h are the respective normals. however, we only need to generate one normal per triangle.

		glNormal3d(normal[0], normal[1], normal[2]);
		glVertex3d(a_vertex[0], a_vertex[1], a_vertex[2]);
		glVertex3d(d_vertex[0], d_vertex[1], d_vertex[2]);
		glVertex3d(g_vertex[0], g_vertex[1], g_vertex[2]);
		//minus 1 because the faces index vertices and normals from 1


	}
	glEnd();



}

void Mesh::loadAttachments(const char* filename, int numJoints)
{
	// 2.2. Implement this method to load the per-vertex attachment weights
	// this method should update m_mesh.attachments

	//each line in .attach file corresponds to a vertice loaded from .obj file.
	//each ith field depicts the weight for the (ith + 1) joint. ( assume zero based ordering)
	ifstream infile;
	infile.open(filename);
	string line;
	while (getline(infile, line)) {
		if (line != " ") {

			vector<string> tokens = anonymous::split(line, ' ');
			vector<float> weights;
			for (int i = 0; i < tokens.size(); i++)
			{
				weights.push_back(stof(tokens[i]));

			}
			attachments.push_back(weights);
		}
	}
	cout << "attachments: " << attachments.size() << endl;
	cout << "weights: " << attachments[0].size() << endl;
}



