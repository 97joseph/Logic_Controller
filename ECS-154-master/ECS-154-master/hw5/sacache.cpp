//Christopher Chan
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>

#define READ 0
#define MAIN_MEM_SIZE 65536
#define TAG_BITS 2048 //number of unique tags
#define SET_BITS 8 //number of sets
#define OFFSET_BITS 4 //number of offsets
#define NUM_LINES 5//number of lines per set
#define EMPTY -1
#define LRU 4
#define MRU 0

using namespace std;

class Line
{
    public:
    bool dirty;
    int pos;
    int tag;
    unsigned int *offset;
};


class Cache
{
    public:
    Line **set;
    int ***mainMemory;

    int tempLine;
    int tempTag;
    unsigned int tempOffset;
    unsigned int tempSet;
    bool hit;
    ofstream outputFile;

    Cache();
    void read(unsigned int address);
    void write(unsigned int address, unsigned int inputData);

    void extractInfo(unsigned int address);

    void writeToFile();
    bool compareTags();

    bool isEmpty();
    void cacheMainMemData();
    void setCheckBits();
    void searchLRU();
    void checkForDirty();
    void returnLine();
    void writeToCache(unsigned int input);    

};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////main

int main(int argc, char** argv)
{
    string textLine;
    ifstream testFile;
    unsigned int address;
    unsigned int operation;
    unsigned int byteData;


    testFile.open(argv[1]);

    Cache obj;

    (obj.outputFile).open("sa-out.txt");

    while(getline(testFile,textLine) )
    {
        istringstream iss(textLine);
        iss>> std::hex >> address;
        iss>> std::hex >> operation;
        iss>> std::hex >> byteData;

        if(operation == READ)
            obj.read(address);
        else
            obj.write(address, byteData);
    }
        testFile.close();
            (obj.outputFile).close();

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// Class definitions
Cache::Cache()
{
    mainMemory = new int**[TAG_BITS];
    for(int i = 0; i< TAG_BITS; i++)
    {
        mainMemory[i] = new int*[SET_BITS];
        for(int j = 0; j< SET_BITS; j++)
        {
            mainMemory[i][j] = new int[OFFSET_BITS];
            for(int k = 0; k< OFFSET_BITS; k++)
                mainMemory[i][j][k] = 0;
        }
    }//for loop to dynamically allocate & initialize all entries of main memory to 0
    
    set = new Line*[SET_BITS];
    for(int i = 0; i< SET_BITS; i++)
    {
	set[i] = new Line[NUM_LINES];
	for(int j = 0; j< NUM_LINES; j++)
	{
	    (set[i][j]).dirty = false;
	    (set[i][j]).pos = j;
	    (set[i][j]).tag = EMPTY;
	    (set[i][j]).offset = new unsigned int[OFFSET_BITS];
	}
    }//for loop initializes the cache to 8 sets, each containing 5 lines with each line containing 4 1-byte blocks    


}

void Cache::extractInfo(unsigned int address)
{
    unsigned int mask;

    mask = (1 << (2-0)) - 1;
    this->tempOffset= (address >> 0) & mask;//extracts bits 1~2 (offset)

    mask = (1 << (5-2)) - 1;
    this->tempSet = (address >> 2) & mask;//extracts bits 3-5 (Set)

    mask = (1 << (16-5)) - 1;
    this->tempTag = (address >> 5) & mask;//extracts bits 6~16 (tag)
}

bool Cache::isEmpty()
{
    for(int i = 0; i< NUM_LINES; i++)
    {
        if((set[tempSet][i]).tag == tempTag )
        {
	    this->tempLine = i;
	    return false;
	}
	if((set[tempSet][i]).tag == EMPTY)
	{
	    this->tempLine = i;
	    return true;
	}
    }
    
    return false;
}


void Cache::read(unsigned int address)
{
    this->extractInfo(address);
    if(this->isEmpty() == true)
	cacheMainMemData();
    else if(this->compareTags() == false)
    {
	searchLRU();
	checkForDirty();
	cacheMainMemData();		
    }
    else
    {
	this->hit = true;
	returnLine();
	setCheckBits();
    }

    writeToFile();
}

void Cache::writeToFile()
{
    stringstream stream;
    stream << std::hex << std::uppercase << (set[tempSet][tempLine]).offset[tempOffset]; 
    this->outputFile << setfill('0') << setw(2) << stream.str() << " " << this->hit<< " " << (set[tempSet][tempLine]).dirty << endl;

}

void Cache::returnLine()
{
    for(int i = 0; i < NUM_LINES; i++)
    {
	if((set[tempSet][i]).tag == tempTag)
	{
            tempLine = i;
	    return;
	}
    }
}

void Cache::searchLRU()
{
    for(int i = 0; i< NUM_LINES; i++)
    {
	if((set[tempSet][i]).pos == LRU)
	{
	    tempLine = i;
	    return;
	}
    }
}

void Cache::checkForDirty()
{
    if((set[tempSet][tempLine]).dirty == true)
    {
	for(int i = 0; i< OFFSET_BITS; i++)
	    mainMemory[ (set[tempSet][tempLine]).tag ][tempSet][i]  = set[tempSet][tempLine].offset[i];
    }
}

bool Cache::compareTags()
{
    for(int i = 0; i< NUM_LINES; i++)
    {
	if((set[tempSet][i]).tag == tempTag)
	{
	    tempLine = i;
	    return true;
	}
    }

    return false;
}

void Cache::cacheMainMemData()
{
    for(int i = 0; i< OFFSET_BITS; i++)
    {
	(set[tempSet][tempLine]).offset[i] = mainMemory[tempTag][tempSet][i];
	(set[tempSet][tempLine]).dirty = false;
	(set[tempSet][tempLine]).tag = tempTag;
	this->hit = false;
	setCheckBits();	
    }    
}

void Cache::setCheckBits()
{
    if((set[tempSet][tempLine]).pos == MRU)
	return;
    else
    {
	for(int i = 0; i< NUM_LINES; i++)
	{
           if((set[tempSet][i]).pos < set[tempSet][tempLine].pos)
	   (set[tempSet][i]).pos++;
	}
	(set[tempSet][tempLine]).pos = MRU;
    }
}

void Cache::write(unsigned int address, unsigned int inputData)
{
    this->extractInfo(address);
    if(this->isEmpty() == true)
    {	
	cacheMainMemData();
	writeToCache(inputData);
    }
    else if(this->compareTags() == false)
    {
	searchLRU();
	checkForDirty();
	cacheMainMemData();
	writeToCache(inputData);
    }
    else
    {
	returnLine();
	writeToCache(inputData);
	setCheckBits();
    }
}

void Cache::writeToCache(unsigned int input)
{  
    (set[tempSet][tempLine]).offset[tempOffset] = input;
    (set[tempSet][tempLine]).dirty = true;
}
