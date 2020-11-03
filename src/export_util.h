#include <glm/glm.hpp>

// gives a rotation about the origin that transforms vector {0,1,0} to the direction of the vector vec
glm::mat4 rotationTransform(const glm::vec4& vec)
{
	float v1 = vec.x;
	float v2 = vec.y;
	float v3 = vec.z;

	glm::mat4 orientMat(1.0f);
	orientMat[0][0] = (v1 * v1 * v2 + v3 * v3 * sqrtf(v1 * v1 + v2 * v2 + v3 * v3)) / ((v1 * v1 + v3 * v3) * sqrtf(v1 * v1 + v2 * v2 + v3 * v3));
	orientMat[0][1] = v1 / sqrtf(v1 * v1 + v2 * v2 + v3 * v3);
	orientMat[0][2] = -(v1 * v3 * (-v2 + sqrtf(v1 * v1 + v2 * v2 + v3 * v3))) / ((v1 * v1 + v3 * v3) * sqrtf(v1 * v1 + v2 * v2 + v3 * v3));
	orientMat[1][0] = -v1 / sqrtf(v1 * v1 + v2 * v2 + v3 * v3);
	orientMat[1][1] = v2 / sqrtf(v1 * v1 + v2 * v2 + v3 * v3);
	orientMat[1][2] = -v3 / sqrtf(v1 * v1 + v2 * v2 + v3 * v3);
	orientMat[2][0] = -(v1 * v3 * (-v2 + sqrtf(v1 * v1 + v2 * v2 + v3 * v3))) / ((v1 * v1 + v3 * v3) * sqrtf(v1 * v1 + v2 * v2 + v3 * v3));
	orientMat[2][1] = v3 / sqrtf(v1 * v1 + v2 * v2 + v3 * v3);
	orientMat[2][2] = (v2 * v3 * v3 + v1 * v1 * sqrtf(v1 * v1 + v2 * v2 + v3 * v3)) / ((v1 * v1 + v3 * v3) * sqrtf(v1 * v1 + v2 * v2 + v3 * v3));

	return orientMat;
}

void plyPlaneOutput(glm::vec4*& data, unsigned int nX, unsigned int nY, const std::string& fileName)
{
	std::ofstream plyFile;
	plyFile.open(fileName, std::ios_base::out);
	std::stringstream ss;

	ss << "ply\n";
	ss << "format ascii 1.0\n";
	ss << "element vertex " << (nX+1) * (nY+1) << "\n";
	ss << "property float x\n";
	ss << "property float y\n";
	ss << "property float z\n";
	ss << "property uchar red\n";
	ss << "property uchar green\n";
	ss << "property uchar blue\n";
	ss << "element face " << 2 * nX * nY << "\n";
	ss << "property list uchar int vertex_indices\n";
	ss << "end_header\n";

	for (unsigned int j = 0; j < nY; ++j)
	{
		for (unsigned int i = 0; i < nX; ++i)
		{
			ss << data[i + j * nX].x << " " << data[i + j * nX].y << " " << data[i + j * nX].z << " "; // vertID: i + j * (nX + 1)
			ss << 255 << " " << 255 << " " << 255 << "\n";
		}
		ss << 1.0f + data[j * nX].x << " " << data[j * nX].y << " " << data[j * nX].z << " "; // vertID: (j+1) * (nX + 1) - 1
		ss << 255 << " " << 255 << " " << 255 << "\n";
	}
	{
		unsigned int j = 0;
		for (unsigned int i = 0; i < nX; ++i)
		{
			ss << data[i + j * nX].x << " " << data[i + j * nX].y << " " << 1.0f + data[i + j * nX].z << " "; // vertID: i + nY * (nX + 1)
			ss << 255 << " " << 255 << " " << 255 << "\n";
		}
		ss << 1.0f + data[j * nX].x << " " << data[j * nX].y << " " << 1.0f + data[j * nX].z << " "; // vertID: (nY + 1) * (nX + 1) - 1
		ss << 255 << " " << 255 << " " << 255 << "\n";
	}

	for (unsigned int j = 0; j < nY; ++j)
	{
		for (unsigned int i = 0; i < nX; ++i)
		{
			ss << 3 << " " << i + 1 + j * (nX + 1) << " " << i + j * (nX + 1) << " " << i + nX + 1 + j * (nX + 1) << "\n"; // faceID: 2 * (i + 0 + j * nX)
			ss << 3 << " " << i + 1 + j * (nX + 1) << " " << i + nX + 1 + j * (nX + 1) << " " << i + nX + 2 + j * (nX + 1) << "\n"; // faceID: 2 * (i + 0 + j * nX)
		}
	}

	plyFile << ss.str();
	plyFile.close();
}

void plyPlaneBlendOutput(glm::vec4*& data, unsigned int nX, unsigned int nY, const std::string& fileName)
{
	std::ofstream plyFile;
	plyFile.open(fileName, std::ios_base::out);
	std::stringstream ss;

	ss << "ply\n";
	ss << "format ascii 1.0\n";
	ss << "element vertex " << 6 * nX * nY << "\n";
	ss << "property float x\n";
	ss << "property float y\n";
	ss << "property float z\n";
	ss << "property uchar red\n";
	ss << "property uchar green\n";
	ss << "property uchar blue\n";
	ss << "element face " << 2 * nX * nY << "\n";
	ss << "property list uchar int vertex_indices\n";
	ss << "end_header\n";

	int triColR, triColG, triColB;

	for (unsigned int j = 0; j < nY - 1; ++j)
	{
		for (unsigned int i = 0; i < nX - 1; ++i)
		{
			int i0 = (i + 0) + (j + 0) * nX;
			int i1 = (i + 1) + (j + 0) * nX;
			int i2 = (i + 0) + (j + 1) * nX;
			int i3 = (i + 1) + (j + 1) * nX;

			float tri0_area = nX * nY * abs(((data[i0].x + 0.5f) - (data[i1].x + 0.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 0.5f)) - ((data[i1].x + 0.5f) - (data[i2].x + 0.5f)) * ((data[i0].z + 0.5f) - (data[i1].z + 0.5f)));
			float tri1_area = nX * nY * abs(((data[i1].x + 0.5f) - (data[i2].x + 0.5f)) * ((data[i2].z + 0.5f) - (data[i3].z + 0.5f)) - ((data[i2].x + 0.5f) - (data[i3].x + 0.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 0.5f)));

			glm::vec3 tri0_col(24.0f / tri0_area, 24.0f / tri0_area, 160.0f / tri0_area);
			glm::vec3 tri1_col(24.0f / tri1_area, 24.0f / tri1_area, 160.0f / tri1_area);

			if (tri0_col.x > 255.0f)
				triColR = 255;
			else
				triColR = static_cast<int>(tri0_col.x);
			if (tri0_col.y > 255.0f)
				triColG = 255;
			else
				triColG = static_cast<int>(tri0_col.y);
			if (tri0_col.z > 255.0f)
				triColB = 255;
			else
				triColB = static_cast<int>(tri0_col.z);

			ss << (data[i0].x + 0.5f) << " 0.0 " << (data[i0].z + 0.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";
			ss << (data[i1].x + 0.5f) << " 0.0 " << (data[i1].z + 0.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";
			ss << (data[i2].x + 0.5f) << " 0.0 " << (data[i2].z + 0.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";

			if (tri1_col.x > 255.0f)
				triColR = 255;
			else
				triColR = static_cast<int>(tri1_col.x);
			if (tri1_col.y > 255.0f)
				triColG = 255;
			else
				triColG = static_cast<int>(tri1_col.y);
			if (tri1_col.z > 255.0f)
				triColB = 255;
			else
				triColB = static_cast<int>(tri1_col.z);

			ss << (data[i1].x + 0.5f) << " 0.0 " << (data[i1].z + 0.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";
			ss << (data[i2].x + 0.5f) << " 0.0 " << (data[i2].z + 0.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";
			ss << (data[i3].x + 0.5f) << " 0.0 " << (data[i3].z + 0.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";
		}

		int i0 = (nX - 1) + (j + 0) * nX;
		int i1 = 0 + (j + 0) * nX;
		int i2 = (nX - 1) + (j + 1) * nX;
		int i3 = 0 + (j + 1) * nX;

		float tri0_area = nX * nY * abs(((data[i0].x + 0.5f) - (data[i1].x + 1.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 0.5f)) - ((data[i1].x + 1.5f) - (data[i2].x + 0.5f)) * ((data[i0].z + 0.5f) - (data[i1].z + 0.5f)));
		float tri1_area = nX * nY * abs(((data[i1].x + 1.5f) - (data[i2].x + 0.5f)) * ((data[i2].z + 0.5f) - (data[i3].z + 0.5f)) - ((data[i2].x + 0.5f) - (data[i3].x + 1.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 0.5f)));

		glm::vec3 tri0_col(24.0f / tri0_area, 24.0f / tri0_area, 160.0f / tri0_area);
		glm::vec3 tri1_col(24.0f / tri1_area, 24.0f / tri1_area, 160.0f / tri1_area);

		if (tri0_col.x > 255.0f)
			triColR = 255;
		else
			triColR = static_cast<int>(tri0_col.x);
		if (tri0_col.y > 255.0f)
			triColG = 255;
		else
			triColG = static_cast<int>(tri0_col.y);
		if (tri0_col.z > 255.0f)
			triColB = 255;
		else
			triColB = static_cast<int>(tri0_col.z);

		ss << (data[i0].x + 0.5f) << " 0.0 " << (data[i0].z + 0.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";
		ss << (data[i1].x + 1.5f) << " 0.0 " << (data[i1].z + 0.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";
		ss << (data[i2].x + 0.5f) << " 0.0 " << (data[i2].z + 0.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";

		if (tri1_col.x > 255.0f)
			triColR = 255;
		else
			triColR = static_cast<int>(tri1_col.x);
		if (tri1_col.y > 255.0f)
			triColG = 255;
		else
			triColG = static_cast<int>(tri1_col.y);
		if (tri1_col.z > 255.0f)
			triColB = 255;
		else
			triColB = static_cast<int>(tri1_col.z);

		ss << (data[i1].x + 1.5f) << " 0.0 " << (data[i1].z + 0.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";
		ss << (data[i2].x + 0.5f) << " 0.0 " << (data[i2].z + 0.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";
		ss << (data[i3].x + 1.5f) << " 0.0 " << (data[i3].z + 0.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";
	}
	{
		for (unsigned int i = 0; i < nX - 1; ++i)
		{
			int i0 = (i + 0) + (nY - 1) * nX;
			int i1 = (i + 1) + (nY - 1) * nX;
			int i2 = (i + 0) + 0 * nX;
			int i3 = (i + 1) + 0 * nX;

			float tri0_area = nX * nY * abs(((data[i0].x + 0.5f) - (data[i1].x + 0.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 1.5f)) - ((data[i1].x + 0.5f) - (data[i2].x + 0.5f)) * ((data[i0].z + 0.5f) - (data[i1].z + 0.5f)));
			float tri1_area = nX * nY * abs(((data[i1].x + 0.5f) - (data[i2].x + 0.5f)) * ((data[i2].z + 1.5f) - (data[i3].z + 1.5f)) - ((data[i2].x + 0.5f) - (data[i3].x + 0.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 1.5f)));

			glm::vec3 tri0_col(24.0f / tri0_area, 24.0f / tri0_area, 160.0f / tri0_area);
			glm::vec3 tri1_col(24.0f / tri1_area, 24.0f / tri1_area, 160.0f / tri1_area);

			if (tri0_col.x > 255.0f)
				triColR = 255;
			else
				triColR = static_cast<int>(tri0_col.x);
			if (tri0_col.y > 255.0f)
				triColG = 255;
			else
				triColG = static_cast<int>(tri0_col.y);
			if (tri0_col.z > 255.0f)
				triColB = 255;
			else
				triColB = static_cast<int>(tri0_col.z);

			ss << (data[i0].x + 0.5f) << " 0.0 " << (data[i0].z + 0.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";
			ss << (data[i1].x + 0.5f) << " 0.0 " << (data[i1].z + 0.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";
			ss << (data[i2].x + 0.5f) << " 0.0 " << (data[i2].z + 1.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";

			if (tri1_col.x > 255.0f)
				triColR = 255;
			else
				triColR = static_cast<int>(tri1_col.x);
			if (tri1_col.y > 255.0f)
				triColG = 255;
			else
				triColG = static_cast<int>(tri1_col.y);
			if (tri1_col.z > 255.0f)
				triColB = 255;
			else
				triColB = static_cast<int>(tri1_col.z);

			ss << (data[i1].x + 0.5f) << " 0.0 " << (data[i1].z + 0.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";
			ss << (data[i2].x + 0.5f) << " 0.0 " << (data[i2].z + 1.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";
			ss << (data[i3].x + 0.5f) << " 0.0 " << (data[i3].z + 1.5f) << " ";
			ss << triColR << " " << triColG << " " << triColB << "\n";
		}

		int i0 = (nX - 1) + (nY - 1) * nX;
		int i1 = 0 + (nY - 1) * nX;
		int i2 = (nX - 1) + 0 * nX;
		int i3 = 0 + 0 * nX;

		float tri0_area = nX * nY * abs(((data[i0].x + 0.5f) - (data[i1].x + 1.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 1.5f)) - ((data[i1].x + 1.5f) - (data[i2].x + 0.5f)) * ((data[i0].z + 0.5f) - (data[i1].z + 0.5f)));
		float tri1_area = nX * nY * abs(((data[i1].x + 1.5f) - (data[i2].x + 0.5f)) * ((data[i2].z + 1.5f) - (data[i3].z + 1.5f)) - ((data[i2].x + 0.5f) - (data[i3].x + 1.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 1.5f)));

		glm::vec3 tri0_col(24.0f / tri0_area, 24.0f / tri0_area, 160.0f / tri0_area);
		glm::vec3 tri1_col(24.0f / tri1_area, 24.0f / tri1_area, 160.0f / tri1_area);

		if (tri0_col.x > 255.0f)
			triColR = 255;
		else
			triColR = static_cast<int>(tri0_col.x);
		if (tri0_col.y > 255.0f)
			triColG = 255;
		else
			triColG = static_cast<int>(tri0_col.y);
		if (tri0_col.z > 255.0f)
			triColB = 255;
		else
			triColB = static_cast<int>(tri0_col.z);

		ss << (data[i0].x + 0.5f) << " 0.0 " << (data[i0].z + 0.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";
		ss << (data[i1].x + 1.5f) << " 0.0 " << (data[i1].z + 0.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";
		ss << (data[i2].x + 0.5f) << " 0.0 " << (data[i2].z + 1.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";

		if (tri1_col.x > 255.0f)
			triColR = 255;
		else
			triColR = static_cast<int>(tri1_col.x);
		if (tri1_col.y > 255.0f)
			triColG = 255;
		else
			triColG = static_cast<int>(tri1_col.y);
		if (tri1_col.z > 255.0f)
			triColB = 255;
		else
			triColB = static_cast<int>(tri1_col.z);

		ss << (data[i1].x + 1.5f) << " 0.0 " << (data[i1].z + 0.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";
		ss << (data[i2].x + 0.5f) << " 0.0 " << (data[i2].z + 1.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";
		ss << (data[i3].x + 1.5f) << " 0.0 " << (data[i3].z + 1.5f) << " ";
		ss << triColR << " " << triColG << " " << triColB << "\n";
	}

	for (unsigned int j = 0; j < nY; ++j)
	{
		for (unsigned int i = 0; i < nX; ++i)
		{
			ss << 3 << " " << 6 * (i + j * nX) + 1 << " " << 6 * (i + j * nX) + 0 << " " << 6 * (i + j * nX) + 2 << "\n";
			ss << 3 << " " << 6 * (i + j * nX) + 3 << " " << 6 * (i + j * nX) + 4 << " " << 6 * (i + j * nX) + 5 << "\n";
		}
	}

	plyFile << ss.str();
	plyFile.close();
}

void svgPlaneOutput(glm::vec4*& data, unsigned int nX, unsigned int nY, const std::string& fileName, const bool isShell)
{
	std::ofstream svgFile;
	svgFile.open(fileName, std::ios_base::out);
	std::stringstream ss;
	int resolution = 640;

	ss << "<svg height=\"" << resolution << "\" width=\"640\">\n";
	ss << "\t<defs>\n";
	ss << "\t<filter id=\"Screen\">\n";
	ss << "\t\t<feBlend mode=\"screen\" in2=\"BackgroundImage\" in=\"SourceGraphic\" />\n";
	ss << "\t</filter>\n";
	ss << "\t</defs>\n";
	ss << "\t<polygon points=\"0, 0 " << resolution << ", 0 " << resolution << ", " << resolution << " 0, " << resolution << "\" fill=\"rgb(255, 255, 255)\" stroke=\"none\" />\n";
	ss << "\t<g enable-background=\"new\" >\n";
	
	std::string fillType = "\"none\"";
	std::string strokeType = "\"none\" filter=\"url(#Screen)\"";

	if (isShell)
		strokeType = "\"black\"";

	int triColR, triColG, triColB;

	for (unsigned int j = 0; j < nY - 1; ++j)
	{
		for (unsigned int i = 0; i < nX - 1; ++i)
		{
			int i0 = (i + 0) + (j + 0) * nX;
			int i1 = (i + 1) + (j + 0) * nX;
			int i2 = (i + 0) + (j + 1) * nX;
			int i3 = (i + 1) + (j + 1) * nX;

			float tri0_area = nX * nY * abs(((data[i0].x + 0.5f) - (data[i1].x + 0.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 0.5f)) - ((data[i1].x + 0.5f) - (data[i2].x + 0.5f)) * ((data[i0].z + 0.5f) - (data[i1].z + 0.5f)));
			float tri1_area = nX * nY * abs(((data[i1].x + 0.5f) - (data[i2].x + 0.5f)) * ((data[i2].z + 0.5f) - (data[i3].z + 0.5f)) - ((data[i2].x + 0.5f) - (data[i3].x + 0.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 0.5f)));

			glm::vec3 tri0_col(24.0f / tri0_area, 24.0f / tri0_area, 160.0f / tri0_area);
			glm::vec3 tri1_col(24.0f / tri1_area, 24.0f / tri1_area, 160.0f / tri1_area);

			if (tri0_col.x > 255.0f)
				triColR = 255;
			else
				triColR = static_cast<int>(tri0_col.x);
			if (tri0_col.y > 255.0f)
				triColG = 255;
			else
				triColG = static_cast<int>(tri0_col.y);
			if (tri0_col.z > 255.0f)
				triColB = 255;
			else
				triColB = static_cast<int>(tri0_col.z);
			if (!isShell)
				fillType = "\"rgb(" + std::to_string(triColR) + ", " + std::to_string(triColG) + ", " + std::to_string(triColB) + ")\"";

			for (int jTile = -1; jTile <= 1; ++jTile)
			{
				for (int iTile = -1; iTile <= 1; ++iTile)
				{
					ss << "\t\t<polygon points=\"";
					ss << static_cast<float>(resolution) * (data[i0].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i0].z + 0.5f + static_cast<float>(jTile)) << " ";
					ss << static_cast<float>(resolution) * (data[i1].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i1].z + 0.5f + static_cast<float>(jTile)) << " ";
					ss << static_cast<float>(resolution) * (data[i2].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i2].z + 0.5f + static_cast<float>(jTile)) << "\" fill=" << fillType << " stroke=" << strokeType << "/>\n";
				}
			}

			if (tri1_col.x > 255.0f)
				triColR = 255;
			else
				triColR = static_cast<int>(tri1_col.x);
			if (tri1_col.y > 255.0f)
				triColG = 255;
			else
				triColG = static_cast<int>(tri1_col.y);
			if (tri1_col.z > 255.0f)
				triColB = 255;
			else
				triColB = static_cast<int>(tri1_col.z);
			if (!isShell)
				fillType = "\"rgb(" + std::to_string(triColR) + ", " + std::to_string(triColG) + ", " + std::to_string(triColB) + ")\"";

			for (int jTile = -1; jTile <= 1; ++jTile)
			{
				for (int iTile = -1; iTile <= 1; ++iTile)
				{
					ss << "\t\t<polygon points=\"";
					ss << static_cast<float>(resolution) * (data[i1].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i1].z + 0.5f + static_cast<float>(jTile)) << " ";
					ss << static_cast<float>(resolution) * (data[i2].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i2].z + 0.5f + static_cast<float>(jTile)) << " ";
					ss << static_cast<float>(resolution) * (data[i3].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i3].z + 0.5f + static_cast<float>(jTile)) << "\" fill=" << fillType << " stroke=" << strokeType << "/>\n";
				}
			}
		}

		int i0 = (nX - 1) + (j + 0) * nX;
		int i1 = 0 + (j + 0) * nX;
		int i2 = (nX - 1) + (j + 1) * nX;
		int i3 = 0 + (j + 1) * nX;

		float tri0_area = nX * nY * abs(((data[i0].x + 0.5f) - (data[i1].x + 1.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 0.5f)) - ((data[i1].x + 1.5f) - (data[i2].x + 0.5f)) * ((data[i0].z + 0.5f) - (data[i1].z + 0.5f)));
		float tri1_area = nX * nY * abs(((data[i1].x + 1.5f) - (data[i2].x + 0.5f)) * ((data[i2].z + 0.5f) - (data[i3].z + 0.5f)) - ((data[i2].x + 0.5f) - (data[i3].x + 1.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 0.5f)));

		glm::vec3 tri0_col(24.0f / tri0_area, 24.0f / tri0_area, 160.0f / tri0_area);
		glm::vec3 tri1_col(24.0f / tri1_area, 24.0f / tri1_area, 160.0f / tri1_area);

		if (tri0_col.x > 255.0f)
			triColR = 255;
		else
			triColR = static_cast<int>(tri0_col.x);
		if (tri0_col.y > 255.0f)
			triColG = 255;
		else
			triColG = static_cast<int>(tri0_col.y);
		if (tri0_col.z > 255.0f)
			triColB = 255;
		else
			triColB = static_cast<int>(tri0_col.z);
		if (!isShell)
			fillType = "\"rgb(" + std::to_string(triColR) + ", " + std::to_string(triColG) + ", " + std::to_string(triColB) + ")\"";

		for (int jTile = -1; jTile <= 1; ++jTile)
		{
			for (int iTile = -1; iTile <= 1; ++iTile)
			{
				ss << "\t\t<polygon points=\"";
				ss << static_cast<float>(resolution) * (data[i0].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i0].z + 0.5f + static_cast<float>(jTile)) << " ";
				ss << static_cast<float>(resolution) * (data[i1].x + 1.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i1].z + 0.5f + static_cast<float>(jTile)) << " ";
				ss << static_cast<float>(resolution) * (data[i2].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i2].z + 0.5f + static_cast<float>(jTile)) << "\" fill=" << fillType << " stroke=" << strokeType << "/>\n";
			}
		}

		if (tri1_col.x > 255.0f)
			triColR = 255;
		else
			triColR = static_cast<int>(tri1_col.x);
		if (tri1_col.y > 255.0f)
			triColG = 255;
		else
			triColG = static_cast<int>(tri1_col.y);
		if (tri1_col.z > 255.0f)
			triColB = 255;
		else
			triColB = static_cast<int>(tri1_col.z);
		if (!isShell)
			fillType = "\"rgb(" + std::to_string(triColR) + ", " + std::to_string(triColG) + ", " + std::to_string(triColB) + ")\"";

		for (int jTile = -1; jTile <= 1; ++jTile)
		{
			for (int iTile = -1; iTile <= 1; ++iTile)
			{
				ss << "\t\t<polygon points=\"";
				ss << static_cast<float>(resolution) * (data[i1].x + 1.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i1].z + 0.5f + static_cast<float>(jTile)) << " ";
				ss << static_cast<float>(resolution) * (data[i2].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i2].z + 0.5f + static_cast<float>(jTile)) << " ";
				ss << static_cast<float>(resolution) * (data[i3].x + 1.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i3].z + 0.5f + static_cast<float>(jTile)) << "\" fill=" << fillType << " stroke=" << strokeType << "/>\n";
			}
		}
	}
	{
		for (unsigned int i = 0; i < nX - 1; ++i)
		{
			int i0 = (i + 0) + (nY - 1) * nX;
			int i1 = (i + 1) + (nY - 1) * nX;
			int i2 = (i + 0) + 0 * nX;
			int i3 = (i + 1) + 0 * nX;

			float tri0_area = nX * nY * abs(((data[i0].x + 0.5f) - (data[i1].x + 0.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 1.5f)) - ((data[i1].x + 0.5f) - (data[i2].x + 0.5f)) * ((data[i0].z + 0.5f) - (data[i1].z + 0.5f)));
			float tri1_area = nX * nY * abs(((data[i1].x + 0.5f) - (data[i2].x + 0.5f)) * ((data[i2].z + 1.5f) - (data[i3].z + 1.5f)) - ((data[i2].x + 0.5f) - (data[i3].x + 0.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 1.5f)));

			glm::vec3 tri0_col(24.0f / tri0_area, 24.0f / tri0_area, 160.0f / tri0_area);
			glm::vec3 tri1_col(24.0f / tri1_area, 24.0f / tri1_area, 160.0f / tri1_area);

			if (tri0_col.x > 255.0f)
				triColR = 255;
			else
				triColR = static_cast<int>(tri0_col.x);
			if (tri0_col.y > 255.0f)
				triColG = 255;
			else
				triColG = static_cast<int>(tri0_col.y);
			if (tri0_col.z > 255.0f)
				triColB = 255;
			else
				triColB = static_cast<int>(tri0_col.z);
			if (!isShell)
				fillType = "\"rgb(" + std::to_string(triColR) + ", " + std::to_string(triColG) + ", " + std::to_string(triColB) + ")\"";

			for (int jTile = -1; jTile <= 1; ++jTile)
			{
				for (int iTile = -1; iTile <= 1; ++iTile)
				{
					ss << "\t\t<polygon points=\"";
					ss << static_cast<float>(resolution) * (data[i0].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i0].z + 0.5f + static_cast<float>(jTile)) << " ";
					ss << static_cast<float>(resolution) * (data[i1].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i1].z + 0.5f + static_cast<float>(jTile)) << " ";
					ss << static_cast<float>(resolution) * (data[i2].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i2].z + 1.5f + static_cast<float>(jTile)) << "\" fill=" << fillType << " stroke=" << strokeType << "/>\n";
				}
			}

			if (tri1_col.x > 255.0f)
				triColR = 255;
			else
				triColR = static_cast<int>(tri1_col.x);
			if (tri1_col.y > 255.0f)
				triColG = 255;
			else
				triColG = static_cast<int>(tri1_col.y);
			if (tri1_col.z > 255.0f)
				triColB = 255;
			else
				triColB = static_cast<int>(tri1_col.z);
			if (!isShell)
				fillType = "\"rgb(" + std::to_string(triColR) + ", " + std::to_string(triColG) + ", " + std::to_string(triColB) + ")\"";

			for (int jTile = -1; jTile <= 1; ++jTile)
			{
				for (int iTile = -1; iTile <= 1; ++iTile)
				{
					ss << "\t\t<polygon points=\"";
					ss << static_cast<float>(resolution) * (data[i1].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i1].z + 0.5f + static_cast<float>(jTile)) << " ";
					ss << static_cast<float>(resolution) * (data[i2].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i2].z + 1.5f + static_cast<float>(jTile)) << " ";
					ss << static_cast<float>(resolution) * (data[i3].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i3].z + 1.5f + static_cast<float>(jTile)) << "\" fill=" << fillType << " stroke=" << strokeType << "/>\n";
				}
			}
		}
		
		int i0 = (nX - 1) + (nY - 1) * nX;
		int i1 = 0 + (nY - 1) * nX;
		int i2 = (nX - 1) + 0 * nX;
		int i3 = 0 + 0 * nX;

		float tri0_area = nX * nY * abs(((data[i0].x + 0.5f) - (data[i1].x + 1.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 1.5f)) - ((data[i1].x + 1.5f) - (data[i2].x + 0.5f)) * ((data[i0].z + 0.5f) - (data[i1].z + 0.5f)));
		float tri1_area = nX * nY * abs(((data[i1].x + 1.5f) - (data[i2].x + 0.5f)) * ((data[i2].z + 1.5f) - (data[i3].z + 1.5f)) - ((data[i2].x + 0.5f) - (data[i3].x + 1.5f)) * ((data[i1].z + 0.5f) - (data[i2].z + 1.5f)));

		glm::vec3 tri0_col(24.0f / tri0_area, 24.0f / tri0_area, 160.0f / tri0_area);
		glm::vec3 tri1_col(24.0f / tri1_area, 24.0f / tri1_area, 160.0f / tri1_area);

		if (tri0_col.x > 255.0f)
			triColR = 255;
		else
			triColR = static_cast<int>(tri0_col.x);
		if (tri0_col.y > 255.0f)
			triColG = 255;
		else
			triColG = static_cast<int>(tri0_col.y);
		if (tri0_col.z > 255.0f)
			triColB = 255;
		else
			triColB = static_cast<int>(tri0_col.z);
		if (!isShell)
			fillType = "\"rgb(" + std::to_string(triColR) + ", " + std::to_string(triColG) + ", " + std::to_string(triColB) + ")\"";

		for (int jTile = -1; jTile <= 1; ++jTile)
		{
			for (int iTile = -1; iTile <= 1; ++iTile)
			{
				ss << "\t\t<polygon points=\"";
				ss << static_cast<float>(resolution) * (data[i0].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i0].z + 0.5f + static_cast<float>(jTile)) << " ";
				ss << static_cast<float>(resolution) * (data[i1].x + 1.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i1].z + 0.5f + static_cast<float>(jTile)) << " ";
				ss << static_cast<float>(resolution) * (data[i2].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i2].z + 1.5f + static_cast<float>(jTile)) << "\" fill=" << fillType << " stroke=" << strokeType << "/>\n";
			}
		}

		if (tri1_col.x > 255.0f)
			triColR = 255;
		else
			triColR = static_cast<int>(tri1_col.x);
		if (tri1_col.y > 255.0f)
			triColG = 255;
		else
			triColG = static_cast<int>(tri1_col.y);
		if (tri1_col.z > 255.0f)
			triColB = 255;
		else
			triColB = static_cast<int>(tri1_col.z);
		if (!isShell)
			fillType = "\"rgb(" + std::to_string(triColR) + ", " + std::to_string(triColG) + ", " + std::to_string(triColB) + ")\"";

		for (int jTile = -1; jTile <= 1; ++jTile)
		{
			for (int iTile = -1; iTile <= 1; ++iTile)
			{
				ss << "\t\t<polygon points=\"";
				ss << static_cast<float>(resolution) * (data[i1].x + 1.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i1].z + 0.5f + static_cast<float>(jTile)) << " ";
				ss << static_cast<float>(resolution) * (data[i2].x + 0.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i2].z + 1.5f + static_cast<float>(jTile)) << " ";
				ss << static_cast<float>(resolution) * (data[i3].x + 1.5f + static_cast<float>(iTile)) << ", " << static_cast<float>(resolution) * (data[i3].z + 1.5f + static_cast<float>(jTile)) << "\" fill=" << fillType << " stroke=" << strokeType << "/>\n";
			}
		}
	}

	ss << "\t</g>\n";
	ss << "</svg>\n";

	svgFile << ss.str();
	svgFile.close();
}

void plyNormalOutput(glm::vec4*& data, glm::vec4*& dataNormal, unsigned int nX, unsigned int nY, float stepX, float stepZ, const std::string& fileName)
{
	std::ofstream plyFile;
	plyFile.open(fileName, std::ios_base::out);
	std::stringstream ss;

	ss << "ply\n";
	ss << "format ascii 1.0\n";
	ss << "element vertex " << 8 * (nX + 1) * (nY + 1) << "\n";
	ss << "property float x\n";
	ss << "property float y\n";
	ss << "property float z\n";
	ss << "element face " << 2 * (nX + 1) * (nY + 1) << "\n";
	ss << "property list uchar int vertex_indices\n";
	ss << "end_header\n";

	glm::vec4 normalVerts[] = {
		glm::vec4(stepX / 10.0f, 0.0f, stepZ / 10.0f, 1.0f),
		glm::vec4(stepX / 10.0f, 0.025f, stepZ / 10.0f, 1.0f),
		glm::vec4(-stepX / 10.0f, 0.025f, -stepZ / 10.0f, 1.0f),
		glm::vec4(-stepX / 10.0f, 0.0f, -stepZ / 10.0f, 1.0f),
		glm::vec4(-stepX / 10.0f, 0.0f, stepZ / 10.0f, 1.0f),
		glm::vec4(-stepX / 10.0f, 0.025f, stepZ / 10.0f, 1.0f),
		glm::vec4(stepX / 10.0f, 0.025f, -stepZ / 10.0f, 1.0f),
		glm::vec4(stepX / 10.0f, 0.0f, -stepZ / 10.0f, 1.0f)
	};

	for (unsigned int j = 0; j < nY; ++j)
	{
		for (unsigned int i = 0; i < nX; ++i)
		{
			glm::mat4 translateMat = glm::translate(glm::mat4(1.f), glm::vec3(data[i + j * nX].x, data[i + j * nX].y, data[i + j * nX].z));
			glm::mat4 orientMat = rotationTransform(dataNormal[i + j * nX]);
			
			for (unsigned int it = 0; it < 8; ++it)
			{
				glm::vec4 normalVec = translateMat * orientMat * normalVerts[it];
				ss << normalVec.x << " " << normalVec.y << " " << normalVec.z << "\n";
			}
		}

		glm::mat4 translateMat = glm::translate(glm::mat4(1.f), glm::vec3(1.0f + data[j * nX].x, data[j * nX].y, data[j * nX].z));
		glm::mat4 orientMat = rotationTransform(dataNormal[j * nX]);

		for (unsigned int it = 0; it < 8; ++it)
		{
			glm::vec4 normalVec = translateMat * orientMat * normalVerts[it];
			ss << normalVec.x << " " << normalVec.y << " " << normalVec.z << "\n";
		}
	}
	{
		unsigned int j = 0;
		for (unsigned int i = 0; i < nX; ++i)
		{
			glm::mat4 translateMat = glm::translate(glm::mat4(1.f), glm::vec3(data[i + j * nX].x, data[i + j * nX].y, 1.0f + data[i + j * nX].z));
			glm::mat4 orientMat = rotationTransform(dataNormal[i + j * nX]);

			for (unsigned int it = 0; it < 8; ++it)
			{
				glm::vec4 normalVec = translateMat * orientMat * normalVerts[it];
				ss << normalVec.x << " " << normalVec.y << " " << normalVec.z << "\n";
			}
		}
		glm::mat4 translateMat = glm::translate(glm::mat4(1.f), glm::vec3(1.0f + data[j * nX].x, data[j * nX].y, 1.0f + data[j * nX].z));
		glm::mat4 orientMat = rotationTransform(dataNormal[j * nX]);

		for (unsigned int it = 0; it < 8; ++it)
		{
			glm::vec4 normalVec = translateMat * orientMat * normalVerts[it];
			ss << normalVec.x << " " << normalVec.y << " " << normalVec.z << "\n";
		}
	}

	for (unsigned int it = 0; it < (nX + 1)*(nY + 1); ++it)
	{
		ss << 4 << " " << 8 * it + 0 << " " << 8 * it + 1 << " " << 8 * it + 2 << " " << 8 * it + 3 << "\n";
		ss << 4 << " " << 8 * it + 4 << " " << 8 * it + 5 << " " << 8 * it + 6 << " " << 8 * it + 7 << "\n";
	}

	plyFile << ss.str();
	plyFile.close();
}

void binOutput(glm::vec2*& data, unsigned int nX, unsigned int nY, unsigned int nT, const std::string& fileName)
{
	std::ofstream out;
	out.open(fileName, std::ios::out | std::ios::binary);
	out.write(reinterpret_cast<const char*>(data), sizeof(glm::vec2) * nX * nY * nT);
	out.close();
}
