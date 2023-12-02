#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>

#include "C:\Users\moimeme\Downloads\CSE-687-Object-Oriented-Design-Project-2-main\CSE-687-Object-Oriented-Design-Project-2-main\mapDLL\mapDLL\MapInterface.h"
#include "C:\Users\moimeme\Downloads\CSE-687-Object-Oriented-Design-Project-2-main\CSE-687-Object-Oriented-Design-Project-2-main\reduceDLL\reduceDLL\ReduceInterface.h"
#include "File Management.h"

#include <Windows.h>

using std::stringstream;
using std::vector;
using std::string;
using std::to_string;
using std::getline;
using std::cout;
using std::cin;
using std::endl;
/*
typedef void (*MapFunction)(const char* input, char* output);
typedef void (*ReduceFunction)(const char* input, char* output);
*/

typedef MapInterface* (*CREATE_MAPPER) ();
typedef ReduceInterface* (*CREATE_REDUCER) ();

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

PROCESS_INFORMATION piProcInfo;

void CreateChildProcess()
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
    TCHAR szCmdline[] = TEXT("child");
    //PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure. 

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process. 

    bSuccess = CreateProcess(NULL, // No module name (use command line)
        szCmdline,     // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,             // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

    // If an error occurs, exit the application. 
    if (!bSuccess)
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return;
    }
    else
    {
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example. 

        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);

        // Close handles to the stdin and stdout pipes no longer needed by the child process.
        // If they are not explicitly closed, there is no way to recognize that the child process has ended.

        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_IN_Rd);
    }
}

int main() {
    cout << "Program started. Press any key to continue...\n";
    system("pause");

    //int pid = fork();


    int R = 0; // this is the total number of processes, which should equal the number of files in the inputDirectory folder

    string fileName = "";
    string fileString = "";
    string inputDirectory = "";
    string outputDirectory = "";
    string tempDirectory = "";

    string mapped_string;
    string tempFilename = "TempFile";
    string tempFileContent;
    string reduced_string;
    string outputFilename = "Final_OutputFile.txt";
    string successString = "";
    string successFilename = "SUCCESS.txt";
    string finalTempFilename;

    vector <string> fileStringVector;

    HMODULE mapDLL = LoadLibraryA("MapDLL.dll"); // load dll for map functions
    HMODULE reduceDLL = LoadLibraryA("ReduceDLL.dll"); // load dll for library functions

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
    


    FileManagement FileManage(inputDirectory, outputDirectory, tempDirectory); //Create file management class based on the user inputs
    cout << "FileManagement Class initialized.\n";

    R = FileManage.getCount();

    CREATE_MAPPER mapperPtr = (CREATE_MAPPER)GetProcAddress(mapDLL, "CreateMap"); // create pointer to function to create new Map object
    MapInterface* pMapper = mapperPtr();

    CREATE_REDUCER reducerPtr = (CREATE_REDUCER)GetProcAddress(reduceDLL, "CreateReduce");  // create pointer to function to create new Reduce object
    ReduceInterface* pReducer = reducerPtr();


    fileStringVector = FileManage.ReadAllFiles();     //Read all files into single string and pass to Map class
    cout << "All files read.\n";

    cout << "Beginning map function\n";
    for (int i = 0; i < R; i++) {
        
        CreateChildProcess();
        cout << "Strings from files passed to map function.\n";
        pMapper->map(fileStringVector[i]);
     
        mapped_string = pMapper->vector_export();     //Write mapped output string to intermediate file 
        cout << "Mapping complete; exporting resulting string.\n";

        finalTempFilename = tempFilename + to_string(i) + ".txt";

        FileManage.WriteToTempFile(finalTempFilename, mapped_string);
        cout << "String from mapping written to temp file.\n";
    }

    TerminateProcess(piProcInfo.hProcess, 0);

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

    FileManage.WriteToOutputFile(successFilename, successString);
    cout << "Success.\n";
    system("pause");


    FreeLibrary(mapDLL);
    FreeLibrary(reduceDLL);

    return 0;
}
