#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>

#include "C:/Users/d17jo/Desktop/Final Project 2/Dll1/MapInterface.h"
#include "C:/Users/d17jo/Desktop/Final Project 2/Dll1/ReduceInterface.h"
#include "File Management.h"

#include <Windows.h>

using std::stringstream;
using std::vector;
using std::string;
using std::thread;
using std::to_string;
using std::getline;
using std::cout;
using std::cin;
using std::endl;
using std::thread;

typedef MapInterface* (*CREATE_MAPPER) ();
typedef ReduceInterface* (*CREATE_REDUCER) ();

void mapThreadFunction(MapInterface* pMapper, const string& fileString, string& mapped_string) {
    pMapper->map(fileString);
    mapped_string = pMapper->vector_export();
}

void reduceThreadFunction(ReduceInterface* pReducer, const string& tempFileContent, string& reduced_string) {
    pReducer->import(tempFileContent);
    pReducer->sort();
    pReducer->aggregate();
    pReducer->reduce();
    reduced_string = pReducer->vector_export();
}

int main() {
    cout << "Program started. Press any key to continue...\n";
    cin;
    string fileName = "";
    string fileString = "";
    string inputDirectory = "";
    string outputDirectory = "";
    string tempDirectory = "";
    string mapped_string;
    string tempFilename = "TempFile.txt";
    string tempFileContent;
    string reduced_string;
    string outputFilename = "Final_OutputFile.txt";
    string successString = "";
    string successFilename = "SUCCESS.txt";

    HMODULE mapDLL = LoadLibrary("MapDLL.dll"); // load dll for map functions
    HMODULE reduceDLL = LoadLibrary("ReduceDLL.dll"); // load dll for library functions

    if (mapDLL == NULL) // exit main function if mapDLL is not found
    {
        cout << "Failed to load mapDLL." << endl;
        return 1;
    }

    if (reduceDLL == NULL) // exit main function if reduceDLL is not found
    {
        cout << "Failed to load reduceDLL." << endl;
        return 1;
    }

    cout << "==== MAP & REDUCE ====\n\n"; // add title
    cout << "Enter the input directory: "; // prompt user to input i/o directories
    cin >> inputDirectory;
    cout << "Enter the output directory: ";
    cin >> outputDirectory;
    cout << "Enter the temp directory: ";
    cin >> tempDirectory;

    //WORKFLOW//

    CREATE_MAPPER mapperPtr = (CREATE_MAPPER)GetProcAddress(mapDLL, "CreateMap");
    MapInterface* pMapper = mapperPtr();

    CREATE_REDUCER reducerPtr = (CREATE_REDUCER)GetProcAddress(reduceDLL, "CreateReduce");
    ReduceInterface* pReducer = reducerPtr();

    FileManagement FileManage(inputDirectory, outputDirectory, tempDirectory);
    cout << "FileManagement Class initialized.\n";

    string fileString = FileManage.ReadAllFiles();
    cout << "All files read.\n";
    cout << "String from files passed to map function.\n";

    string mapped_string;
    thread mapThread(mapThreadFunction, pMapper, std::ref(fileString), std::ref(mapped_string));

    FileManage.WriteToTempFile("TempFile.txt", mapped_string);
    cout << "String from mapping written to temp file.\n";

    string tempFileContent = FileManage.ReadFromTempFile("TempFile.txt");
    cout << "New string read from temp file.\n";

    string reduced_string;
    thread reduceThread(reduceThreadFunction, pReducer, std::ref(tempFileContent), std::ref(reduced_string));

    // Wait for threads to finish
    mapThread.join();
    reduceThread.join();

    // Sorted, aggregated, and reduced output string is written into the final output file
    FileManage.WriteToOutputFile("Final_OutputFile.txt", reduced_string);
    cout << "String written to output file.\n";

    FileManage.WriteToOutputFile("SUCCESS.txt", "");
    cout << "Success.\n";

    FreeLibrary(mapDLL);
    FreeLibrary(reduceDLL);

    return 0;
}