//Christopher Chan
#include <iostream>
#include <iomanip>
#include <fstream> 
#include <string>
#include <sstream>

#define READ 0
#define TAG_BITS 256
#define LINE_BITS 64
#define OFFSET_BITS 4
#define EMPTY -1

using namespace std;


class Data
{
    public:
    int tag;
    unsigned int byteData;
    bool dirty;

};


class Cache{
public:
    Data **cache;
    int ***mainMemory;
    unsigned int tempLine;
    int tempTag;
    unsigned int tempOffset;
    bool hit;
    ofstream outputFile;

    Cache();
    void read(unsigned int address);
    void write(unsigned int address, unsigned int inputData);
    void extractInfo(unsigned int address);
    void writeToFile();
    bool compareTags(int tempTag, int tag);
    bool isEmpty();
    void cacheMainMemData();
    void checkForDirty();
    void writeToCache(unsigned int input);
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////Main

int main(int argc, char** argv)
{
    string textLine;
    ifstream testFile;
    unsigned int address;
    unsigned int operation;
    unsigned int byteData;
    
 
    testFile.open(argv[1]);

    Cache obj;
    
    (obj.outputFile).open("dm-out.txt");
 
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
//to test if it's reading in the file correctly
//	cout<< textLine[0] << textLine[1] << textLine[2]<< textLine[3]<< textLine[5]<< textLine[6] << textLine[8] << textLine[9];
//	cout<<" = " << address<< " "<< operation<< " " << data<<  endl; 	
		
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
        mainMemory[i] = new int*[LINE_BITS];
        for(int j = 0; j< LINE_BITS; j++)
        {
            mainMemory[i][j] = new int[OFFSET_BITS];
            for(int k = 0; k< OFFSET_BITS; k++)
                mainMemory[i][j][k] = 0;
        }
    }//for loop to dynamically allocate & initialize all entries of main memory to 0
    cache = new Data*[LINE_BITS];
    for(int i = 0; i< LINE_BITS; i++)
        cache[i] = new Data[OFFSET_BITS];
    for(int i = 0; i< LINE_BITS; i++)
    {
        for(int j = 0; j< OFFSET_BITS; j++)
        {
            (cache[i][j]).tag = EMPTY;
            (cache[i][j]).dirty = false;
        }
    }//initializes cache, EMPTY == cache slot is empty and contains no data
}


void Cache::read(unsigned int address)
{
    this->extractInfo(address);
    if(this->isEmpty() == true)
        cacheMainMemData();
//if cache entry is empty

    else if(this->compareTags(tempTag, (cache[tempLine][tempOffset]).tag) == false)
    {
	checkForDirty();
	cacheMainMemData();	
    }//else if there is cache entry, but tag doesn't match
    else
    {
        this->hit = true;
    }//else not empty

    writeToFile();
}

void Cache::write(unsigned int address, unsigned int inputData)
{
    this->extractInfo(address);
    if(this->isEmpty() == true)
    {
        cacheMainMemData();
	writeToCache(inputData);
    }//if cache entry was empty
		
    else if(this->compareTags(tempTag, (cache[tempLine][tempOffset]).tag) == false)
    {
        checkForDirty();
        cacheMainMemData();
	writeToCache(inputData);
    }//else if tag doesn't match

    else
    {
        this->hit = true;
	writeToCache(inputData);
    }//else cache entry exists with identical tag
}

void Cache::extractInfo(unsigned int address)
{
    unsigned int mask;

    mask = (1 << (2-0)) - 1;
    this->tempOffset= (address >> 0) & mask;//extracts bits 1~2 (offset)

    mask = (1 << (8-2)) - 1;
    this->tempLine = (address >> 2) & mask;//extracts bits 3-8 (line)

    mask = (1 << (16-8)) - 1;
    this->tempTag = (address >> 8) & mask;//extracts bits 9~16 (tag)
}

void Cache::writeToFile()
{
    stringstream stream;
    stream<< std::hex<< std::uppercase<< (cache[tempLine][tempOffset]).byteData;
    this->outputFile << setfill('0') << setw(2)<<stream.str()<< " "<< this->hit<<" "<< (cache[tempLine][tempOffset]).dirty<< endl;
}

bool Cache::compareTags(int tempTag, int tag)
{
    if(tempTag == tag)
	return true;
    else
	return false;
} 

bool Cache::isEmpty()
{
    if((cache[tempLine][tempOffset]).tag == EMPTY)
	return true;
    else
	return false;
}

void Cache::cacheMainMemData()
{
    for(int i = 0; i< OFFSET_BITS; i++)
    {
        (cache[tempLine][i]).byteData = (mainMemory[tempTag][tempLine][i]);
        (cache[tempLine][i]).tag = tempTag;
	(cache[tempLine][i]).dirty = false;
        this->hit = false;
    }
} 

void Cache::checkForDirty()
{
    for(int i = 0; i< OFFSET_BITS; i++)
    {
	if((cache[tempLine][i]).dirty == true)
	{
	    mainMemory[(cache[tempLine][i]).tag][tempLine][i] = (cache[tempLine][i]).byteData;	     
	}
    }   
}   

void Cache::writeToCache(unsigned int input)
{
    (cache[tempLine][tempOffset]).byteData = input;
    for(int i = 0; i < OFFSET_BITS; i++)
        (cache[tempLine][i]).dirty = true;
}
