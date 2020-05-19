
struct InputRecord
{
    unsigned int frame : 24;
    unsigned int leftChanged : 1;
    unsigned int rightChanged : 1;
    unsigned int upChanged : 1;
    unsigned int downChanged : 1;
    unsigned int fireChanged : 1;
    unsigned int switchColorChanged : 1;
    unsigned int dischargeChanged : 1;
};

#define MAX_RECORD 10240
struct InputRecord Record[MAX_RECORD];
unsigned int RecordEntryCount = 0;
unsigned int PlaybackIndex = 0;


bool LoadRecord(const char* recordFile)
{
    FILE* fp = fopen(recordFile, "rb");
    if (fp)
    {
        fread(&RecordEntryCount, sizeof(unsigned int), 1, fp);
        fread(Record, sizeof(struct InputRecord), RecordEntryCount, fp);
        fclose(fp);
        return true;
    }
    return false;
}

void SaveRecord(const char* recordFile)
{
    FILE* fp = fopen(recordFile, "wb");
    if (fp)
    {
        fwrite(&RecordEntryCount, sizeof(unsigned int), 1, fp);
        fwrite(Record, sizeof(struct InputRecord), RecordEntryCount, fp);
        fclose(fp);
    }
}

void HandlePlayback(struct Input_t* Input)
{
    struct InputRecord* currentRecord = &Record[PlaybackIndex];
    if (currentRecord->frame == GlobalFrame)
    {
        if (currentRecord->leftChanged)
        {
            Input->left = !Input->left;
        }
        if (currentRecord->rightChanged)
        {
            Input->right = !Input->right;
        }
        if (currentRecord->upChanged)
        {
            Input->up = !Input->up;
        }
        if (currentRecord->downChanged)
        {
            Input->down = !Input->down;
        }
        if (currentRecord->fireChanged)
        {
            Input->fire = !Input->fire;
        }
        if (currentRecord->switchColorChanged)
        {
            Input->switchColor = !Input->switchColor;
        }
        if (currentRecord->dischargeChanged)
        {
            Input->discharge = !Input->discharge;
        }
        PlaybackIndex++;
    }
}