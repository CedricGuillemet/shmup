#pragma once
#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "maths.h"

inline std::vector<uint8_t> ReadFile(const std::string& filePathName)
{
	std::vector<uint8_t> res;
	FILE* fp = fopen(filePathName.c_str(), "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		res.resize(ftell(fp));
		fseek(fp, 0, SEEK_SET);
		fread(res.data(), res.size(), 1, fp);
		fclose(fp);
	}
	return res;
}

class Mesh
{
public:
    Mesh() {}


    bool LoadObj(const std::string& path, const std::string& fileName)
    {
		mPositions.clear();
		mFaces.clear();

		std::vector<uint8_t> buffer = ReadFile("D:\\Dev\\shmup\\msh\\testRender.obj");

		int imgx, imgy, imgn;
		stbi_uc* img = stbi_load("D:\\Dev\\shmup\\msh\\testRender.png", &imgx, &imgy, &imgn, 4);

		//std::vector<vec_t> positions;
		//std::vector<vec_t> normals;
		std::vector<vec_t> uvs;

		char* src = (char*)buffer.data();
		char* srcEnd = (char*)buffer.data() + buffer.size();

		char row[512];
		int face[32];
		int nv;
		float x, y, z;
		int vertCount = 0;
		while (src < srcEnd)
		{
			// Parse one row
			row[0] = '\0';
			src = ParseRow(src, srcEnd, row, sizeof(row) / sizeof(char));
			// Skip comments
			if (row[0] == '#') continue;
			if (row[0] == 'v' && row[1] != 'n' && row[1] != 't')
			{
				// Vertex pos
				sscanf(row + 1, "%f %f %f", &x, &y, &z);
				//addVertex(x, y, z, vcap);
				mPositions.push_back(vec_t(x, y, z));
			}
			if (row[0] == 'v' && row[1] == 't')
			{
				// Vertex texture
				sscanf(row + 2, "%f %f %f", &x, &y, &z);
				//addVertex(x, y, z, vcap);
				uvs.push_back(vec_t(x, y, z));
			}
			/*
			if (row[0] == 'v' && row[1] == 'n')
			{
				// Vertex normal
				sscanf(row + 2, "%f %f %f", &x, &y, &z);
				normals.push_back(vec_t(x, y, z));
			}
			*/
			if (row[0] == 'f')
			{
				// Faces
				if (!uvs.empty())
					nv = ParseFace(row, face);
				/*else
					nv = ParseFaceb(row, face);
					*/
				const int pos[] = { face[0], face[3], face[6] };
				const int tex[] = { face[1], face[4], face[7] };
				//const int nrm[] = { face[2], face[5], face[8] };

				//mIndices.push_back(vertCount + 2); mIndices.push_back(vertCount + 1); mIndices.push_back(vertCount);
				//mFaces.push_back({ uint16_t(vertCount + 2), uint16_t(vertCount + 1), uint16_t(vertCount), {63,63,63}});

				uint8_t *texel = &img[int(uvs[tex[0]].x * imgx) * 4];
				mFaces.push_back({ uint16_t(pos[0]), uint16_t(pos[1]), uint16_t(pos[2]), {uint8_t(texel[0]), uint8_t(texel[1]), uint8_t(texel[2])} });
#if 0
				for (int i = 0; i < 3; i++)
				{
					const vec_t& vtPos = positions[pos[i]];
					//mPositions.push_back(vtPos.x); mPositions.push_back(vtPos.y); mPositions.push_back(vtPos.z);
					mPositions.push_back(vtPos);

					/*mAABBMin.isMinOf(vtPos);
					mAABBMax.isMaxOf(vtPos);
					*/
					/*if (!normals.empty())
					{
						mVertices.push_back(normals[nrm[i]].x); mVertices.push_back(normals[nrm[i]].y); mVertices.push_back(normals[nrm[i]].z);
					}
					*/
					/*if (!uvs.empty())
					{
						mVertices.push_back(uvs[tex[i]].x); mVertices.push_back(uvs[tex[i]].y);
					}*/
				}
				vertCount += 3;
#endif
			}
		}
		return true;
	}


	char* ParseRow(char* buf, char* bufEnd, char* row, int len)
	{
		bool cont = false;
		bool start = true;
		bool done = false;
		int n = 0;
		while (!done && buf < bufEnd)
		{
			char c = *buf;
			buf++;
			// multirow
			switch (c)
			{
			case '\\':
				cont = true; // multirow
				break;
			case '\n':
				if (start) break;
				done = true;
				break;
			case '\r':
				break;
			case '\t':
			case ' ':
				if (start) break;
			default:
				start = false;
				cont = false;
				row[n++] = c;
				if (n >= len - 1)
					done = true;
				break;
			}
		}
		row[n] = '\0';
		return buf;
	}

	int ParseFace(char* row, int* data)
	{
		int res = sscanf(row, "f %d/%d/%d %d/%d/%d %d/%d/%d", &data[0], &data[1], &data[2],
			&data[3], &data[4], &data[5],
			&data[6], &data[7], &data[8]);

		for (int i = 0; i < 9; i++)
			data[i] -= 1;

		return res;
	}

	int ParseFaceb(char* row, int* data)
	{
		int res = sscanf(row, "f %d//%d %d//%d %d//%d", &data[0], &data[2],
			&data[3], &data[5],
			&data[6], &data[8]);

		for (int i = 0; i < 9; i++)
			data[i] -= 1;

		return res;
	}

	void Transform(const matrix_t& view, const matrix_t proj)
	{
		matrix_t matrix = view * proj;

		mTransformedPositions.resize(mPositions.size());
		for (size_t i = 0;i<mPositions.size();i++)
		{
			mTransformedPositions[i].TransformPoint(mPositions[i], matrix);
			mTransformedPositions[i] *= 1.f / mTransformedPositions[i].w;
			//mTransformedPositions[i].x *= 1.f / mTransformedPositions[i].w;
			//mTransformedPositions[i].y *= 1.f / mTransformedPositions[i].w;
		}
		// depth
		for (size_t i = 0; i < mFaces.size(); i++)
		{
			auto& face = mFaces[i];
			vec_t p[3];
			p[0] = mTransformedPositions[face.a];
			p[1] = mTransformedPositions[face.b];
			p[2] = mTransformedPositions[face.c];

			face.z = (p[0].z + p[1].z + p[2].z) * 0.3333f;

		}
		SortFaces();

		// clip
		mClippedFaces.clear();

		ZFrustum frustum;
		frustum.Update(view, proj);

		for (size_t i = 0; i < mSortedFaces.size(); i++)
		{
			auto& face = mSortedFaces[i];
			vec_t p[3];
			p[0] = mPositions[face.a];
			p[1] = mPositions[face.b];
			p[2] = mPositions[face.c];

			int count = 0;

			for (int j = 0;j<3;j++)
			{
				if (frustum.PointInFrustum(p[j]))
				{
					count ++;
				}
			}

			if (count == 3) 
			{
				vec_t n;
				n.cross(normalized(p[2] - p[0]), normalized(p[1] - p[0]));
				n.normalize();

				vec_t mid = (p[0] + p[1] + p[2]) * 0.33333f;
				vec_t eye(4.f, 4.f, 4.f);
				vec_t eye2tri = normalized(mid - eye);

				if (eye2tri.dot(n) > 0.f)
				{
					mClippedFaces.push_back(face);
				}
			}
		}
	}

	void ApplyDirectional()
	{
		vec_t lightDir = normalized(vec_t(-0.6f, -0.7f, -0.8f));
		for (size_t i = 0; i < mFaces.size(); i++)
		{
			vec_t n = WorldNormal(i);
			float dt = n.dot(lightDir);
			vec_t color = mFaces[i].mColor.GetVect();
			color *= (dt * 0.5f) + 0.5f;
			mFaces[i].mColor.SetVect(color);
		}
	}

	vec_t WorldNormal(int faceIndex) const
	{
		auto& face = mFaces[faceIndex];
		vec_t p[3];
		p[0] = mPositions[face.a];
		p[1] = mPositions[face.b];
		p[2] = mPositions[face.c];

		vec_t n;
		n.cross(normalized(p[2] - p[0]), normalized(p[1] - p[0]));
		n.normalize();

		return n;
	}

	void SortFaces()
	{
		mSortedFaces = mFaces;

		qsort(mSortedFaces.data(), mSortedFaces.size(), sizeof(Face), [](const void*a, const void *b) {
			Face *fa = (Face*)a;
			Face* fb = (Face*)b;
			if (fa->z < fb->z)
			{
				return 1;
			}
			if (fa->z > fb->z)
			{
				return -1;
			}
			return 0;
			});
	}

	void DebugDraw(ImDrawList& list)
	{
		for (size_t i = 0; i < mClippedFaces.size(); i++)
		{
			const auto& face = mClippedFaces[i];
			vec_t p[3];
			ImVec2 pc[3];

			p[0] = mTransformedPositions[face.a];
			p[1] = mTransformedPositions[face.b];
			p[2] = mTransformedPositions[face.c];

			auto& io = ImGui::GetIO();

			for (int j = 0; j < 3; j++)
			{
				pc[j] = ImVec2(p[j].x * io.DisplaySize.x / 2 + io.DisplaySize.x / 2, io.DisplaySize.y - (p[j].y * io.DisplaySize.y / 2 + io.DisplaySize.y / 2));
			}

			list.AddTriangleFilled(pc[0], pc[1], pc[2], face.mColor.Get32());
		}
	}
	

	const uint32_t GetFaceCount() const { return uint32_t(mFaces.size()); }
	const uint32_t GetTransformedFaceCount() const { return uint32_t(mClippedFaces.size()); }

private:

    struct Color
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
		uint32_t Get32() const
		{
			return (0xff << 24) + (b << 16) + (g << 8) + r;
		}
		void Set32(uint32_t v)
		{
			r = v & 0xFF;
			g = (v >> 8) & 0xFF;
			b = (v >> 16) & 0xFF;
		}
		vec_t GetVect() const
		{
			vec_t res;
			res.fromUInt32(Get32());
			return res;
		}
		void SetVect(const vec_t& v)
		{
			uint32_t c = v.toUInt32();
			Set32(c);
		}
    };
    struct Face
    {
        uint16_t a,b,c;
        Color mColor;
		float z;
    };
    std::vector<vec_t> mPositions;
	std::vector<vec_t> mTransformedPositions;
    std::vector<Face> mFaces;
	std::vector<Face> mSortedFaces;
	std::vector<Face> mClippedFaces;
};