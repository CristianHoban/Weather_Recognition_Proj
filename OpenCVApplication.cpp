// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <opencv2/core/utils/logger.hpp>
#include <windows.h>
#include <iostream>
#include <cfloat> // Pentru DBL_MAX
wchar_t* projectPath;

struct ImageData {
	Mat img;
	std::string path;
	int label;
};

bool isImage(const char* filename) {
	Mat img = imread(filename, IMREAD_UNCHANGED);
	return !img.empty();
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
				imageData.img = imread(imageData.path);
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

double accuracy(std::vector<ImageData> original, std::vector<ImageData> generate) {
	double ok = 0;
	for (int i = 0; i < generate.size(); i++) {
		if (original[i].label == generate[i].label) {
			ok++;
		}
	}
	return ok / original.size();
}

ImageData generateRandomLabel(ImageData imageData) {
	ImageData newImageData;
	newImageData.path = imageData.path;
	newImageData.label = rand() % 11;
	return newImageData;
}
// Test 1
void testNumberImages(std::vector<ImageData> trainImages, std::vector<ImageData> testImages) {
	printf("Test 1 -> Numarare imagini!\n");
	if (trainImages.size() + testImages.size() == 6862) {
		printf("Test reusit!\n");
	}
	else {
		printf("Test picat!\n");
	}
}

// Test 2
void testCompareLabel(std::vector<ImageData> testImages) {
	printf("Test 2 -> Comparare etichete!\n");
	std::vector<ImageData> randomLabel;
	for (ImageData imageData : testImages) {
		ImageData newImageData = generateRandomLabel(imageData);
		randomLabel.push_back(newImageData);
	}
	printf("Accuracy: %.2lf%%\n", accuracy(testImages, randomLabel)*100);
}

std::vector<Scalar> calculMedieCuloriPerClasa(std::vector<ImageData>& images) {
	const int numarClase = 11; // Avem 11 clase (tipuri de imagini)
	std::vector<Scalar> medii(numarClase, Scalar(0, 0, 0)); // Vector pentru a stoca sumele medii pentru fiecare clasă
	std::vector<int> numarImaginiPerClasa(numarClase, 0); // Vector pentru a stoca numărul de imagini pentru fiecare clasă

	for (const auto& imageData : images) {
		// Verificăm dacă imaginea a fost încărcată corect
		if (!imageData.img.empty()) {
			// Calculăm media culorilor pentru imagine
			Scalar media = mean(imageData.img);
			// Adăugăm media culorilor la suma mediei pentru clasa corespunzătoare
			medii[imageData.label] += media;
			// Incrementăm numărul de imagini pentru clasa corespunzătoare
			numarImaginiPerClasa[imageData.label]++;
		}
	}
	// Afișăm rezultatele pentru fiecare clasă
	for (int i = 0; i < numarClase; ++i) {
		if (numarImaginiPerClasa[i] > 0) {
			// Calculăm media mediei culorilor pentru clasa i
			medii[i] /= numarImaginiPerClasa[i];
		}
	}
	return medii;
}

ImageData generateSomethingLabel(ImageData image, std::vector<Scalar> medii) {
	int newLabel = -1;
	double distanceMin = DBL_MAX;
	ImageData newImageData;
	newImageData.path = image.path;
	if (!image.img.empty()) {
		Scalar media = mean(image.img);
		for (int i = 0; i < medii.size(); ++i) {
			double distanta = norm(media, medii[i], NORM_L2);
			if (distanta < distanceMin) {
				distanceMin = distanta;
				newLabel = i; 
			}
		}
	}
	newImageData.label = newLabel;
	return newImageData;
}

const char* numeCategorie(int indice) {
	switch (indice) {
	case 0: return "dew";
	case 1: return "fogsmog";
	case 2: return "frost";
	case 3: return "glaze";
	case 4: return "hail";
	case 5: return "lightning";
	case 6: return "rain";
	case 7: return "rainbow";
	case 8: return "rime";
	case 9: return "sandstorm";
	case 10: return "snow";
	default: return "";
	}
}

void accPerClass(std::vector<ImageData> images, std::vector<Scalar> medii) {
	const int size = 11;
	int matrix[size][size];
	std::vector<ImageData> newVector;
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			matrix[i][j] = 0;
		}
	}
	for (ImageData imageData : images) {
		ImageData newImageData = generateSomethingLabel(imageData,medii);
		newVector.push_back(newImageData);
		int x = imageData.label;
		int y = newImageData.label;
		matrix[x][y]++;
	}
	double d = accuracy(images, newVector);
	printf("Accuracy: %.2lf%%\n", d*100);

	for (int i = 1; i < 146; i++) {
		printf("_");
	}
	printf("\n|%10s |", " ");
	for (int coloana = 0; coloana < 11; ++coloana) {
		printf("%10s |", numeCategorie(coloana));
	}
	printf("\n");
	for (int i = 1; i < 146; i++) {
		if (i % 12 == 1) {
			printf("|");
		}
		else {
			printf("_");
		}
	}

	// Afisam liniile matricei cu valorile
	for (int linie = 0; linie < 11; ++linie) {
		printf("\n|%10s |", numeCategorie(linie)); // Afisam numele categoriei pentru prima coloană

		// Afisam valorile matricei pentru fiecare coloana
		for (int coloana = 0; coloana < 11; ++coloana) {
			if (linie == coloana) {
				printf(" **%7d |", matrix[linie][coloana]);
			}
			else {
				printf("%10d |", matrix[linie][coloana]);
			}
		}
		printf("\n");
		for (int i = 1; i < 146; i++) {
			if (i % 12 == 1) {
				printf("|");
			}
			else {
				printf("_");
			}
		}
	}
	printf("\n");
}


int main() {
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
	projectPath = _wgetcwd(0, 0);
	srand(time(NULL));
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
		else if (strcmp(username, "HP") == 0) {
			rootFolderPath = "D:\\OpenCVApplication-VS2022_OCV490_basic\\dataset";
		}
	}
	else {
		std::cerr << "Couldn't obtain username!!" << std::endl;
	}
	traverseFolder(rootFolderPath, trainImages, testImages, ok);
	testCompareLabel(testImages);
	int op;
	do
	{
		system("cls");
		destroyAllWindows();
		printf("Menu:\n");
		printf(" 1 - Test number images\n");
		printf(" 2 - Test compare random label\n");
		printf(" 3 - Test compare something label\n");
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d", &op);
		switch (op)
		{
		case 1:
			testNumberImages(trainImages, testImages);
			break;
		case 2:
			testCompareLabel(testImages);
			break;
		case 3:
			std::vector<Scalar> medii = calculMedieCuloriPerClasa(trainImages);
			accPerClass(testImages, medii);
			break;
		}
		system("pause");
	} while (op != 0);
	return 0;
}