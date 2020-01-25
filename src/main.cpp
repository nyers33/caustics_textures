#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "fft.h"
#include "export_util.h"

#define HEIGHTFIELD_DEBUG
#define NORMAL_DEBUG
#define SVG_DEBUG
#define BIN_DEBUG

const int nT = 48;
const int nX = 64;
const int nZ = 64;
const int sizeImg = nX * nZ;
const int nGrid = nT * nX * nZ;
const float scale = 16.0f;

const float sizeX = 1.0f;
const float sizeZ = 1.0f;

const float stepX = sizeX / (nX - 1.0f);
const float stepZ = sizeZ / (nZ - 1.0f);

int main()
{
	glm::vec2* dataIn = new glm::vec2[nGrid];
	glm::vec2* dataOut = new glm::vec2[nGrid];
	memset(dataIn, 0, nGrid * sizeof(glm::vec2));
	memset(dataOut, 0, nGrid * sizeof(glm::vec2));

	glm::vec4* initGrid = new glm::vec4[sizeImg];
	memset(initGrid, 0, sizeImg * sizeof(glm::vec4));
	glm::vec4* positionCaustics = new glm::vec4[sizeImg];
	memset(positionCaustics, 0, sizeImg * sizeof(glm::vec4));
	glm::vec4* normalCaustics = new glm::vec4[sizeImg];
	memset(normalCaustics, 0, sizeImg * sizeof(glm::vec4));
	glm::vec3* colorCaustics = new glm::vec3[2 * sizeImg];
	memset(colorCaustics, 0, 2 * sizeImg * sizeof(glm::vec3));

	for (int j = 0; j < nZ; ++j)
	{
		for (int i = 0; i < nX; ++i)
		{
			initGrid[i + j * nZ] = positionCaustics[i + j * nZ] = glm::vec4(static_cast<float>(i) / static_cast<float>(nX), 0.0f, static_cast<float>(j) / static_cast<float>(nZ), 1.0f) - glm::vec4(0.5f, 0.0f, 0.5f, 0.0f);
			normalCaustics[i + j * nZ] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		}
	}
	plyPlaneOutput(initGrid, nX, nZ, "base_plane.ply");

	// create dataIn
	initData(dataIn, nX, nZ, nT);

	// make the center pixel of the image the one with zero-phase shift
	for (int z = 0; z < nT; ++z)
		for (int j = 0; j < nZ; ++j)
			for (int i = 0; i < nX; ++i)
				dataIn[i + nX * j + nX * nZ * z].x *= powf(-1.0f, static_cast<float>(i + j));

	#ifdef BIN_DEBUG
	binOutput(dataIn, nX, nZ, nT, "bin_fft_pre.dat");
	#endif

	// fft in x dim
	for (int i = 0; i < nZ * nT; ++i)
	{
		radix2(dataIn + nX * i, dataOut + nX * i, nX, 1, 1);
	}

	// fft in z dim
	for (int j = 0; j < nT; ++j)
	{
		for (int i = 0; i < nX; ++i)
		{
			radix2(dataIn + i + nX * nZ * j, dataOut + i + nX * nZ * j, nZ, nX, 1);
		}
	}

	#ifdef BIN_DEBUG
	binOutput(dataIn, nX, nZ, nT, "bin_fft_post.dat");
	#endif

	for (int z = 0; z < nT; ++z)
	{
		for (int j = 0; j < nZ; ++j)
		{
			for (int i = 0; i < nX; ++i)
			{
				dataIn[i + nX * j + nX * nZ * z].x *= static_cast<float>(1.0 / pow((double)sqrt(pow((i + 1 - nX * 0.5), 2.0) + pow((j + 1 - nZ * 0.5), 2.0)), 1.95));
				dataIn[i + nX * j + nX * nZ * z].y *= static_cast<float>(1.0 / pow((double)sqrt(pow((i + 1 - nX * 0.5), 2.0) + pow((j + 1 - nZ * 0.5), 2.0)), 1.95));
			}
		}

		dataIn[nX / 2 - 1 + nX * (nZ / 2 - 1) + nX * nZ * z].x = 0.0f;
		dataIn[nX / 2 - 1 + nX * (nZ / 2 - 1) + nX * nZ * z].y = 0.0f;
	}

	float sigma = 6.0f;
	float* gaussCoeff = new float[nT];
	float* gaussedRe = new float[nT];
	float* gaussedIm = new float[nT];

	for (int z = 0; z <= nT / 2; ++z)
	{
		gaussCoeff[z] = powf(2.71828f, -static_cast<int>(z * z) / (2.0f * sigma * sigma)) / (sigma * sqrtf(2.0f * 3.14159f));
	}
	for (int z = 0; z < nT / 2 - 1; ++z)
	{
		gaussCoeff[z + nT / 2 + 1] = powf(2.71828f, -static_cast<int>((z - static_cast<int>(nT / 2) + 1) * (z - static_cast<int>(nT / 2) + 1)) / (2.0f * sigma * sigma)) / (sigma * sqrtf(2.0f * 3.14159f));
	}

	for (int j = 0; j < nZ; ++j)
	{
		for (int i = 0; i < nX; ++i)
		{
			memset(gaussedRe, 0, nT * sizeof(float));
			memset(gaussedIm, 0, nT * sizeof(float));

			for (int z = 0; z < nT; ++z)
			{
				for (int it = 0; it < nT; ++it)
				{
					gaussedRe[z] += gaussCoeff[(nT - z + it) % nT] * dataIn[i + nX * j + nX * nZ * it].x;
					gaussedIm[z] += gaussCoeff[(nT - z + it) % nT] * dataIn[i + nX * j + nX * nZ * it].y;
				}
			}

			for (int z = 0; z < nT; ++z)
			{
				dataIn[i + nX * j + nX * nZ * z].x = gaussedRe[z];
				dataIn[i + nX * j + nX * nZ * z].y = gaussedIm[z];
			}
		}
	}

	delete[] gaussCoeff;
	delete[] gaussedRe;
	delete[] gaussedIm;

	// ifft in x dim
	for (int i = 0; i < nZ * nT; ++i)
	{
		radix2(dataIn + nX * i, dataOut + nX * i, nX, 1, -1);
	}
	// ifft in z dim
	for (int j = 0; j < nT; ++j)
	{
		for (int i = 0; i < nX; ++i)
		{
			radix2(dataIn + i + nX * nZ * j, dataOut + i + nX * nZ * j, nZ, nX, -1);
		}
	}

	for (int i = 0; i < nGrid; ++i)
	{
		dataIn[i].x /= (nX * nZ);
		dataIn[i].y /= (nX * nZ);
	}

	for (int i = 0; i < nGrid; ++i)
	{
		dataIn[i].x = sqrtf(dataIn[i].x * dataIn[i].x + dataIn[i].y * dataIn[i].y);
		dataIn[i].y = 0.0f;
	}

	for (int t = 0; t < nT; ++t)
	{
		std::stringstream padZeroSS;
		padZeroSS << std::setfill('0') << std::setw(4) << t;

		for (int i = 0; i < nX * nZ; ++i)
		{
			positionCaustics[i].x = initGrid[i].x;
			positionCaustics[i].y = scale * dataIn[i + nX * nZ * t].x;
			positionCaustics[i].z = initGrid[i].z;
		}
		#ifdef HEIGHTFIELD_DEBUG
		plyPlaneOutput(positionCaustics, nX, nZ, "wave_heightfield_" + padZeroSS.str() + ".ply");
		#endif

		// normal calculation
		for (int j = 0; j < nZ; ++j)
		{
			for (int i = 0; i < nX; ++i)
			{
				int index_left = j * nX + mod((i - 1), nX);
				int index_right = j * nX + mod((i + 1), nX);
				int index_bottom = mod((j - 1), nZ) * nX + i;
				int index_top = mod((j + 1), nZ) * nX + i;
				float gradX = positionCaustics[index_left].y - positionCaustics[index_right].y;
				float gradZ = positionCaustics[index_bottom].y - positionCaustics[index_top].y;
				glm::vec3 normal = glm::normalize(glm::vec3(-2.0f * stepX * gradX, 4 * stepX * stepZ, -2.0f * stepZ * gradZ));
				normalCaustics[i + j * nZ] = glm::vec4(normal.x, normal.y, normal.z, 0.0f);
			}
		}
		#ifdef NORMAL_DEBUG
		plyNormalOutput(positionCaustics, normalCaustics, nX, nZ, stepX, stepZ, "wave_normal_" + padZeroSS.str() + ".ply");
		#endif

		// caustics calculation
		for (int i = 0; i < nX * nZ; i++)
		{
			glm::vec4 sun(0.0f, -1.0f, 0.0f, 0.0f);

			float initNormY = normalCaustics[i].y;
			float eta = 1.33f;

			glm::vec4 refractVec = sun / eta + normalCaustics[i] * (initNormY / eta - sqrtf(1.0f - (1.0f - initNormY * initNormY) / (eta * eta)));

			float depth = -3.6f;

			positionCaustics[i] = -((depth + positionCaustics[i].y) / refractVec.y) * refractVec + positionCaustics[i];
		}
		plyPlaneOutput(positionCaustics, nX, nZ, "caustics_plane_" + padZeroSS.str() + ".ply");
		plyPlaneBlendOutput(positionCaustics, nX, nZ, "caustics_plane_blend_color_" + padZeroSS.str() + ".ply");
		
		// inkscape can render wireframe without any issues
		// full triangle mesh with colors and blending, on the other hand, is too heavy to process
		#ifdef SVG_DEBUG
		svgPlaneOutput(positionCaustics, nX, nZ, "caustics_texture_shell_" + padZeroSS.str() + ".svg", true);
		svgPlaneOutput(positionCaustics, nX, nZ, "caustics_texture_color_" + padZeroSS.str() + ".svg", false);
		#endif
		
	}

	delete[] dataIn;
	delete[] dataOut;
	delete[] initGrid;
	delete[] positionCaustics;
	delete[] normalCaustics;
	delete[] colorCaustics;
}
