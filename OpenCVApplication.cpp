// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <opencv2/core/utils/logger.hpp>
#include <windows.h>
#include <iostream>
wchar_t* projectPath;

struct ImageData {
	std::string path;
	int label;
};

bool isImage(const char* filename) {
	Mat img = imread(filename, IMREAD_UNCHANGED);
	return !img.empty();
}

void testNumberImages(std::vector<ImageData> trainImages, std::vector<ImageData> testImages) {
	printf("Test numarare imagini!\n");
	if (trainImages.size() + testImages.size() == 6862) {
		printf("Test reusit!!\n");
	}
	else {
		printf("Test picat!!\n");
	}
}


int extractLabel(std::string& path) {
	size_t lastApp = path.find_last_of("\\/");
	std::string aux = path.substr(lastApp + 1);
	if (aux == "dew") {
		return 0;
	}
	else if (aux == "fogsmog") {
		return 1;
	}
	else if (aux == "frost") {
		return 2;
	}
	else if (aux == "glaze") {
		return 3;
	}
	else if (aux == "hail") {
		return 4;
	}
	else if (aux == "lightning") {
		return 5;
	}
	else if (aux == "rain") {
		return 6;
	}
	else if (aux == "rainbow") {
		return 7;
	}
	else if (aux == "rime") {
		return 8;
	}
	else if (aux == "sandstorm") {
		return 9;
	}
	else if (aux == "snow") {
		return 10;
	}
	return 0;
}

void traverseFolder(const char* folderPath, std::vector<ImageData>& trainImages, std::vector<ImageData>& testImages, bool& ok) {
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;

	
	char searchPath[MAX_PATH];
	snprintf(searchPath, MAX_PATH, "%s\\*", folderPath);

	
	hFind = FindFirstFile(searchPath, &findFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		printf("Nu s-au găsit fișiere în directorul specificat.\n");
		return;
	}

	do {
	
		if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
			
			char filePath[MAX_PATH];
			snprintf(filePath, MAX_PATH, "%s\\%s", folderPath, findFileData.cFileName);

			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { 
				
				traverseFolder(filePath, trainImages, testImages, ok);
			}
			else { 
					ImageData imageData;
					imageData.path = std::string(filePath);
					imageData.label = extractLabel(std::string(folderPath));
					if (ok) {
						testImages.push_back(imageData);
						ok = false;
					}
					else {
						trainImages.push_back(imageData);
						ok = true;
					}
			}
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
}

int main() {
	const char* rootFolderPath = NULL;
	char username[100]; // Spațiul pentru stocarea numelui utilizatorului
	DWORD username_len = 100; // Lungimea buffer-ului
	
	std::vector<ImageData> trainImages;
	std::vector<ImageData> testImages;
	bool ok = false;

	if (GetUserName(username, &username_len)) {
		if (strcmp(username, "Cristi") == 0) {
			rootFolderPath = "C:\\Users\\Cristian\\Desktop\\Weather_Recognition_Proj\\dataset";
		}
		else if (strcmp(username, "ioanf") == 0) {
			rootFolderPath = "C:\\Users\\ioanf\\Desktop\\Facultate An 3 SEM 2\\PI\\Proiect\\dataset";
		}
		else if(strcmp(username, "HP") == 0) {
			rootFolderPath = "D:\\OpenCVApplication-VS2022_OCV490_basic\\dataset";
		}
	}
	else {
		std::cerr << "Couldn't obtain username!!" << std::endl;
	}
	traverseFolder(rootFolderPath, trainImages, testImages, ok);
	testNumberImages(trainImages, testImages);
	printf("%d, %d\n", trainImages.size(), testImages.size());
	for (ImageData imageData : testImages) {
		printf("path: %s\neticheta: %d\n", imageData.path.c_str(), imageData.label);
	}
	return 0;
}
