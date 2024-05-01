#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;
using namespace std;
DirectX::XMFLOAT3 Position;

std::vector<DirectX::XMFLOAT3> vertices;

DirectX::XMFLOAT3 Normal;

std::vector<DirectX::XMFLOAT3> normals;

DirectX::XMFLOAT3 TangentU;
DirectX::XMFLOAT2 TexC;

std::vector<DirectX::XMFLOAT2> texs;
std::vector<int> indices;


struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
}; 

std::vector<Vertex> Mesh;
std::vector<int> mesh_indices;

int main()
{
	using uint32 = std::uint32_t;
	string filename = "box-triangle.obj";
	ifstream in;

	in.open(filename);
	string fileOutput;
	string token; // parse /
	size_t pos = -1;
	if (in.is_open())
	{
		while (std::getline(in, fileOutput))
			//while(in >> fileOutput)
		{
			//std::cout << fileOutput << std::endl;
			if (fileOutput.substr(0, 2) == "v ")
			{
				std::istringstream v(fileOutput.substr(2));
				double x, y, z;
				v >> x;
				v >> y;
				v >> z;

				Position.x = x;
				Position.y = y;
				Position.z = z;
				vertices.push_back(Position);
			}
			else if (fileOutput.substr(0, 3) == "vn ")
			{
				std::istringstream v(fileOutput.substr(3));
				double x, y, z;
				v >> x;
				v >> y;
				v >> z;
				Normal.x = x;
				Normal.y = y;
				Normal.z = z;
				normals.push_back(Normal);
			}
			else if ((fileOutput.substr(0, 3) == "vt "))
			{
				std::istringstream v(fileOutput.substr(3));
				double x, y;
				v >> x;
				v >> y;

				TexC.x = x;
				TexC.y = y;
				texs.push_back(TexC);
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

				indices.push_back(p1);
				indices.push_back(n1);
				indices.push_back(t1);

				indices.push_back(p2);
				indices.push_back(n2);
				indices.push_back(t2);

				indices.push_back(p3);
				indices.push_back(n3);
				indices.push_back(t3);
				std::cout << p1 << " "  << n1 << " " << t1 << "  "
						<< p2 << " " << n2 << " " << t2 << "  "
						<< p3 << " " << n3 << " " << t3 << std::endl;
			}
		}
	}
	else
	{
		std::cout << "File " << filename << "Failed to open" << std::endl;
	}
	std::cout << "Vertices" << std::endl;
	for (auto i : vertices)
	{
		std::cout << " x:" << i.x
			<< " y:" << i.y
			<< " z:" << i.z << endl;
	}
	std::cout << "Normals" << std::endl;
	for (auto i : normals)
	{
		std::cout << " x:" << i.x
			<< " y:" << i.y
			<< " z:" << i.z << endl;
	}
	std::cout << "Tex" << std::endl;
	for (auto i : texs)
	{
		std::cout << " x:" << i.x
			<< " y:" << i.y
			<<  endl;
	}

	Vertex meshdata;
	int size = indices.size() / 3;
	//std::cout << " x:" << size << endl;
	for (int i = 0; i < size ; i+3)
	{
		int position_index = indices[i];
		//std::cout << " x:" << vertices[position_index].x << endl;
		int tex_index = indices[i + 1];
		int normal_index = indices[i + 2];
		meshdata.Position = vertices[position_index];
		meshdata.Normal = normals[normal_index];
		meshdata.TexC = texs[tex_index];

		Mesh.push_back(meshdata);

		mesh_indices.push_back(i);
	}


	int x, y, z;
	x = Mesh[0].Position.x;
	y = Mesh[0].Position.y;
	z = Mesh[0].Position.z;

	std::cout << " Position:" << " x:" << x <<
		" y:" << y <<
		" z:" << z << endl;
		
		

	//for (auto i : Mesh)
	//{
	//	std::cout << " Position:" << " x:" << i.Position.x << 
	//		" y:" << i.Position.y<< 
	//		" z:" << i.Position.z
	//		<< " Normal:" << i.Normal.x << i.Normal.y << i.Normal.z
	//		<< " Tex: " << i.TexC.x << i.TexC.y
	//		<< endl;
	//}

	in.close();

	return 0;


}