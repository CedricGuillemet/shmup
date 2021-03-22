

unsigned char* movieData = NULL;
unsigned char* movieFrame = NULL;
bool ReadMovie(const char* szPath)
{
	if (movieData)
	{
		free(movieData);
		movieData = NULL;
		movieFrame = NULL;
	}
	FILE* fp = fopen(szPath, "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		movieData = (unsigned char*)malloc(size);
		movieFrame = movieData;
		fseek(fp, 0, SEEK_SET);
		fread(movieData, size, 1, fp);
		fclose(fp);
		return true;
	}
	return false;
}