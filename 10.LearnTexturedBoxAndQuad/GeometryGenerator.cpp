//***************************************************************************************
// GeometryGenerator.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Defines a static class for procedurally generating the geometry of 
// common mathematical objects.
//
// All triangles are generated "outward" facing.  If you want "inward" 
// facing triangles (for example, if you want to place the camera inside
// a sphere to simulate a sky), you will need to:
//   1. Change the Direct3D cull mode or manually reverse the winding order.
//   2. Invert the normal.
//   3. Update the texture coordinates and tangent vectors.
//***************************************************************************************

#include "GeometryGenerator.h"
#include <algorithm>
#include <sstream>
#include <fstream>
using namespace DirectX;

GeometryGenerator::MeshData GeometryGenerator::CreateBox(float width, float height, float depth)
{
	MeshData meshData;

	//
	// Create the vertices.
	//

	Vertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face vertex data.
	//							    Normal:x,y,z        Tex:u,v              
	v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f,  0.0f, 1.0f);
	v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f,  0.0f, 0.0f);
	v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f,  1.0f, 0.0f);
	v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f,  1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f);
	v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f);
	v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f);
	v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f);
	v[9] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f);
	v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f,  1.0f, 0.0f);
	v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f,  1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f);
	v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f);
	v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f);
	v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f);
	v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f);
	v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f);
	v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f);

	meshData.Vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	uint32 i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices32.assign(&i[0], &i[36]);

	return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateFromObj(string filename)
{
	MeshData meshData;
	std::ifstream in;
	in.open(filename);
	string fileOutput;

	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT2> texs;
	std::vector<int> index;
	if (in.is_open())
	{
		while (std::getline(in, fileOutput))
			//while(in >> fileOutput)
		{
			//std::cout << fileOutput << std::endl;
			if (fileOutput.substr(0, 2) == "v ")
			{
				std::istringstream v(fileOutput.substr(2));
				XMFLOAT3 p;
				v >> p.x;
				v >> p.y;
				v >> p.z;

				positions.push_back(p);
			}
			else if (fileOutput.substr(0, 3) == "vn ")
			{
				std::istringstream v(fileOutput.substr(3));
				XMFLOAT3 n;
				v >> n.x;
				v >> n.y;
				v >> n.z;
				
				normals.push_back(n);
			}
			else if ((fileOutput.substr(0, 3) == "vt "))
			{
				std::istringstream v(fileOutput.substr(3));
				XMFLOAT2 t;
				v >> t.x;
				v >> t.y;

				texs.push_back(t);
			}
			else if ((fileOutput.substr(0, 2) == "f "))
			{
				std::istringstream v(fileOutput.substr(2));
				std::string r = v.str();
				std::replace(r.begin(), r.end(), '/', ' ');
				v.str(r);
				//std::cout << "After: " << v.str() << std::endl;
				int p1, n1, t1; //triangle 1
				int p2, n2, t2; // triangle 2
				int p3, n3, t3; // triangle 3

				v >> p1; v >> n1;  v >> t1;
				v >> p2; v >> n2;  v >> t2;
				v >> p3; v >> n3;  v >> t3;

				p1--; n1--; t1--;
				p2--; n2--; t2--;
				p3--; n3--; t3--;

				index.push_back(p1);
				index.push_back(n1);
				index.push_back(t1);

				index.push_back(p2);
				index.push_back(n2);
				index.push_back(t2);

				index.push_back(p3);
				index.push_back(n3);
				index.push_back(t3);

			}
		}
	}
	else
	{
	}

	return meshData;
}