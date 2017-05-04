#include "Mesh.h"
#include <iterator>
#include <vector>
#include <random>
#include "Joint.h"
#include "Logger.h"
#define M_PI           3.14159265358979323846
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
                faces_c.push_back(Vector3f(0.f,0.f,0.f));
			}
		}
	}
	klog.l("Mesh") << "# of vertices" << vecv.size();

	//cout << "number of faces: " << faces.size();
	//cout << "  number of vertices: " << bindVertices.size() << endl;
	// make a copy of the bind vertices as the current vertices
	currentVertices = vecv;
}

void Mesh::initiate_collide()
{
        collide_on = true;
}

bool Mesh::check_collide()
{
        return collide_on;
}

void Mesh::draw()
{
	//glEnable(GL_DEPTH_TEST);   // Depth testing must be turned on
	glEnable(GL_LIGHTING);     // Enable lighting calculations
	glEnable(GL_LIGHT0);       // Turn on light #0.

	//glEnable(GL_NORMALIZE);

	//glShadeModel(GL_SMOOTH);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	glBegin(GL_TRIANGLES);

    Vector2f max_min = getYmaxmin();
	for (int i = 0; i < faces.size(); i++)
	{
		//int a,d,g are the indices of the vertices.
        // if(collide_on){
    	// 	float change = (float)(rand() % 1000000) / 1000000.f;
        //     if(change < 0.000002){
        //         faces_c[i] = Vector3f(1.0f,1.0f,1.0f);
        //     }
        // }

		int a,d,g;
        a = faces[i][0];
    	d = faces[i][1];
    	g = faces[i][2];

		//minus 1 because the faces index vertices and normals from 1
		Vector3f a_vertex = currentVertices[a - 1];
		Vector3f d_vertex = currentVertices[d - 1];
		Vector3f g_vertex = currentVertices[g - 1];
		Vector3f normal = Vector3f::cross(d_vertex - a_vertex, g_vertex - a_vertex).normalized();

        float y_center = (a_vertex[1] + d_vertex[1] + g_vertex[1])/3.f;
        // std::cout << y_center <<" " <<ymax<<std::endl;
        // float y_normal = 1.f/sqrt(2*M_PI*30*30)*exp(-(pow((y_center - ymax),2.0)/2*30*30));
		float prob = exp(y_center) / ((exp(max_min[0]) - exp(max_min[1])) * 1000);
        // std::cout << y_normal << std::endl;

        if(collide_on){
    		float change = (float)((rand() % 10000) / 10000.f);
            if(change < prob){
                faces_c[i] = Vector3f(1.0f,1.0f,1.0f);
            }
        }

		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, faces_c[i]);
		glMaterialfv(GL_FRONT, GL_SPECULAR, faces_c[i]);
		glMaterialf(GL_FRONT, GL_SHININESS, 10.0f);

		//int b,e,h are the respective normals. however, we only need to generate one normal per triangle.
		if (faces_c[i][0] == 1.0f && faces_c[i][1] == 1.0f) {
			//glColor3f(faces_c[i][0], faces_c[i][1], faces_c[i][2]);

			glNormal3d(normal[0], normal[1], normal[2]);
			glVertex3f(a_vertex[0], a_vertex[1], a_vertex[2]);
			glVertex3f(d_vertex[0], d_vertex[1], d_vertex[2]);
			glVertex3f(g_vertex[0], g_vertex[1], g_vertex[2]);

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

				glNormal3f(normal[0], normal[1], normal[2]);
				glVertex3f(a_vertex[0], a_vertex[1], a_vertex[2]);
				glVertex3f(d_vertex[0], d_vertex[1], d_vertex[2]);
				glVertex3f(g_vertex[0], g_vertex[1], g_vertex[2]);


			}
		}
		

	}
	glEnd();

	glDisable(GL_LIGHTING);     // Enable lighting calculations
	glDisable(GL_LIGHT0);       // Turn on light #0.

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
}


Vector2f Mesh::getYmaxmin(){
    Vector2f  max_min= Vector2f(0,100);
    for(int i = 0; i < faces.size() ; i+=2){
        int a,d,g;
        a = faces[i][0];
    	d = faces[i][1];
    	g = faces[i][2];
        Vector3f a_vertex = currentVertices[a - 1];
		Vector3f d_vertex = currentVertices[d - 1];
		Vector3f g_vertex = currentVertices[g - 1];
        float y_center = (a_vertex[1] + d_vertex[1] + g_vertex[1])/3.f;
        if(y_center > max_min[0]){
            max_min[0] = y_center;
        }
        if(y_center < max_min[1]){
            max_min[1] = y_center;
        }
    }
	return max_min;
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

