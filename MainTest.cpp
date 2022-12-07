// Copyright Mass Media. All rights reserved. DO NOT redistribute.

////////////////////////////////////////////////////////////////////////////////////////////////////
// Task List
////////////////////////////////////////////////////////////////////////////////////////////////////
// Notes
//	* This test requires a compiler with C++17 support and was built for Visual Studio 2017.
// 		* Tested on Linux (Ubuntu 20.04) with: g++ -Wall -Wextra -pthread -std=c++17 MainTest.cpp
//		* Tested on Mac OS Big Sur, 11.0.1 and latest XCode updates.
//	* Correct output for all three sorts is in the CorrectOutput folder. BeyondCompare is recommended for comparing output.
//	* Functions, classes, and algorithms can be added and changed as needed.
//	* DO NOT use std::sort().
// Objectives
//	* 20 points - Make the program produce a SingleAscending.txt file that matches CorrectOutput/SingleAscending.txt.
//	* 10 points - Make the program produce a SingleDescending.txt file that matches CorrectOutput/SingleDescending.txt.
//	* 10 points - Make the program produce a SingleLastLetter.txt file that matches CorrectOutput/SingleLastLetter.txt.
//	* 20 points - Write a brief report on what you found, what you did, and what other changes to the code you'd recommend.
//	* 10 points - Make the program produce three MultiXXX.txt files that match the equivalent files in CorrectOutput; it must be multi-threaded.
//	* 20 points - Improve performance as much as possible on both single-threaded and multi-threaded versions; speed is more important than memory usage.
//	* 10 points - Improve safety and stability; fix memory leaks and handle unexpected input and edge cases.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <ctime>
#include <vector>


#include <mutex>

#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#   if defined(__cpp_lib_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#   elif defined(__cpp_lib_experimental_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   elif !defined(__has_include)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   elif __has_include(<filesystem>)
#       ifdef _MSC_VER
#           if __has_include(<yvals_core.h>)
#               include <yvals_core.h>
#               if defined(_HAS_CXX17) && _HAS_CXX17
#                   define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#               endif
#           endif
#           ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#               define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#           endif
#       else
#           define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#       endif
#   elif __has_include(<experimental/filesystem>)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   else
#       error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#   endif
#   if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#       include <experimental/filesystem>
     	namespace fs = std::experimental::filesystem;
#   else
#       include <filesystem>
#		if __APPLE__
namespace fs = std::__fs::filesystem;
#		else
namespace fs = std::filesystem;
#		endif
#   endif
#endif

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Definitions and Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////
#define MULTITHREADED_ENABLED 1

enum class ESortType { AlphabeticalAscending, AlphabeticalDescending, LastLetterAscending };

class IStringComparer {
public:
    virtual bool IsFirstAboveSecond(string _first, string _second) = 0;
};

class AlphabeticalAscendingStringComparer : public IStringComparer {
public:
    bool IsFirstAboveSecond(string _first, string _second);
};

class AlphabeticalDescendingStringComparer : public IStringComparer {
public:
    bool IsFirstAboveSecond(string _first, string _second);
};

class LastLetterAscendingStringComparer : public IStringComparer {
public:
    bool IsFirstAboveSecond(string _first, string _second);
};
void DoSingleThreaded(vector<string>& _fileList, ESortType _sortType, string _outputName);
//void DoMultiThreaded(vector<string> _fileList, ESortType _sortType, string _outputName);
void DoMultiThreaded(vector<string>& _fileList);
vector<string> ReadFile(string _fileName);
//void ThreadedReadFile(string _fileName, vector<string>* _listOut);
void ThreadedReadFile(vector<string> _listToSort, ESortType _sortType);
vector<string> BubbleSort(vector<string>& _listToSort, ESortType _sortType);
vector<string> MergeSort(vector<string>& _listToSort, ESortType _sortType);
void WriteAndPrintResults(const vector<string>& _masterStringList, string _outputName, int _clocksTaken);

void test();
mutex mtx;
////////////////////////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
    // Enumerate the directory for input files
    vector<string> fileList;
    string inputDirectoryPath = "../InputFiles";
    for (const auto & entry : fs::directory_iterator(inputDirectoryPath)) {
        if (!fs::is_directory(entry)) {
            fileList.push_back(entry.path().string());
        }
    }
    // Do the stuff
    clock_t startTime = clock()/1000;
    DoSingleThreaded(fileList, ESortType::AlphabeticalAscending,	"SingleAscending");
    DoSingleThreaded(fileList, ESortType::AlphabeticalDescending,	"SingleDescending");
    DoSingleThreaded(fileList, ESortType::LastLetterAscending,		"SingleLastLetter");
    clock_t endTime = clock()/1000;
    std::cout<<"single thread total time: "<<endTime - startTime<<std::endl;
#if MULTITHREADED_ENABLED
//    DoMultiThreaded(fileList, ESortType::AlphabeticalAscending,		"MultiAscending");
//	DoMultiThreaded(fileList, ESortType::AlphabeticalDescending,	"MultiDescending");
//	DoMultiThreaded(fileList, ESortType::LastLetterAscending,		"MultiLastLetter");
    clock_t multiStartTime = clock()/1000;
    DoMultiThreaded(fileList);
    clock_t multiEndTime = clock()/1000;
    std::cout<<"multi thread total time: "<<multiEndTime - multiStartTime<<std::endl;
#endif

    // Wait
    cout << endl << "Finished...";
    getchar();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// The Stuff
////////////////////////////////////////////////////////////////////////////////////////////////////
void DoSingleThreaded(vector<string>& _fileList, ESortType _sortType, string _outputName) {
    clock_t startTime = clock()/1000;
    vector<string> masterStringList;
    for (unsigned int i = 0; i < _fileList.size(); ++i) {
        vector<string> fileStringList = ReadFile(_fileList[i]);
        for (unsigned int j = 0; j < fileStringList.size(); ++j) {
            masterStringList.push_back(fileStringList[j]);
        }
    }
//    masterStringList = BubbleSort(masterStringList, _sortType);
    masterStringList = MergeSort(masterStringList, _sortType);
    clock_t endTime = clock()/1000;

    WriteAndPrintResults(masterStringList, _outputName, endTime - startTime);
}

//void DoMultiThreaded(vector<string> _fileList, ESortType _sortType, string _outputName) {
//    clock_t startTime = clock();
//    vector<string> masterStringList;
//    vector<thread> workerThreads(_fileList.size());
//    for (unsigned int i = 0; i < _fileList.size() - 1; ++i) {
//        workerThreads[i] = thread(ThreadedReadFile, _fileList[i], &masterStringList);
//    }
//
//    workerThreads[workerThreads.size() - 1].join();
//
//    masterStringList = BubbleSort(masterStringList, _sortType);
//    clock_t endTime = clock();
//
//    WriteAndPrintResults(masterStringList, _outputName, endTime - startTime);
//}

void DoMultiThreaded(vector<string>& _fileList) {
    clock_t startTime = clock();
    std::cout<<"multi thread start"<<startTime<<std::endl;
    vector<vector<string>> threadMasterList;
    vector<string> masterStringList1, masterStringList2, masterStringList3;
//    std::thread workerThreads[_fileList.size()];
    std::thread workerThreads[3];
    for (unsigned int i = 0; i < _fileList.size(); ++i) {
        vector<string> fileStringList = ReadFile(_fileList[i]);

        for (unsigned int j = 0; j < fileStringList.size(); ++j) {
            masterStringList1.push_back(fileStringList[j]);
//            masterStringList2.push_back(fileStringList[j]);
//            masterStringList3.push_back(fileStringList[j]);
        }
    }
//    threadMasterList.push_back(masterStringList1);
//    threadMasterList.push_back(masterStringList2);
//    threadMasterList.push_back(masterStringList3);

    vector<ESortType>  threadSortType;
    threadSortType.push_back(ESortType::AlphabeticalAscending);
    threadSortType.push_back(ESortType::AlphabeticalDescending);
    threadSortType.push_back(ESortType::LastLetterAscending);


    for (unsigned int i = 0; i < 3; ++i) {
//            workerThreads[i] = thread(ThreadedReadFile, threadMasterList[i],threadSortType[i]);
        workerThreads[i] = thread(ThreadedReadFile, masterStringList1,threadSortType[i]);
//            workerThreads[i].join();
    }


    for (int i = 0; i< 3; i++)
    {
        workerThreads[i].join();
    }

}

void test()
{
    std::cout<<1<<std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// File Processing
////////////////////////////////////////////////////////////////////////////////////////////////////
vector<string> ReadFile(string _fileName) {
    vector<string> listOut;
    streampos positionInFile = 0;
    bool endOfFile = false;
    while (!endOfFile) {
        ifstream fileIn(_fileName, ifstream::in);
        fileIn.seekg(positionInFile, ios::beg);

        string* tempString = new string();
        getline(fileIn, *tempString);
        positionInFile = endOfFile ? ios::beg : fileIn.tellg();
        if (!endOfFile)
            listOut.push_back(*tempString);
        endOfFile = fileIn.peek() == EOF;
        fileIn.close();
    }
    return listOut;
}

//void ThreadedReadFile(string _fileName, vector<string>* _listOut) {
//    *_listOut = ReadFile(_fileName);
//}

void ThreadedReadFile(vector<string> _listToSort, ESortType _sortType) {

    string outputName;
    switch(_sortType) {
        case ESortType::AlphabeticalAscending:
            outputName = "MultiAscending";
            break;
        case ESortType::AlphabeticalDescending:
            outputName = "MultiDescending";
            break;
        case ESortType::LastLetterAscending:
            outputName = "MultiLastLetter";
            break;
        default:
            break;
    }
    clock_t startTime = clock()/1000;
//    std::cout<<outputName<<" start "<<startTime<<std::endl;
    vector<string> masterStringList;
    masterStringList = MergeSort(_listToSort, _sortType);
    clock_t endTime = clock()/1000;
//    std::cout<<outputName<<" end "<<endTime<<std::endl;
//        mtx.lock();
    WriteAndPrintResults(masterStringList, outputName, endTime - startTime);
//    mtx.unlock();

}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Sorting
////////////////////////////////////////////////////////////////////////////////////////////////////
bool AlphabeticalAscendingStringComparer::IsFirstAboveSecond(string _first, string _second) {
    unsigned int i = 0;
    while (i < _first.length() && i < _second.length()) {
        if (_first[i] < _second[i])
            return true;
        else if (_first[i] > _second[i])
            return false;
        ++i;
    }
    return (i == _first.length());
}

bool AlphabeticalDescendingStringComparer::IsFirstAboveSecond(string _first, string _second) {
    unsigned int i = 0;
    while (i < _first.length() && i < _second.length()) {
        if (_first[i] >_second[i])
            return true;
        else if (_first[i] < _second[i])
            return false;
        ++i;
    }
    return (i == _second.length());
}

bool LastLetterAscendingStringComparer::IsFirstAboveSecond(string _first, string _second) {
    int i = (isalpha(_first.back()))  ? _first.length() - 1 : _first.length() - 2;
    int j = (isalpha(_second.back())) ? _second.length() - 1 : _second.length() - 2;
    while (i >= 0 && j >= 0) {
        if (_first[i] < _second[j])
            return true;
        else if (_first[i] > _second[j])
            return false;
        --i;
        --j;
    }
    return i == -1 && j >= 0;
}

vector<string> MergeSort(vector<string>& _listToSort, ESortType _sortType) {
    IStringComparer* stringSorter = nullptr;
    switch(_sortType) {
        case ESortType::AlphabeticalAscending:
            stringSorter = new AlphabeticalAscendingStringComparer();
            break;
        case ESortType::AlphabeticalDescending:
            stringSorter = new AlphabeticalDescendingStringComparer();
            break;
        case ESortType::LastLetterAscending:
            stringSorter = new LastLetterAscendingStringComparer();
            break;
        default:
            break;
    }
    int len = _listToSort.size();

    vector<string> sortedList = _listToSort;
    int left_min;
    vector<string> temp(len);

    for(int step = 1; step <= len - 1; step = 2 * step)
    {
        for( left_min = 0; left_min < len - 1; left_min += step * 2)
        {
            int right_max = min(left_min + step * 2 - 1, len - 1);
            int mid = min(left_min + step - 1, len - 1);

            int i, j, k;
            int n1 = mid - left_min + 1;
            int n2 = right_max - mid;

            vector<string> left(n1), right(n2);
            for(i = 0; i < n1; ++i)
                left[i] = sortedList[left_min + i];
            for(j = 0; j < n2; ++j)
                right[j] = sortedList[mid + 1 + j];

            i = 0;
            j = 0;
            k = left_min;
            while(i < n1 && j < n2)
            {
                if(stringSorter->IsFirstAboveSecond(left[i], right[j]))
                {
                    sortedList[k] = left[i];
                    i++;
                }
                else
                {
                    sortedList[k] = right[j];
                    j++;
                }
                k++;
            }
            while(i < n1)
            {
                sortedList[k] = left[i];
                i++;
                k++;
            }

            while(j < n2)
            {
                sortedList[k] = right[j];
                j++;
                k++;
            }
        }
    }
    return sortedList;
}

vector<string> BubbleSort(vector<string>& _listToSort, ESortType _sortType) {
    IStringComparer* stringSorter = nullptr;
    switch(_sortType) {
        case ESortType::AlphabeticalAscending:
            stringSorter = new AlphabeticalAscendingStringComparer();
            break;
        case ESortType::AlphabeticalDescending:
            stringSorter = new AlphabeticalDescendingStringComparer();
            break;
        case ESortType::LastLetterAscending:
            stringSorter = new LastLetterAscendingStringComparer();
            break;
        default:
            break;
    }

    vector<string> sortedList = _listToSort;
    for (unsigned int i = 0; i < sortedList.size() - 1; ++i) {
        for (unsigned int j = 0; j < sortedList.size() - 1; ++j) {
            if (!stringSorter->IsFirstAboveSecond(sortedList[j], sortedList[j+1])) {
                string tempString = sortedList[j];
                sortedList[j] = sortedList[j+1];
                sortedList[j+1] = tempString;
            }
        }
    }
    return sortedList;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Output
////////////////////////////////////////////////////////////////////////////////////////////////////
void WriteAndPrintResults(const vector<string>& _masterStringList, string _outputName, int _clocksTaken) {
    cout << endl << _outputName << "\t- Clocks Taken: " << _clocksTaken << endl;

    ofstream fileOut(_outputName + ".txt", ofstream::trunc);
    for (unsigned int i = 0; i < _masterStringList.size(); ++i) {
        fileOut << _masterStringList[i] << endl;
    }
    fileOut.close();
}
