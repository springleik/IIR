//
//  main.cpp
//  iir
//
//  Created by Mark Williamsen on 10/6/14.
//  Copyright (c) 2014 Williamsonic. All rights reserved.
//

#include "iir.h"

bool verbose = true;

int main(int argc, const char *argv[])
{
    // input file, must be opened in binary mode
    ifstream inFile("input.wav", ios::in | ios::binary);
    if (!inFile)
    {
        cerr << "Failed to open input file input.wav." << endl;
        return -1;
    }

    // read WAVE header from disk
    riffWAVEchunk theRiff(inFile);
    if (strncmp(theRiff.chunkID, "RIFF", 4)) {return false;}
    if (strncmp(theRiff.format, "WAVE", 4)) {return false;}
    
    // obtain a list of chunks and file positions
    map<string, long int> theMap;
    theMap["RIFF"] = 0;
    while (inFile)
    {
        long thePos = inFile.tellg();
        chunkWAVEchunk theChunk(inFile);
        if (!inFile) {break;}
        char s[] = "ABCD";
        memcpy(s, theChunk.chunkID, 4);
        theMap[s] = thePos;
        inFile.seekg(theChunk.chunkSize, inFile.cur);
    };
    
    // show list contents on console
    cout << "Chunk List:" << endl;
    for (auto i = theMap.begin(); i != theMap.end(); i++)
    {
        cout << ' ' << i->first << ',' << i->second << endl;
    }
    
    // populate required common chunk
    long chunkPos = 0;
    chunkPos = theMap.find("fmt ")->second;
    if (!chunkPos)
    {
        cerr << "Required fmt chunk not found!" << endl;
        return -2;
    }
    inFile.clear();
    inFile.seekg(chunkPos, inFile.beg);
    formatWAVEchunk theFormat(inFile);
    if (!inFile)
    {
        cerr << "Failed to read fmt chunk." << endl;
        return -3;
    }
    
    // populate required sound chunk
    chunkPos = 0;
    chunkPos = theMap.find("data")->second;
    if (!chunkPos)
    {
        cerr << "Required data chunk not found!" << endl;
        return -4;
    }
    inFile.clear();
    inFile.seekg(chunkPos, inFile.beg);
    dataWAVEchunk theData(inFile);
    if (!inFile)
    {
        cerr << "Failed to read data chunk." << endl;
        return -5;
    }
    
    // show results on console
    if (verbose)
    {
        theRiff.showDetails(cout);
        theFormat.showDetails(cout);
        theData.showDetails(cout);
    }
    
    // output file, must be opened in binary mode
    ofstream outFile("output.wav", ios::out | ios::binary);
    
    // write WAVE header to file
    outFile.write((char *)&theRiff,   sizeof(theRiff));
    outFile.write((char *)&theFormat, sizeof(theFormat));
    outFile.write((char *)&theData,   sizeof(theData));
   
    // transfer data from input to ouput
    short a = 0, b = 0, c = 0, d = 0;
    double lx[3] = {0}, ly[3] = {0}, rx[3] = {0}, ry[3] = {0};
    
    // enter filter coefficients
    const double la1 = -1.80;
    const double la2 =  0.82;
    const double lb0 =  0.00;
    const double lb1 =  0.02;
    const double lb2 =  0.00;
    
    const double ra1 = -1.80;
    const double ra2 =  0.82;
    const double rb0 =  0.90;
    const double rb1 = -1.80;
    const double rb2 =  0.90;
    
    while(inFile)
    {
        // read data from input
        inFile.read((char *)&a, 2);
        inFile.read((char *)&b, 2);
        
        // computer IIR filters
        lx[2] = lx[1]; lx[1] = lx[0];
        rx[2] = rx[1]; rx[1] = rx[0];

        lx[0] = double(a);
        rx[0] = double(b);
        
        ly[2] = ly[1]; ly[1] = ly[0];
        ry[2] = ry[1]; ry[1] = ry[0];

        ly[0] = -la1*ly[1] - la2*ly[2] + lb0*lx[0] + lb1*lx[1] + lb2*lx[2];
        ry[0] = -ra1*ry[1] - ra2*ry[2] + rb0*rx[0] + rb1*rx[1] + rb2*rx[2];
        
        // write data to output
        c = short(ly[0]);
        d = short(ry[0]);
        outFile.write((const char *)&c, 2);
        outFile.write((const char *)&d, 2);
    }
    
    // all done
    inFile.close();
    outFile.close();
    
    return 0;
}

