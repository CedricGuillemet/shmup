#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Immath.h"
#include "moviePlayback.h"

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
		Imm::vec4 GetVect() const
		{
			Imm::vec4 res;
			res.fromUInt32(Get32());
			return res;
		}
		void SetVect(const Imm::vec4& v)
		{
			uint32_t c = v.toUInt32();
			Set32(c);
		}

		static Color Green()
		{
			return { 0,255,0 };
		}
		static Color Red()
		{
			return { 255,0,0 };
		}
		static Color Blue()
		{
			return { 0,0,255 };
		}
		static Color White()
		{
			return { 255,255,255 };
		}
		static Color Purple()
		{
			return { 255,0,255 };
		}
	};
	struct Face
	{
		uint16_t a, b, c;
		Color mColor;
		bool visible;
		bool rendered;
	};

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
		minX = std::max(minX, 0);
		minY = std::max(minY, 0);
		maxX = std::min(maxX, bufferWidth - 1);
		maxY = std::min(maxY, bufferHeight - 1);

		// Rasterize
		int x, y;
		for (y = minY; y <= maxY; y++) {
			for (x = minX; x <= maxX; x++) {
				// Determine barycentric coordinates
				V p{(int16_t)x, (int16_t)y};

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

	template<typename T, typename V> void DrawTriangleDepthTested(T* buffer, float* zBuffer, int bufferWidth, int bufferHeight, V v0, V v1, V v2, T color)
	{
		// Compute triangle bounding box
		int minX = int(min3(v0.x, v1.x, v2.x));
		int minY = int(min3(v0.y, v1.y, v2.y));
		int maxX = int(max3(v0.x, v1.x, v2.x));
		int maxY = int(max3(v0.y, v1.y, v2.y));

		// Clip against screen bounds
		minX = std::max(minX, 0);
		minY = std::max(minY, 0);
		maxX = std::min(maxX, bufferWidth - 1);
		maxY = std::min(maxY, bufferHeight - 1);

		// Rasterize
		int x, y;
		for (y = minY; y <= maxY; y++) {
			for (x = minX; x <= maxX; x++) {
				// Determine barycentric coordinates
				V p{ (float)x, (float)y };

				int w0 = orient2d(v1, v2, p);
				int w1 = orient2d(v2, v0, p);
				int w2 = orient2d(v0, v1, p);

				// If p is on or inside all edges, render pixel.
				if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				{
					float sum = w0 + w1 + w2;
					float fw0 = w0 / sum;
					float fw1 = w1 / sum;
					float fw2 = w2 / sum;
					float depth = v0.z * fw0 + v1.z * fw1 + v2.z * fw2;

					int pixelIndex = y * bufferWidth + x;
					float ldepth = zBuffer[pixelIndex];
					if (ldepth > depth) {
						buffer[pixelIndex] = color;
						zBuffer[pixelIndex] = depth;
					}
				}
			}
		}
	}

	template<typename T, typename V> void TestTriangleOcclusion(const T* buffer, const float* zBuffer, int bufferWidth, int bufferHeight, V v0, V v1, V v2, T color/*, const std::vector<Face>& faces*/, std::vector<int>& occluders)
	{
		// Compute triangle bounding box
		int minX = int(min3(v0.x, v1.x, v2.x));
		int minY = int(min3(v0.y, v1.y, v2.y));
		int maxX = int(max3(v0.x, v1.x, v2.x));
		int maxY = int(max3(v0.y, v1.y, v2.y));

		// Clip against screen bounds
		minX = std::max(minX, 0);
		minY = std::max(minY, 0);
		maxX = std::min(maxX, bufferWidth - 1);
		maxY = std::min(maxY, bufferHeight - 1);

		// Rasterize
		int x, y;
		for (y = minY; y <= maxY; y++) {
			for (x = minX; x <= maxX; x++) {
				// Determine barycentric coordinates
				V p{ (float)x, (float)y };

				int w0 = orient2d(v1, v2, p);
				int w1 = orient2d(v2, v0, p);
				int w2 = orient2d(v0, v1, p);

				// If p is on or inside all edges, render pixel.
				if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				{
					float sum = w0 + w1 + w2;
					float fw0 = w0 / sum;
					float fw1 = w1 / sum;
					float fw2 = w2 / sum;
					float depth = v0.z * fw0 + v1.z * fw1 + v2.z * fw2;

					int pixelIndex = y * bufferWidth + x;
					uint16_t occluder = buffer[pixelIndex];
					float ldepth = zBuffer[pixelIndex];
					if (ldepth < depth && color != occluder) {
						assert(occluder != 0xFFFF);
						occluders.push_back(occluder);
					}
				}
			}
		}
	}

	std::vector<Face> ClipFaces(const std::vector<Face>& faces, std::vector<Imm::vec4>& positions, const Imm::vec4& plan) const
	{
		std::vector<Face> res;

		for (size_t i = 0; i < faces.size(); i++)
		{
			const auto& face = faces[i];
			Imm::vec3 p[3];
			p[0] = positions[face.a];
			p[1] = positions[face.b];
			p[2] = positions[face.c];

			float distancesToPlan[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
			int indexBehindPlan[3] = { -1, -1, -1 };
			int indexBehindPlanCount = 0;
			int indexFrontPlan[3] = { -1, -1, -1 };
			int indexFrontPlanCount = 0;
			int indices[3] = { face.a, face.b, face.c };
			// near clip
			for (int j = 0; j < 3; j++)
			{
				const auto vt = p[j];
                float distanceToPlan = plan.signedDistanceTo({vt.x, vt.y, vt.z, 0.f});
				distancesToPlan[j] = distanceToPlan;
				if (distanceToPlan > 0.f) {
					indexFrontPlan[indexFrontPlanCount++] = j;
				}
				else {
					indexBehindPlan[indexBehindPlanCount++] = j;
				}
			}
			if (indexFrontPlanCount == 3)
			{
				res.push_back(face);
			}
			else if (indexFrontPlanCount == 1)
			{
				// degenerate 1  triangle
				int frontIndex = indexFrontPlan[0];
				int b[2] = { indexBehindPlan[0], indexBehindPlan[1] };
				if (abs(b[0] - b[1]) == 2) {
					std::swap(b[0], b[1]);
				}
				float d0 = distancesToPlan[frontIndex];
				float da = -distancesToPlan[b[0]];
				float db = -distancesToPlan[b[1]];
				float ta = d0 / (d0 + da);
				float tb = d0 / (d0 + db);
				Imm::vec4 newA = Imm::Lerp(mPositions[indices[frontIndex]], mPositions[indices[b[0]]], ta);
				Imm::vec4 newB = Imm::Lerp(mPositions[indices[frontIndex]], mPositions[indices[b[1]]], tb);
				auto baseIndex = static_cast<uint16_t>(mPositions.size());
				positions.push_back(newA);
				positions.push_back(newB);
				res.push_back({ static_cast<uint16_t>(baseIndex + 1), static_cast<uint16_t>(indices[frontIndex]), static_cast<uint16_t>(baseIndex), face.mColor });
			}
			else if (indexFrontPlanCount == 2)
			{
				int behindIndex = indexBehindPlan[0];
				int b[2] = { indexFrontPlan[0], indexFrontPlan[1] };
				if (abs(b[0] - b[1]) == 2) {
					std::swap(b[0], b[1]);
				}
				float d0 = -distancesToPlan[behindIndex];
				float da = distancesToPlan[b[0]];
				float db = distancesToPlan[b[1]];
				float ta = da / (d0 + da);
				float tb = db / (d0 + db);
				Imm::vec4 newA = Imm::Lerp(mPositions[indices[b[0]]], mPositions[indices[behindIndex]], ta);
				Imm::vec4 newB = Imm::Lerp(mPositions[indices[b[1]]], mPositions[indices[behindIndex]], tb);
				auto baseIndex = static_cast<uint16_t>(mPositions.size());
				positions.push_back(newA);
				positions.push_back(newB);
				res.push_back({ baseIndex, static_cast<uint16_t>(indices[b[0]]), static_cast<uint16_t>(indices[b[1]]), face.mColor });
				res.push_back({ baseIndex, static_cast<uint16_t>(indices[b[1]]), static_cast<uint16_t>(baseIndex + 1), face.mColor });
			}
		}

		return res;
	}

	void Transform(const Imm::matrix& view, const Imm::matrix proj, float znear, int renderWidth, int renderHeight)
	{
		Imm::matrix invView;
		invView.inverse(view);

		//invView.lookAtLH(view.position, view.position+view.dir, view.up);
		Imm::matrix matrix = invView * proj;
		
		// reject faces not in frustum, clip with near plane all others
		std::vector<Face> mClippedFaces = mFaces;

        Imm::ZFrustum frustum;
		frustum.Update(invView, proj);

        for (int planIndex = 0; planIndex < 6; planIndex++)
		{
			float d = frustum.m_Frustum[planIndex][3];
			mClippedFaces = ClipFaces(mClippedFaces, mPositions, Imm::vec4{ frustum.m_Frustum[planIndex][0], frustum.m_Frustum[planIndex][1], frustum.m_Frustum[planIndex][2], -d });
		}

		// transform to clip space

		mTransformedPositions.resize(mPositions.size());
		for (size_t positionIndex = 0; positionIndex < mPositions.size(); positionIndex++)
		{
            const auto p = mPositions[positionIndex];
            mTransformedPositions[positionIndex].TransformPoint({p.x, p.y, p.z, 0.f}, matrix);
			mTransformedPositions[positionIndex] *= 1.f / mTransformedPositions[positionIndex].w;
		}

		// depth
		auto mSortedFaces = mClippedFaces;

		for (size_t faceIndex = 0; faceIndex < mSortedFaces.size(); faceIndex++)
		{
			auto& face = mSortedFaces[faceIndex];
			face.visible = false;
			face.rendered = false;
		}

		// rasterizer test
		std::vector<uint16_t> drawBuffer(renderWidth * renderHeight, 0xFFFF);
		std::vector<float> zbuffer(renderWidth * renderHeight, FLT_MAX);

		// go
		for (size_t sortedFaceIndex = 0; sortedFaceIndex < mSortedFaces.size(); sortedFaceIndex++)
		{
			const auto& face = mSortedFaces[sortedFaceIndex];

			Imm::vec3 p[3];
			Imm::vec3 pc[3];

			p[0] = mTransformedPositions[face.a];
			p[1] = mTransformedPositions[face.b];
			p[2] = mTransformedPositions[face.c];

			for (int j = 0; j < 3; j++)
			{
                pc[j] = Imm::vec3{(p[j].x * renderWidth / 2 + renderWidth / 2), renderHeight - (p[j].y * renderHeight / 2 + renderHeight / 2), p[j].z};
			}

			DrawTriangleDepthTested(drawBuffer.data(), zbuffer.data(), renderWidth, renderHeight, pc[2], pc[1], pc[0], uint16_t(sortedFaceIndex));
		}
	
		mDepthTestedColor.resize(renderWidth * renderHeight);
		for (size_t texelIndex = 0; texelIndex < drawBuffer.size(); texelIndex++)
		{
			auto faceIndex = drawBuffer[texelIndex];
			if (faceIndex == 0xFFFF)
			{
				mDepthTestedColor[texelIndex] = 0xFFFFFFFF;
			}
			else
			{
				mDepthTestedColor[texelIndex] = mSortedFaces[faceIndex].mColor.Get32();
			}
		}

		mRasterizedFaces.clear();
		int effectiveVisibleFaceCount = 0;
		for (auto index = 0; index < drawBuffer.size(); index++)
		{
			auto pixel = drawBuffer[index];
			if (pixel == 0xFFFF)
			{
				continue;
			}
			auto& face = mSortedFaces[pixel];
			if (!face.visible)
			{
				effectiveVisibleFaceCount++;
			}
			face.visible = true;
			face.rendered = false;
		}

		// new sort
		///////////////////////////////////////////////////////////////////////////////////
		std::vector<Face> reverseSortedFaceList;
		std::vector<std::vector<int>> occludedBy;
		occludedBy.resize(mSortedFaces.size());
		std::vector<int> occluderCount;
		occluderCount.resize(mSortedFaces.size());

		for (size_t sortedFaceIndex = 0; sortedFaceIndex < mSortedFaces.size(); sortedFaceIndex++)
		{
			const auto& face = mSortedFaces[sortedFaceIndex];
			if (face.visible && !face.rendered)
			{
				Imm::vec3 p[3];
				Imm::vec3 pc[3];

				p[0] = mTransformedPositions[face.a];
				p[1] = mTransformedPositions[face.b];
				p[2] = mTransformedPositions[face.c];

				for (int j = 0; j < 3; j++)
				{
                    pc[j] = Imm::vec3{(p[j].x * renderWidth / 2 + renderWidth / 2), renderHeight - (p[j].y * renderHeight / 2 + renderHeight / 2), p[j].z};
				}

				auto& occluders = occludedBy[sortedFaceIndex];
				TestTriangleOcclusion(drawBuffer.data(), zbuffer.data(), renderWidth, renderHeight, pc[2], pc[1], pc[0], uint16_t(sortedFaceIndex), occluders);
			}
		}

		while (effectiveVisibleFaceCount)
		{
			// count occluders
			for (size_t occluderIndex = 0; occluderIndex < mSortedFaces.size(); occluderIndex++)
			{
				int& occluderCountValue = occluderCount[occluderIndex];
				occluderCountValue = 0;
				for (auto occluderFaceIndex : occludedBy[occluderIndex])
				{
					assert(mSortedFaces[occluderFaceIndex].visible);
					if (!mSortedFaces[occluderFaceIndex].rendered)
					{
						occluderCountValue++;
					}
				}
			}

			// get lowest occluded tri
			int bestOccluderIndex = -1;
			int bestOccludedValue = 0xFFFF;
			for (size_t occluderIndex = 0; occluderIndex < mSortedFaces.size(); occluderIndex++)
			{
				const auto& face = mSortedFaces[occluderIndex];
				if (face.visible && !face.rendered)
				{
					int occ = occluderCount[occluderIndex];
					if (occ < bestOccludedValue)
					{
						bestOccludedValue = occ;
						bestOccluderIndex = occluderIndex;
					}
				}
			}
			assert(bestOccluderIndex != -1);
			mSortedFaces[bestOccluderIndex].rendered = true;
			reverseSortedFaceList.push_back(mSortedFaces[bestOccluderIndex]);
			effectiveVisibleFaceCount--;
		};
		std::reverse(reverseSortedFaceList.begin(), reverseSortedFaceList.end());

		mSortedFaces = reverseSortedFaceList;


		///////////////////////////////////////////////////////////////////////////////////
		/// <summary>
		std::map<uint32_t, uint8_t> faceColorToColorIndex;
		std::map<uint16_t, uint16_t> oldToNewVertexIndex;
		frames.push_back({});
		Frame& currentFrame = frames.back();
		currentFrame.faces.clear();
		currentFrame.colors.clear();
		currentFrame.vertices.clear();
		for (size_t i = 0; i < mSortedFaces.size(); i++)
		{
			Face transient = mSortedFaces[i];
			if (transient.visible)
			{
				mRasterizedFaces.push_back(transient);

				FrameFace frameFace;

				// append used color
				auto faceColor = mSortedFaces[i].mColor.Get32();
				faceColor &= 0xFCFCFC;

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

						Imm::vec4 trPos = mTransformedPositions[index];
                        FrameVertex frameVertex{int16_t((trPos.x * (renderWidth/2) + (renderWidth/2))), static_cast<int16_t>(renderHeight - ((trPos.y * (renderHeight/2) + renderHeight/2)))};

						int foundSameVertex = -1;
						for (size_t k = 0;k< currentFrame.vertices.size(); k++)
						{
							auto& frmp = currentFrame.vertices[k];
							if (frmp.x == frameVertex.x && frmp.y == frameVertex.y)
							{
								foundSameVertex = (int)k;
								break;
							}
						}

						if (foundSameVertex == -1)
						{
							oldToNewVertexIndex[index] = (uint16_t)currentFrame.vertices.size();
							currentFrame.vertices.push_back(frameVertex);
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
				currentFrame.faces.push_back(frameFace);
			}
		}

		// generate frame colors
		currentFrame.colors.resize(faceColorToColorIndex.size());
		for (auto& faceColor : faceColorToColorIndex)
		{
			auto& color = currentFrame.colors[faceColor.second];
			color.index = faceColor.second;
			Color col;
			col.Set32(faceColor.first);
			color.r = col.r;
			color.g = col.g;
			color.b = col.b;
		}
	}

	void ReorderPositions()
	{
		struct Position
		{
			uint16_t value;
			int previousIndex;
		};

		for (size_t i = 1; i < frames.size(); i++)
		{
			std::vector<Position> positions;
			for (int v = 0; v < frames[i].vertices.size(); v++)
			{
				auto p = frames[i].vertices[v];
				positions.push_back({uint16_t((p.y << 8) + p.x), v});
				std::qsort(positions.data(), positions.size(), sizeof(Position), [](const void* A, const void* B) {
					const Position* pa = (const Position*)A;
					const Position* pb = (const Position*)B;
					if (pa->value < pb->value)
					{
						return 1;
					}
					if (pa->value > pb->value)
					{
						return -1;
					}
					return 0;
				});
			}
			int oldToNew[256];
			for (int v = 0; v< positions.size();v++)
			{
				oldToNew[positions[v].previousIndex] = v;
			}
			for (int f = 0; f< frames[i].faces.size(); f++)
			{
				auto& face = frames[i].faces[f];
				face.a = oldToNew[face.a];
				face.b = oldToNew[face.b];
				face.c = oldToNew[face.c];
			}

			// delta positions
			for (int v = positions.size() - 1; v > 0; v--)
			{
				positions[v].value -= positions[v-1].value;
			}

			// reset positions

			for (int v = positions.size() - 1; v > 0; v--)
			{
				frames[i].vertices[v].y = (positions[v].value >> 8);
				frames[i].vertices[v].x = (positions[v].value & 0xFF);
			}
			// test indexed/non-indexed mode

			int usedVts[256];
			memset(usedVts, 0, sizeof(int) * 256);

			for (int f = 0; f < frames[i].faces.size(); f++)
			{
				const auto& face = frames[i].faces[f];
				usedVts[face.a] ++;
				usedVts[face.b] ++;
				usedVts[face.c] ++;
			}
			int count[256];
			memset(count, 0, sizeof(int) * 256);
			for (int u = 0;u<positions.size();u++)
			{
				count[usedVts[u]] ++;
			}

			for (int p = 0;p<256;p++)
			{
				if (count[p])
				{
					printf("A vertex used %d times is found %d times\n", p, count[p]);
				}
			}
		}
	}

    // return true if successfull
	bool CompressColors(int maxColors)
	{
        // check color count
        for (size_t i = 0; i < frames.size(); i++)
        {
            if (frames[i].colors.size() > maxColors)
            {
                return false;
            }
        }
        
        static const uint32_t UnassignedColor = 0xFFFFFFFF;
        
        uint32_t palette[256];
        memset(palette, UnassignedColor, sizeof(uint32_t) * 256);
        for (size_t colorIndex = 0; colorIndex < frames[0].colors.size(); colorIndex++)
        {
            const auto currentColor = frames[0].colors[colorIndex];
            uint32_t col32 = FrameColor32(currentColor);
            palette[currentColor.index] = col32;
        }
        for (size_t frameIndex = 1; frameIndex < frames.size(); frameIndex ++)
        {
            std::vector<FrameColor> newFrameColors;
            auto& currentFrame = frames[frameIndex];
            uint8_t remappedColorIndices[256];
            const auto& currentColors = currentFrame.colors;
            // find currently bound colors in new frame
            for (size_t paletteIndex = 0; paletteIndex < 256; paletteIndex++)
            {
                const auto paletteColor = palette[paletteIndex];
                if (paletteColor == UnassignedColor)
                {
                    continue;
                }
                bool paletteColorIsStillUsed = false;
                for (size_t frameColorIndex = 0; frameColorIndex < currentColors.size(); frameColorIndex++)
                {
                    auto currentFrameColor = currentColors[frameColorIndex];
                    uint32_t col32 = FrameColor32(currentFrameColor);
                    if (col32 == paletteColor)
                    {
                        // color is found is new frame, reassign index in faces
                        paletteColorIsStillUsed = true;
                        remappedColorIndices[currentFrameColor.index] = paletteIndex;
                        break;
                    }
                }
                if (!paletteColorIsStillUsed)
                {
                    palette[paletteIndex] = UnassignedColor;
                }
            }

            // second pass to get colors in frame not set in current palette
            for (size_t frameColorIndex = 0; frameColorIndex < currentColors.size(); frameColorIndex++)
            {
                auto currentFrameColor = currentColors[frameColorIndex];
                uint32_t col32 = FrameColor32(currentFrameColor);
                bool alreadyBound = false;
                for (size_t paletteIndex = 0; paletteIndex < 256; paletteIndex++)
                {
                    const auto paletteColor = palette[paletteIndex];
                    if (paletteColor == UnassignedColor)
                    {
                        continue;
                    }
                    if (paletteColor == col32)
                    {
                        alreadyBound = true;
                        break;
                    }
                }
                if (!alreadyBound)
                {
                    // get first free color index
                    bool colorAttributed = false;
                    for (size_t paletteIndex = 0; paletteIndex < 256; paletteIndex++)
                    {
                        const auto paletteColor = palette[paletteIndex];
                        if (paletteColor == UnassignedColor)
                        {
                            colorAttributed = true;
                            palette[paletteIndex] = col32;
                            newFrameColors.push_back({(uint8_t)paletteIndex, currentFrameColor.r, currentFrameColor.g, currentFrameColor.b});
                            remappedColorIndices[currentFrameColor.index] = paletteIndex;
                            break;
                        }
                    }
                    assert(colorAttributed);
                }
            }
            // apply new color delta
            currentFrame.colors = newFrameColors;
            // set remapped faces
            for (auto& face : currentFrame.faces)
            {
                face.colorIndex = remappedColorIndices[face.colorIndex];
            }
        }
        
        return true;
	}
#if 0
	void DebugDraw(ImDrawList& list, ImVec2 displaySize, ImVec2 displayOffset)
	{
		for (size_t i = 0; i < mRasterizedFaces.size(); i++)
		{
			const auto& face = mRasterizedFaces[i];
			Imm::vec3 p[3];
			ImVec2 pc[3];

			p[0] = mTransformedPositions[face.a];
			p[1] = mTransformedPositions[face.b];
			p[2] = mTransformedPositions[face.c];

			auto& io = ImGui::GetIO();

			for (int j = 0; j < 3; j++)
			{
				pc[j] = ImVec2(displayOffset.x + /*displaySize.x -*/ (p[j].x * displaySize.x / 2 + displaySize.x / 2), displayOffset.y + displaySize.y - (p[j].y * displaySize.y / 2 + displaySize.y / 2));
			}

			list.AddTriangleFilled(pc[0], pc[1], pc[2], face.mColor.Get32());
		}
	}
#endif

	const uint32_t GetFaceCount() const { return uint32_t(mFaces.size()); }
	const uint32_t GetRasterizedFaceCount() const { return uint32_t(mRasterizedFaces.size()); }

public:

    std::vector<Imm::vec4> mPositions;
	std::vector<Imm::vec4> mTransformedPositions;
    std::vector<Face> mFaces;
	
	std::vector<Face> mRasterizedFaces;

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
			bytes.resize(szVertices + szFaces + szColors + sizeof(FrameInfos));
			uint8_t *ptr = bytes.data();

			// infos
			assert(vertices.size() < 256);
			assert(colors.size() < 200);

			FrameInfos infos;
			infos.vertexCount = static_cast<uint8_t>(vertices.size());
			infos.colorCount = static_cast<uint8_t>(colors.size());
			infos.faceCount = static_cast<uint16_t>(faces.size());
			memcpy(ptr, &infos, sizeof(FrameInfos));
			ptr += sizeof(FrameInfos);

			// vertices
			memcpy(ptr, vertices.data(), szVertices);
			ptr += szVertices;

			// faces
			memcpy(ptr, faces.data(), szFaces);
			ptr += szFaces;

			// colors
			memcpy(ptr, colors.data(), szColors);
			return bytes;
		}
	};

	std::vector<Frame> frames;
	std::vector<uint32_t> mDepthTestedColor;

	void DebugDrawFrame(Frame* frame, uint32_t* rgbaBuffer, int renderWidth, int renderHeight)
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
			DrawTriangle(rgbaBuffer, renderWidth, renderHeight, v2, v1, v0, uint32_t(0xFF000000 + (color.b << 16) + (color.g << 8) + color.r));
		}
	}
};
