#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>

#include "C:\Users\d17jo\Desktop\CSE-687-Object-Oriented-Design-Project-2-main\mapDLL\mapDLL\MapInterface.h"
#include "C:\Users\d17jo\Desktop\CSE-687-Object-Oriented-Design-Project-2-main\reduceDLL\reduceDLL\ReduceInterface.h"
#include "File Management.h"

#include <Windows.h>

using std::stringstream;
using std::vector;
using std::string;
using std::wstring;
using std::to_string;
using std::getline;
using std::cout;
using std::cin;
using std::endl;
using std::mbstowcs;

typedef MapInterface* (*CREATE_MAPPER) ();
typedef ReduceInterface* (*CREATE_REDUCER) ();


int main(int argc, char *argv[]) {

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    cout << "Program started. step 1...\n";

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));


    cout << "Program started. step 2...\n";
    //system("pause");




    int R = 0; // this is the total number of processes, which should equal the number of files in the inputDirectory folder

    string fileName = "";  // Temporary
    string fileString = "";  // Temporary
    string inputDirectory = "";  // Temporary
    string outputDirectory = "";  // Temporary
    string tempDirectory = "";  // Temporary

    string executableName = "Project3_static_main.exe";  // Temporary
    string sourceName = "";  // Temporary
    string destinationName = "";  // Temporary



    wchar_t* wtemp = (wchar_t*)malloc(10);
    size_t commandLength = 0;

    string mapped_string;
    string inputFilename = "";
    string tempFilename = "TempFile.txt";
    string tempFileContent;
    string reduced_string;
    string outputFilename = "Final_OutputFile.txt";
    string successString = "";
    string successFilename = "SUCCESS.txt";

    /*
    string executableName = argv[0];
    string sourceName = argv[2];
    string destinationName = argv[3];
    string functionSelector = argv[1]; // this becomes argv[1] when uesd as a parameter in a child process;
    */
    string commandLineArguments = "needexehere needfunctionselectionhere needsourcefilepathhere needdestinationfilepathhere";
    

    cout << "Program started. step 3...\n";

    HMODULE mapDLL = LoadLibraryA("MapDLL.dll"); // load dll for map functions
    if (mapDLL == NULL) // exit main function if mapDLL is not found
    {
        cout << "Failed to load mapDLL." << endl;
        return 1;
    }

    HMODULE reduceDLL = LoadLibraryA("ReduceDLL.dll"); // load dll for library functions
    if (reduceDLL == NULL) // exit main function if reduceDLL is not found
    {
        cout << "Failed to load reduceDLL." << endl;
        return 1;
    }

    cout << "Program started. step 4...\n";

    if (argv[1] == "map") {
        inputDirectory = "";
        outputDirectory = "NULL";
        tempDirectory = "";
    }
    else if (argv[1] == "reduce") {
        inputDirectory = "NULL";
        outputDirectory = "";
        tempDirectory = "";
    }
    else{

    cout << "==== MAP & REDUCE ====\n\n"; // add title

    cout << "Enter the input directory: "; // prompt user to input i/o directories
    cin >> inputDirectory;
    cout << "Enter the output directory: ";
    cin >> outputDirectory;
    cout << "Enter the temp directory: ";
    cin >> tempDirectory;

    }

    //WORKFLOW//
   
    FileManagement FileManage(inputDirectory, outputDirectory, tempDirectory); //Create file management class based on the user inputs
    cout << "FileManagement Class initialized.\n";

    if (argv[1] == "map") {

        CREATE_MAPPER mapperPtr = (CREATE_MAPPER)GetProcAddress(mapDLL, "CreateMap"); // create pointer to function to create new Map object
        MapInterface* pMapper = mapperPtr();

        fileString = FileManage.ReadSingleFile(inputFilename);     //Read single file into single string
        cout << "Single file read.\n";

        cout << "Strings from files passed to map function.\n";
        pMapper->map(fileString);

        cout << "Mapping complete; exporting resulting string.\n";
        mapped_string = pMapper->vector_export();     //Write mapped output string to intermediate file 

        FileManage.WriteToTempFile(tempFilename, mapped_string);
        cout << "String from mapping written to temp file.\n";

        return 0;
    }

    if (argv[1] == "reduce") {

        CREATE_REDUCER reducerPtr = (CREATE_REDUCER)GetProcAddress(reduceDLL, "CreateReduce");  // create pointer to function to create new Reduce object
        ReduceInterface* pReducer = reducerPtr();

        //Read from intermediate file and pass data to Reduce class
        tempFileContent = FileManage.ReadFromTempFile(tempFilename);
        cout << "New string read from temp file.\n";

        pReducer->import(tempFileContent);
        cout << "String imported by reduce class function and placed in vector.\n";

        pReducer->sort();
        cout << "Vector sorted.\n";

        pReducer->aggregate();
        cout << "Vector aggregated.\n";

        pReducer->reduce();
        cout << "Vector reduced.\n";

        reduced_string = pReducer->vector_export();
        cout << "Vector exported to string.\n";

        //Sorted, aggregated, and reduced output string is written into final output file
        FileManage.WriteToOutputFile(outputFilename, reduced_string);
        cout << "string written to output file.\n";

        return 0;
    }

    R = FileManage.getCount();
    vector <string> filenames = FileManage.getFilenames();



    cout << "Creating processes for map function\n";
    for (int i = 0; i < R; i++) {

        inputFilename = inputDirectory + filenames[i];
        tempFilename = tempDirectory + filenames[i] + " temp";

        // argv[0]: executable name; argv[1]: function selector; argv[2]: file name/path, argv[3]: temp directory
        commandLineArguments = executableName  + " map " + inputFilename + " " + tempDirectory;
        commandLength = commandLineArguments.length();

        wtemp = (wchar_t*)malloc(4 * commandLineArguments.size());
        mbstowcs(wtemp, commandLineArguments.c_str(), commandLength); //includes null
        LPWSTR args = wtemp;

        // Start the child process. 
        if (!CreateProcess(
            NULL,
            args,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi)           // Pointer to PROCESS_INFORMATION structure
           )
        {
            printf("CreateProcess for mapper failed (%d).\n", GetLastError());
            return(1);
        }
        cout << " Process was created successfully....";
    }

    free(wtemp); // free memory of wtemp 

    // Wait until all child processes exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    //All child processes for mapping sould be completed at this point;

    /*
    cout << "Creating processes for reduce function\n";
    for (int i = 0; i < R; i++) {

        // argv[0]: executable name; argv[1]: function selector; argv[2]: file name/path, argv[3]: temp directory
        commandLineArguments = executableName + " " + "";
        commandLength = commandLineArguments.length();

        wtemp = (wchar_t*)malloc(4 * commandLineArguments.size());
        mbstowcs(wtemp, commandLineArguments.c_str(), commandLength); //includes null
        LPWSTR args = wtemp;

        // Start the child process. 
        if (!CreateProcess(
            NULL,
            args,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi)           // Pointer to PROCESS_INFORMATION structure
            )
        {
            printf("CreateProcess for reducer failed (%d).\n", GetLastError());
            return(1);
        }
    }

    free(wtemp); // free memory of wtemp  

    //all reduce processes should be done at this point.
    */

    fileString = FileManage.ReadAllFiles();     //Read all file into single string
    cout << "All files read.\n";

    FileManage.WriteToOutputFile(successFilename, successString);
    cout << "Success.\n";
    system("pause");


    FreeLibrary(mapDLL);
    FreeLibrary(reduceDLL);

    return 0;
}
