#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>

#include "../../mapDLL/MapDLL/MapInterface.h"
#include "../../ReduceDLL/ReduceDLL/ReduceInterface.h"
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

    cout << "Program started. step 1...\n";

    STARTUPINFO sim;
    PROCESS_INFORMATION pim;

    STARTUPINFO sir;
    PROCESS_INFORMATION pir;

    STARTUPINFO sif;
    PROCESS_INFORMATION pif;

    ZeroMemory(&sim, sizeof(sim));
    sim.cb = sizeof(sim);
    ZeroMemory(&pim, sizeof(pim));

    ZeroMemory(&sir, sizeof(sir));
    sir.cb = sizeof(sir);
    ZeroMemory(&pir, sizeof(pir));

    ZeroMemory(&sif, sizeof(sif));
    sif.cb = sizeof(sif);
    ZeroMemory(&pif, sizeof(pif));


    string functionSelector;
    string sourceName;
    string destinationName;

    cout << "Number of arguments: " << argc << "\n";

    if (argc > 1){
    for (int i = 0; i < (argc - 2); i++) {
        cout << "Argument " << i << ": " << argv[i] << "\n";

    }
        functionSelector = argv[1]; // this becomes argv[1] when uesd as a parameter in a child process;
        sourceName = argv[2];
        destinationName = argv[3];
    }




    cout << "Program started. step 2...\n";
    //system("pause");




    int R = 0; // this is the total number of processes, which should equal the number of files in the inputDirectory folder

    string fileName = "";  // Temporary
    string fileString = "";  // Temporary
    string inputDirectory = "";  // Temporary
    string outputDirectory = "";  // Temporary
    string tempDirectory = "";  // Temporary

    //string sourceName = "";  // Temporary
    //string destinationName = "";  // Temporary



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

    string executableName = argv[0];

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

    if (functionSelector == "map") {
        inputDirectory = "";
        outputDirectory = "NULL";
        tempDirectory = "";
    }
    else if (functionSelector == "reduce") {
        inputDirectory = "NULL";
        outputDirectory = "";
        tempDirectory = "";
    }
    else if (functionSelector == "finalreduce") {
        outputDirectory = argv[2];
    }
    else{

    cout << "==== MAP & REDUCE ====\n\n"; // add title

    //inputDirectory = "../../io_files/input_directory";
    //outputDirectory = "../../io_files/output_directory";
    //tempDirectory = "../../io_files/temp_directory";

    
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

    if (functionSelector == "map") {
        
        CREATE_MAPPER mapperPtr = (CREATE_MAPPER)GetProcAddress(mapDLL, "CreateMap"); // create pointer to function to create new Map object
        MapInterface* pMapper = mapperPtr();

        fileString = FileManage.ReadSingleFile(sourceName);     //Read single file into single string
        //cout << "Single file read.\n";

        //cout << "Strings from files passed to map function.\n";
        pMapper->map(fileString);

        //cout << "Mapping complete; exporting resulting string.\n";
        mapped_string = pMapper->vector_export();     //Write mapped output string to intermediate file 



        FileManage.WriteToTempFile(outputFilename, mapped_string);
        cout << "String from mapping written to temp file.\n";
        
        return 0;
        
    }

    else if (functionSelector == "reduce") {
        
        CREATE_REDUCER reducerPtr = (CREATE_REDUCER)GetProcAddress(reduceDLL, "CreateReduce");  // create pointer to function to create new Reduce object
        ReduceInterface* pReducer = reducerPtr();

        //Read from intermediate file and pass data to Reduce class
        fileString = FileManage.ReadSingleFile(sourceName);     //Read single file into single string
        //cout << "Single file read.\n";

        pReducer->import(fileString);
        //cout << "String imported by reduce class function and placed in vector.\n";

        pReducer->sort();
        //cout << "Vector sorted.\n";

        pReducer->aggregate();
        //cout << "Vector aggregated.\n";

        pReducer->reduce();
        //cout << "Vector reduced.\n";

        reduced_string = pReducer->vector_export();
        //cout << "Vector exported to string.\n";

        //Sorted, aggregated, and reduced output string is written into final output file
        FileManage.WriteToOutputFile(destinationName, reduced_string);
        cout << "Reduced string written to output file.\n";
        

        return 0;

    }

    else if (functionSelector == "finalreduce") {

        CREATE_REDUCER reducerPtr = (CREATE_REDUCER)GetProcAddress(reduceDLL, "CreateReduce");  // create pointer to function to create new Reduce object
        ReduceInterface* pReducer = reducerPtr();

        cout << "Output Directory: " + outputDirectory + "\n";

        //Read from intermediate file and pass data to Reduce class
        fileString = FileManage.ReadAllFiles(); //Read all files in the output folder
        cout << "All files read.\n";

        pReducer->import(fileString);
        cout << fileString << "\n";
        cout << "String imported by reduce class function and placed in vector.\n";

        pReducer->sort();
        //cout << "Vector sorted.\n";

        pReducer->aggregate();
        //cout << "Vector aggregated.\n";

        pReducer->reduce();
        //cout << "Vector reduced.\n";

        reduced_string = pReducer->vector_export();
        //cout << "Vector exported to string.\n";

        outputFilename = "FinalOutput.txt";

        outputFilename = outputDirectory + "/" + outputFilename;

        cout << outputFilename << "\n";
        //Sorted, aggregated, and reduced output string is written into final output file
        FileManage.WriteToOutputFile(outputFilename, reduced_string);
        cout << "Final reduced string written to output file.\n";



        return 0;

    }

    else{
      R = FileManage.getCount();
        vector <string> filenames = FileManage.getFilenames();

    cout << "Creating processes for map function\n";
    for (int i = 0; i < R; i++) {

        inputFilename = inputDirectory + "/" + filenames[i];
        tempFilename = tempDirectory + "/" + filenames[i];

        tempFilename.pop_back(); // remove the existing '.txt'
        tempFilename.pop_back();
        tempFilename.pop_back();
        tempFilename.pop_back();
        
        tempFilename += "temp.txt";

        // argv[0]: executable name; argv[1]: function selector; argv[2]: file name/path, argv[3]: temp directory
        commandLineArguments = "Project3_main_static.exe map " + inputFilename + " " + tempFilename + " ";
        commandLength = commandLineArguments.length();

        wtemp = (wchar_t*)malloc(4 * commandLineArguments.size());
        mbstowcs(wtemp, commandLineArguments.c_str(), commandLength); //includes null
        LPWSTR args = wtemp;

        //child process creation code derived from Microsoft tutorial example here: https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes
        // Start the child map process. 
        if (!CreateProcess(
            NULL, // module name
            args,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &sim,            // Pointer to STARTUPINFO structure
            &pim)           // Pointer to PROCESS_INFORMATION structure
           )
        {
            cout << "CreateProcess for mapper failed" << GetLastError() << "\n";
            
        }
        else {
            cout << "Map process was created successfully...\n";
            WaitForSingleObject(pim.hProcess, INFINITE);
        }

        free(wtemp); // free memory of wtemp

    }
    
    CloseHandle(pim.hProcess);
    CloseHandle(pim.hThread);

    cout << "All mapping processes complete.\n";


    //All child processes for mapping sould be completed at this point.

    FileManage.deleteAllFilesInDirectory(); //Clear output directory contents
    
    cout << "Creating processes for reduce function\n";
    for (int i = 0; i < R; i++) {

        tempFilename = tempDirectory + "/" + filenames[i];

        tempFilename.pop_back(); // remove the existing '.txt'
        tempFilename.pop_back();
        tempFilename.pop_back();
        tempFilename.pop_back();

        tempFilename += "temp.txt";

        outputFilename = outputDirectory + "/" + filenames[i];

        outputFilename.pop_back(); // remove the existing '.txt'
        outputFilename.pop_back();
        outputFilename.pop_back();
        outputFilename.pop_back();

        outputFilename += "output.txt";

        // argv[0]: executable name; argv[1]: function selector; argv[2]: temp path/filename, argv[3]: output path/filename
        commandLineArguments = "Project3_main_static.exe reduce " + tempFilename + " " + outputFilename + " ";
        commandLength = commandLineArguments.length();

        wtemp = (wchar_t*)malloc(4 * commandLineArguments.size());
        mbstowcs(wtemp, commandLineArguments.c_str(), commandLength); //includes null
        LPWSTR args = wtemp;

        //child process creation code derived from Microsoft tutorial example here: https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes
        // Start the child reduce process. 
        if (!CreateProcess(
            NULL, // module name
            args,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &sir,            // Pointer to STARTUPINFO structure
            &pir)           // Pointer to PROCESS_INFORMATION structure
            )
        {
            cout << "CreateProcess for reducer failed" << GetLastError() << "\n";

        }
        else {
            cout << "Reduce process was created successfully...\n";
            WaitForSingleObject(pir.hProcess, INFINITE);
            

        }
        free(wtemp); // free memory of wtemp
    }

    CloseHandle(pir.hProcess);
    CloseHandle(pir.hThread);
    

    cout << "Creating process for final reduce function\n";

        // argv[0]: executable name; argv[1]: function selector; argv[2]: temp path/filename, argv[3]: final output filename
        commandLineArguments = "Project3_main_static.exe finalreduce " + outputDirectory + " " + outputFilename + " ";
        commandLength = commandLineArguments.length();

        wtemp = (wchar_t*)malloc(4 * commandLineArguments.size());
        mbstowcs(wtemp, commandLineArguments.c_str(), commandLength); //includes null
        LPWSTR args = wtemp;

        // Start the child reduce process. 
        if (!CreateProcess(
            NULL, // module name
            args,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &sif,            // Pointer to STARTUPINFO structure
            &pif)           // Pointer to PROCESS_INFORMATION structure
            )
        {
            cout << "CreateProcess for final reducer failed" << GetLastError() << "\n";

        }
        else {
            cout << "Final Reduce Process was created successfully...\n";

            WaitForSingleObject(pif.hProcess, INFINITE);


        }
        free(wtemp); // free memory of wtemp
    }

    CloseHandle(pif.hProcess);
    CloseHandle(pif.hThread);



    cout << "Final reducer process complete.\n";

    //all reducer child processes should be done at this point.
    


    //fileString = FileManage.ReadAllFiles();     //Read all file into single string
    //cout << "All files read.\n";

    FileManage.WriteToOutputFile(successFilename, successString);
    cout << "Success.\n";
    system("pause");


    FreeLibrary(mapDLL);
    FreeLibrary(reduceDLL);

    return 0;
}
