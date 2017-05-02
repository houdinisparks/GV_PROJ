#include "Mesh.h"
#include <iterator>
#include <vector>
#include "Joint.h"
#include "Logger.h"

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

	//while (getline(infile, line)) { //when eof is reached, while output False
	//							 //cout << "loading line" << "\n";
	//							 //delimit the strings with "
	//	if (line != "")	//if line not empty
	//	{
	//		vector<string> temp_stor = anonymous::split(line, ' ');
	//		string type = temp_stor[0];

	//		//for testing, print the contents of the vector
	//		/*for (vector<string>::const_iterator i = temp_stor.begin();
	//		i != temp_stor.end(); ++i) {
	//		cout << *i << "\n";
	//		}*/

	//		if (type == "v") {
	//			vecv.push_back(Vector3f(atof(temp_stor[1].c_str()),
	//				atof(temp_stor[2].c_str()),
	//				atof(temp_stor[3].c_str())));
	//		}
	//		else if (type == "vn") {
	//			vecn.push_back(Vector3f(atof(temp_stor[1].c_str()),
	//				atof(temp_stor[2].c_str()),
	//				atof(temp_stor[3].c_str())));
	//		}
	//		else if (type == "f") {
	//			vector<unsigned> temp_vec;
	//			for (int i = 1; i < temp_stor.size(); i++)
	//			{
	//				char *token = strtok(&temp_stor[i][0], "/");
	//				int index = 0;
	//				while (token != NULL) {
	//					//cout << stoi(token) << "\n";
	//					if (index != 1) { //ignore middle element
	//						temp_vec.push_back(stoi(token));
	//					}
	//					token = strtok(NULL, "/");
	//					index += 1;
	//				}
	//			}
	//			vecf.push_back(temp_vec);
	//		}

	//	}
	//}

	while (getline(infile, line)) {
		if (line != "") {
			vector<string> temp_stor = anonymous::split(line, ' ');
			string type = temp_stor[0];

			if (type == "v") {
				vecv.push_back(Vector3f(atof(temp_stor[1].c_str()),
					atof(temp_stor[2].c_str()),
					atof(temp_stor[3].c_str())));
			}

			else if (type == "f") {
				vector<int> temp_vec;
				for (int i = 1; i < temp_stor.size(); i++)
				{
					char *token = strtok(&temp_stor[i][0], "/");
					
						temp_vec.push_back(stoi(token));

					
				}
				//vecf.push_back(temp_vec);

				faces.push_back(temp_vec);

			}
		}
	}
	klog.l("Mesh") << "# of vertices" << vecv.size();

	//cout << "number of faces: " << faces.size();
	//cout << "  number of vertices: " << bindVertices.size() << endl;
	// make a copy of the bind vertices as the current vertices
	currentVertices = vecv;
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


//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	// Here we use the first color entry as the diffuse color
//	GLfloat diffColors[4] = {  0.5, 0.5, 0.9, 1.0 }
//;
//	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffColors);
//
//
//	// Define specular color and shininess
//	GLfloat specColor[] = { 1.0, 1.0, 1.0, 1.0 };
//	GLfloat shininess[] = { 100.0 };
//
//	// Note that the specular color and shininess can stay constant
//	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specColor);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);


	//glBegin(GL_TRIANGLES);

	//for (unsigned int j = 0; j < vecf.size(); j++)
	//{
	//	//< <a,c,d,f,g,i> .. >
	//	int c = vecf[j][1], a = vecf[j][0], f = vecf[j][3],
	//		d = vecf[j][2], i = vecf[j][5], g = vecf[j][4];
	//	//cout << a << " " << c << " " << d << " " << f << " " << g << " " << i << " " << "\n";
	//	glNormal3d(vecn[c - 1][0], vecn[c - 1][1], vecn[c - 1][2]);
	//	glVertex3d(vecv[a - 1][0], vecv[a - 1][1], vecv[a - 1][2]);
	//	glNormal3d(vecn[f - 1][0], vecn[f - 1][1], vecn[f - 1][2]);
	//	glVertex3d(vecv[d - 1][0], vecv[d - 1][1], vecv[d - 1][2]);
	//	glNormal3d(vecn[i - 1][0], vecn[i - 1][1], vecn[i - 1][2]);
	//	glVertex3d(vecv[g - 1][0], vecv[g - 1][1], vecv[g - 1][2]);

	//}
	//glEnd();
	//GLfloat colour[4] = { 1.0f,0,0,1.0f };
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colour);

	glBegin(GL_TRIANGLES);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT_AND_BACK);
	for (int i = 0; i < faces.size(); i++)
	{
		//int a,d,g are the indices of the vertices.
		int a = faces[i][0];
		int d = faces[i][1];
		int g = faces[i][2];

		//minus 1 because the faces index vertices and normals from 1
		Vector3f a_vertex = currentVertices[a - 1];
		Vector3f d_vertex = currentVertices[d - 1];
		Vector3f g_vertex = currentVertices[g - 1];
		Vector3f normal = Vector3f::cross(d_vertex - a_vertex, g_vertex - a_vertex).normalized();

		//int b,e,h are the respective normals. however, we only need to generate one normal per triangle.

		glNormal3d(normal[0], normal[1], normal[2]);
		glColor3f(0.5f,0.5f,0.5f);
		glVertex3d(a_vertex[0], a_vertex[1], a_vertex[2]);
		glColor3f(0.5f, 0.5f, 0.5f);

		glVertex3d(d_vertex[0], d_vertex[1], d_vertex[2]);
		glColor3f(0.5f, 0.5f, 0.5f);


		glVertex3d(g_vertex[0], g_vertex[1], g_vertex[2]);

		if (faces[i].size() == 4) {
			//int a,d,g are the indices of the vertices.
			int a = faces[i][0];
			int d = faces[i][2];
			int g = faces[i][3];

			//minus 1 because the faces index vertices and normals from 1
			Vector3f a_vertex = currentVertices[a - 1];
			Vector3f d_vertex = currentVertices[d - 1];
			Vector3f g_vertex = currentVertices[g - 1];
			normal = Vector3f::cross(d_vertex - a_vertex, g_vertex - a_vertex).normalized();

			//int b,e,h are the respective normals. however, we only need to generate one normal per triangle.

			glNormal3d(normal[0], normal[1], normal[2]);
			glColor3f(0.5f, 0.5f, 0.5f);
			glVertex3d(a_vertex[0], a_vertex[1], a_vertex[2]);
			glColor3f(0.5f, 0.5f, 0.5f);
			glVertex3d(d_vertex[0], d_vertex[1], d_vertex[2]);
			glColor3f(0.5f, 0.5f, 0.5f);

			glVertex3d(g_vertex[0], g_vertex[1], g_vertex[2]);

		}

	}
	glEnd();



}

void Mesh::loadAttachments(const char* filename)
{
	// 2.2. Implement this method to load the per-vertex attachment weights
	// this method should update m_mesh.attachments

	//each line in .attach file corresponds to a vertice loaded from .obj file.
	//each ith field depicts the weight for the (ith + 1) joint. ( assume zero based ordering)
	ifstream infile;
	infile.open(filename);
	int prev_idx = 0;
	int cur_idx = 0;
	bool first = true;
	vector<float> weights;

	string line;
	while (getline(infile, line)) {
		if (line[0] == '[') {


			char* token = strtok(&line[0], " ,[]'");
			// [jt_idx, jt_wght, jt_idx2, jt_wgt2,...]i

								// 0 ,1   2    , 3 ,         4           ]
			int count_token_idx = 0; //[0, u'Spine1', 12, 0.022650764395880884]
			while (token != NULL) {
				if (count_token_idx == 0) {
					cur_idx = stoi(token);

					if (cur_idx - prev_idx == 1) {
						//klog.l("Weights") << weights[0] << " " << weights[1]; //<< " " << weights[2] ;
						prev_idx = cur_idx;
						attachments.push_back(weights);
						weights = vector<float>{};
					}

				}
				else if (count_token_idx == 3) {
					weights.push_back(stoi(token));
				}
				else if (count_token_idx == 4) {
					weights.push_back(stof(token));
				}
				count_token_idx++;
				token = strtok(NULL, " ,[]'");
			}
		}
	}

	klog.l("Weights") << weights[0] << " " << weights[1]; //<< " " << weights[2] ;
	attachments.push_back(weights); //add the last weight

	klog.l("Attachments") << "# of attachmens: " << attachments.size();

	//ifstream infile;
	//infile.open(filename);
	//string line;
	//while (getline(infile, line)) {
	//	if (line != " ") {

	//		vector<string> tokens = anonymous::split(line, ' ');
	//		vector<float> weights;
	//		for (int i = 0; i < tokens.size(); i++)
	//		{
	//			weights.push_back(stof(tokens[i]));

	//		}
	//		attachments.push_back(weights);
	//	}
	//}
	//cout <
	//cout << "attachments: " << attachments.size() << endl;
	//cout << "weights: " << attachments[0].size() << endl;
}



//string line;
//while (getline(infile, line)) {
//	if (line != "") {
//		vector<string> temp_stor = anonymous::split(line, ' ');
//		string type = temp_stor[0];

//		if (type == "v") {
//			bindVertices.push_back(Vector3f(atof(temp_stor[1].c_str()),
//				atof(temp_stor[2].c_str()),
//				atof(temp_stor[3].c_str())));
//		}

//		else if (type == "f") {
//			faces.push_back(Tuple3u(stoi(temp_stor[1]), stoi(temp_stor[2]), stoi(temp_stor[3])));

//		}
//	}
//}