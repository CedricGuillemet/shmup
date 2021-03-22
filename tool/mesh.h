#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>

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

void WriteFile(const std::string& filePathName, const std::vector<uint8_t>& bytes)
{
	FILE* fp = fopen(filePathName.c_str(), "wb");
	if (fp)
	{
		fwrite(bytes.data(), bytes.size(), 1, fp);
		fclose(fp);
	}
}

class Mesh
{
public:
    Mesh() {}

	template<typename T>T min2(T a, T b)
	{
		return (a < b) ? a : b;
	}

	template<typename T>T max2(T a, T b)
	{
		return (a > b) ? a : b;
	}

	template<typename T>T min3(T a, T b, T c)
	{
		return min2(a, min2(b, c));
	}

	template<typename T>T max3(T a, T b, T c)
	{
		return max2(a, max2(b, c));
	}

	template<typename V> int orient2d(V a, V b, V c)
	{
		return int((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x));
	}

	template<typename T, typename V> void DrawTriangle(T* buffer, int bufferWidth, int bufferHeight, V v0, V v1, V v2, T color)
	{
		// Compute triangle bounding box
		int minX = int(min3(v0.x, v1.x, v2.x));
		int minY = int(min3(v0.y, v1.y, v2.y));
		int maxX = int(max3(v0.x, v1.x, v2.x));
		int maxY = int(max3(v0.y, v1.y, v2.y));

		// Clip against screen bounds
		minX = max(minX, 0);
		minY = max(minY, 0);
		maxX = min(maxX, bufferWidth - 1);
		maxY = min(maxY, bufferHeight - 1);

		// Rasterize
		int x, y;
		for (y = minY; y <= maxY; y++) {
			for (x = minX; x <= maxX; x++) {
				// Determine barycentric coordinates
				V p{x, y};

				int w0 = orient2d(v1, v2, p);
				int w1 = orient2d(v2, v0, p);
				int w2 = orient2d(v0, v1, p);

				// If p is on or inside all edges, render pixel.
				if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				{
					//setPixelNoCheck(x, y, color);
					buffer[y * bufferWidth + x] = color;
				}
			}
		}
	}

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
		matrix_t invView;
		invView.inverse(view);
		matrix_t matrix = invView * proj;

		mTransformedPositions.resize(mPositions.size());
		for (size_t i = 0;i<mPositions.size();i++)
		{
			mTransformedPositions[i].TransformPoint(mPositions[i], matrix);
			mTransformedPositions[i] *= 1.f / mTransformedPositions[i].w;
			//mTransformedPositions[i].x *= 1.f / mTransformedPositions[i].w;
			//mTransformedPositions[i].y *= 1.f / mTransformedPositions[i].w;
		}
		// depth
		auto mSortedFaces = mFaces;

		for (size_t i = 0; i < mSortedFaces.size(); i++)
		{
			auto& face = mSortedFaces[i];
			vec_t p[3];
			p[0] = mTransformedPositions[face.a];
			p[1] = mTransformedPositions[face.b];
			p[2] = mTransformedPositions[face.c];

			face.z = (p[0].z + p[1].z + p[2].z) * 0.3333f;

		}
		// sort faces
		

		qsort(mSortedFaces.data(), mSortedFaces.size(), sizeof(Face), [](const void* a, const void* b) {
			Face* fa = (Face*)a;
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

		// clip
		std::vector<Face> mClippedFaces;

		ZFrustum frustum;
		frustum.Update(invView, proj);

		for (size_t i = 0; i < mSortedFaces.size(); i++)
		{
			auto& face = mSortedFaces[i];
			vec_t p[3];
			p[0] = mPositions[face.a];
			p[1] = mPositions[face.b];
			p[2] = mPositions[face.c];

			// back face culling

			vec_t n;
			n.cross(normalized(p[2] - p[0]), normalized(p[1] - p[0]));
			n.normalize();

			vec_t mid = (p[0] + p[1] + p[2]) * 0.33333f;
			vec_t eye = view.position;
			eye.w = 0.f;
			vec_t eye2tri = normalized(mid - eye);

			if (eye2tri.dot(n) < 0.f)
			{
				continue;
			}

			// clipping
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
				mClippedFaces.push_back(face);
			}
			if (count == 1 || count == 2)
			{
				int countFront = 0;
				// near clip
				for (int j = 0; j < 3; j++)
				{
					const auto vt = p[j];
					if (frustum.m_Frustum[ZFrustum::BACK][ZFrustum::A] * vt.x + frustum.m_Frustum[ZFrustum::BACK][ZFrustum::B] * vt.y + frustum.m_Frustum[ZFrustum::BACK][ZFrustum::C] * vt.z + frustum.m_Frustum[ZFrustum::BACK][ZFrustum::D] > 0)
					{
						countFront ++;
					}
				}
				if (countFront == 3)
				{
					mClippedFaces.push_back(face);
				}
			}
		}

		// rasterizer test
		std::vector<uint16_t> drawBuffer(320*200, 0xFFFF);
		for (size_t i = 0; i < mClippedFaces.size(); i++)
		{
			const auto& face = mClippedFaces[i];

			vec_t p[3];
			vec_t pc[3];

			p[0] = mTransformedPositions[face.a];
			p[1] = mTransformedPositions[face.b];
			p[2] = mTransformedPositions[face.c];

			auto& io = ImGui::GetIO();

			for (int j = 0; j < 3; j++)
			{
				pc[j] = vec_t(p[j].x * 320 / 2 + 320 / 2, 200 - (p[j].y * 200 / 2 + 200 / 2));
			}

			DrawTriangle(drawBuffer.data(), 320, 200, pc[2], pc[1], pc[0], uint16_t(i));
		}

		mRasterizedFaces.clear();
		std::vector<bool> faceSeen(mClippedFaces.size(), false);
		for (auto pixel: drawBuffer)
		{
			if (pixel == 0xFFFF)
			{
				continue;
			}
			faceSeen[pixel] = true;
		}
		std::map<uint32_t, uint8_t> faceColorToColorIndex;
		std::map<uint16_t, uint16_t> oldToNewVertexIndex;
		frames.resize(1);
		frames[0].faces.clear();
		frames[0].colors.clear();
		frames[0].vertices.clear();
		for (size_t i = 0; i < faceSeen.size(); i++)
		{
			if (faceSeen[i])
			{
				Face transient = mClippedFaces[i];
				mRasterizedFaces.push_back(transient);

				FrameFace frameFace;

				// append used color
				auto faceColor = mClippedFaces[i].mColor.Get32();

				auto iter = faceColorToColorIndex.find(faceColor);
				if (iter != faceColorToColorIndex.end())
				{
					frameFace.colorIndex = iter->second;
				}
				else
				{
					frameFace.colorIndex = uint8_t(faceColorToColorIndex.size());
					faceColorToColorIndex[faceColor] = frameFace.colorIndex;
				}

				uint16_t indices[3] = {transient.a, transient.b, transient.c};
				uint16_t newIndices[3];
				for (int j = 0; j < 3; j++)
				{
					uint16_t index = indices[j];
					auto iterv = oldToNewVertexIndex.find(index);
					if (iterv != oldToNewVertexIndex.end())
					{
						newIndices[j] = iterv->second;
					}
					else
					{

						vec_t trPos = mTransformedPositions[index];
						FrameVertex frameVertex{int16_t(trPos.x * 320 / 2 + 320 / 2), int16_t(200 - (trPos.y * 200 / 2 + 200 / 2))};

						int foundSameVertex = -1;
						for (size_t k = 0;k< frames[0].vertices.size(); k++)
						{
							auto& frmp = frames[0].vertices[k];
							if (frmp.x == frameVertex.x && frmp.y == frameVertex.y)
							{
								foundSameVertex = (int)k;
								break;
							}
						}

						if (foundSameVertex == -1)
						{
							oldToNewVertexIndex[index] = (uint16_t)frames[0].vertices.size();
							frames[0].vertices.push_back(frameVertex);
							newIndices[j] = oldToNewVertexIndex[index];
						}
						else
						{
							newIndices[j] = foundSameVertex;
						}
					}
				}

				// frame face indices
				frameFace.a = (uint8_t)newIndices[0];
				frameFace.b = (uint8_t)newIndices[1];
				frameFace.c = (uint8_t)newIndices[2];

				// append face
				frames[0].faces.push_back(frameFace);
			}
		}

		// generate frame colors
		frames[0].colors.resize(faceColorToColorIndex.size());
		for (auto& faceColor : faceColorToColorIndex)
		{
			auto& color = frames[0].colors[faceColor.second];
			color.index = faceColor.second;
			Color col;
			col.Set32(faceColor.first);
			color.r = col.r;
			color.g = col.g;
			color.b = col.b;
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

	vec_t WorldNormal(size_t faceIndex) const
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

	void DebugDraw(ImDrawList& list, ImVec2 displaySize, ImVec2 displayOffset)
	{
		for (size_t i = 0; i < mRasterizedFaces.size(); i++)
		{
			const auto& face = mRasterizedFaces[i];
			vec_t p[3];
			ImVec2 pc[3];

			p[0] = mTransformedPositions[face.a];
			p[1] = mTransformedPositions[face.b];
			p[2] = mTransformedPositions[face.c];

			auto& io = ImGui::GetIO();

			for (int j = 0; j < 3; j++)
			{
				pc[j] = ImVec2(displayOffset.x + p[j].x * displaySize.x / 2 + displaySize.x / 2, displayOffset.y + displaySize.y - (p[j].y * displaySize.y / 2 + displaySize.y / 2));
			}

			list.AddTriangleFilled(pc[0], pc[1], pc[2], face.mColor.Get32());
			//list.AddTriangle(pc[0], pc[1], pc[2], 0xFFFFFFFF);//face.mColor.Get32());
		}
	}
	

	const uint32_t GetFaceCount() const { return uint32_t(mFaces.size()); }
	//const uint32_t GetTransformedFaceCount() const { return uint32_t(mClippedFaces.size()); }
	const uint32_t GetRasterizedFaceCount() const { return uint32_t(mRasterizedFaces.size()); }

public:

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
		//uint8_t colorIndex;
    };
    std::vector<vec_t> mPositions;
	std::vector<vec_t> mTransformedPositions;
    std::vector<Face> mFaces;
	
	std::vector<Face> mRasterizedFaces;

	struct FrameVertex
	{
		int16_t x,y;
	};
	struct FrameFace
	{
		uint8_t a,b,c, colorIndex;
	};
	struct FrameColor
	{
		uint8_t index, r, g, b;
	};

	struct Frame
	{
		std::vector<FrameVertex> vertices;
		std::vector<FrameFace> faces;
		std::vector<FrameColor> colors;

		std::vector<uint8_t> GetBytes() const
		{
			std::vector<uint8_t> bytes;
			size_t szVertices = vertices.size() * sizeof(FrameVertex);
			size_t szFaces = faces.size() * sizeof(FrameFace);
			size_t szColors = colors.size() * sizeof(FrameColor);
			bytes.resize(szVertices + szFaces + szColors);
			uint8_t *ptr = bytes.data();
			memcpy(ptr, vertices.data(), szVertices);
			ptr += szVertices;
			memcpy(ptr, faces.data(), szFaces);
			ptr += szFaces;
			memcpy(ptr, colors.data(), szColors);
			return bytes;
		}
	};

	std::vector<Frame> frames;


	void DebugDrawFrame(Frame* frame, uint32_t* rgbaBuffer)
	{
		for (size_t i = 0; i < frame->faces.size(); i++)
		{
			FrameFace* face = &frame->faces[i];
			FrameVertex& v0 = frame->vertices[face->a];
			FrameVertex& v1 = frame->vertices[face->b];
			FrameVertex& v2 = frame->vertices[face->c];
			//FrameColor& color = frame->colors[face->colorIndex];
			FrameColor color;
			color.r = 0xFF;
			color.g = 0xFF;
			color.b = 0xFF;
			if (face->colorIndex < frame->colors.size())
			{
				color = frame->colors[face->colorIndex];
			}
			DrawTriangle(rgbaBuffer, 320, 200, v2, v1, v0, uint32_t(0xFF000000 + (color.b << 16) + (color.g << 8) + color.r));
		}
	}
};