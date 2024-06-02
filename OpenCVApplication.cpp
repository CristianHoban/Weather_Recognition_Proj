#include "stdafx.h"
#include "common.h"
#include <opencv2/core/utils/logger.hpp>
#include <windows.h>
#include <iostream>
#include <cfloat> // Pentru DBL_MAX;

wchar_t* projectPath;

struct ImageData {
	Mat img;
	std::string path;
	int label;
};

struct MediiColori {
	Scalar medieRGB;
	Scalar medieHSV;
};

bool isImage(const char* filename) {
	Mat img = imread(filename, IMREAD_UNCHANGED);
	return !img.empty();
}

int ok = 1;

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
	printf("Accuracy: %.2lf%%\n", accuracy(testImages, randomLabel) * 100);
}

std::vector<MediiColori> calculMedieCuloriPerClasa(std::vector<ImageData>& images) {
	const int numarClase = 11; // Avem 11 clase (tipuri de imagini)
	std::vector<MediiColori> medii(numarClase, MediiColori()); // Vector pentru a stoca sumele medii pentru fiecare clasă
	std::vector<int> numarImaginiPerClasa(numarClase, 0); // Vector pentru a stoca numărul de imagini pentru fiecare clasă

	for (const auto& imageData : images) {
		// Verificăm dacă imaginea a fost încărcată corect
		if (!imageData.img.empty()) {
			// Calculăm media culorilor pentru imagine în format RGB
			Scalar mediaRGB = mean(imageData.img);
			// Convertim imaginea la spațiul de culoare HSV
			Mat hsv;
			cvtColor(imageData.img, hsv, COLOR_BGR2HSV);
			// Calculăm media culorilor pentru imagine în format HSV
			Scalar mediaHSV = mean(hsv);
			// Adăugăm mediile culorilor la structura corespunzătoare pentru clasa
			medii[imageData.label].medieRGB += mediaRGB;
			medii[imageData.label].medieHSV += mediaHSV;
			// Incrementăm numărul de imagini pentru clasa corespunzătoare
			numarImaginiPerClasa[imageData.label]++;
		}
	}

	// Calculăm media mediei culorilor pentru fiecare clasă
	for (int i = 0; i < numarClase; ++i) {
		if (numarImaginiPerClasa[i] > 0) {
			medii[i].medieRGB /= numarImaginiPerClasa[i];
			medii[i].medieHSV /= numarImaginiPerClasa[i];
		}
	}

	return medii;
}


ImageData generateSomethingLabel(ImageData image, const std::vector<MediiColori>& medii, int numar) {
	int newLabel = -1;
	double distanceMin = DBL_MAX;
	ImageData newImageData;
	newImageData.path = image.path;

	if (!image.img.empty()) {
		Scalar media;
		if (numar == 1) {
			// Calculați media culorilor în format RGB
			media = mean(image.img);
		}
		else if (numar == 2) {
			// Calculați media culorilor în format HSV
			Mat hsv;
			cvtColor(image.img, hsv, COLOR_BGR2HSV);
			media = mean(hsv);
		}
		else if (numar == 3) {
			// Calculați media culorilor în format RGB și HSV
			Scalar mediaRGB = mean(image.img);
			Mat hsv;
			cvtColor(image.img, hsv, COLOR_BGR2HSV);
			Scalar mediaHSV = mean(hsv);
			// Combinați cele două medii într-un singur vector
			media = Scalar((mediaRGB[0] + mediaHSV[0]) / 2, (mediaRGB[1] + mediaHSV[1]) / 2, (mediaRGB[2] + mediaHSV[2]) / 2);
		}

		// Comparați media calculată cu mediile date și găsiți cea mai mică distanță
		for (int i = 0; i < medii.size(); ++i) {
			double distanta;
			double distanta1;
			double distanta2;
			if (numar == 1) {
				distanta = norm(media, medii[i].medieRGB, NORM_L2);
			}
			else if (numar == 2) {
				distanta = norm(media, medii[i].medieHSV, NORM_L2);
			}
			else{
				Scalar media2 = Scalar((medii[i].medieRGB[0] + medii[i].medieHSV[0]) / 2, (medii[i].medieRGB[1] + medii[i].medieHSV[1]) / 2, (medii[i].medieRGB[2] + medii[i].medieHSV[2]) / 2);
				distanta = norm(media, media2, NORM_L2);
			}
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

void accPerClass(std::vector<ImageData> images, std::vector<MediiColori> medii, int caz) {
	const int size = 11;
	int matrix[size][size];
	std::vector<ImageData> newVector;
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			matrix[i][j] = 0;
		}
	}
	for (ImageData imageData : images) {
		ImageData newImageData = generateSomethingLabel(imageData, medii, caz);
		newVector.push_back(newImageData);
		int x = imageData.label;
		int y = newImageData.label;
		matrix[x][y]++;
	}
	double d = accuracy(images, newVector);
	printf("Accuracy: %.2lf%%\n", d * 100);

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

void fftshift(Mat& magI) {
	// Schimbam quadrantele astfel incat frecventele joase sa fie in centru
	int cx = magI.cols / 2;
	int cy = magI.rows / 2;

	Mat q0(magI, Rect(0, 0, cx, cy));
	Mat q1(magI, Rect(cx, 0, cx, cy));
	Mat q2(magI, Rect(0, cy, cx, cy));
	Mat q3(magI, Rect(cx, cy, cx, cy));

	Mat tmp;
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);
}

// Functie pentru calculul transformatei Fourier
std::vector<double> computeFourierDescriptors(Mat img, int numDescriptors) {
	std::vector<double> descriptors(numDescriptors, 0.0);
	Mat gray, padded;

	// Convertim imaginea in grayscale
	cvtColor(img, gray, COLOR_BGR2GRAY);

	// Gasim cel mai apropiat numar putere a lui 2 pentru dimensiuni
	int m = getOptimalDFTSize(gray.rows);
	int n = getOptimalDFTSize(gray.cols);

	// "Pad" imaginea
	copyMakeBorder(gray, padded, 0, m - gray.rows, 0, n - gray.cols, BORDER_CONSTANT, Scalar::all(0));

	// Convertim imaginea la tipul CV_32F
	padded.convertTo(padded, CV_32F);

	// Efectuam DFT
	Mat complexI;
	dft(padded, complexI, DFT_COMPLEX_OUTPUT);
	//if (ok < 3) {
	//	ok++;
	//	imshow("image", img);
	//	imshow("fourier", complexI);
	//}

	// Reordonam coeficientii DFT in centru
	fftshift(complexI);

	// Calculam magnitudinea si faza
	Mat magI;
	Mat planes[] = { Mat::zeros(complexI.size(), CV_32F), Mat::zeros(complexI.size(), CV_32F) };
	split(complexI, planes);
	magnitude(planes[0], planes[1], magI);

	// Adaugam coeficientii DFT in vectorul descriptors
	for (int i = 0; i < numDescriptors && i < magI.total(); i++) {
		descriptors[i] = magI.at<float>(i);
	}

	return descriptors;
}

ImageData generateFourierLabel(ImageData image, std::vector<std::vector<double>>& classDescriptors, int numDescriptors) {
	int newLabel = -1;
	double distanceMin = DBL_MAX;
	ImageData newImageData;
	newImageData.path = image.path;

	if (!image.img.empty()) {
		std::vector<double> descriptors = computeFourierDescriptors(image.img, numDescriptors);

		for (int i = 0; i < classDescriptors.size(); ++i) {
			double distance = 0.0;
			for (int j = 0; j < numDescriptors; ++j) {
				distance += pow(descriptors[j] - classDescriptors[i][j], 2);
			}
			distance = sqrt(distance);

			if (distance < distanceMin) {
				distanceMin = distance;
				newLabel = i;
			}
		}
	}

	newImageData.label = newLabel;
	return newImageData;
}

void accFourierClass(std::vector<ImageData> images, std::vector<std::vector<double>>& classDescriptors, int numDescriptors) {
	const int size = 11;
	int matrix[size][size];
	std::vector<ImageData> newVector;
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			matrix[i][j] = 0;
		}
	}
	for (ImageData imageData : images) {
		ImageData newImageData = generateFourierLabel(imageData, classDescriptors, numDescriptors);
		newVector.push_back(newImageData);
		int x = imageData.label;
		int y = newImageData.label;
		matrix[x][y]++;
	}
	double d = accuracy(images, newVector);
	printf("Accuracy: %.2lf%%\n", d * 100);

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

std::vector<std::vector<double>> calculateClassDescriptors(std::vector<ImageData>& images, int numDescriptors) {
	const int numarClase = 11;
	std::vector<std::vector<double>> classDescriptors(numarClase, std::vector<double>(numDescriptors, 0.0));
	std::vector<int> numarImaginiPerClasa(numarClase, 0);

	for (const auto& imageData : images) {
		if (!imageData.img.empty()) {
			std::vector<double> descriptors = computeFourierDescriptors(imageData.img, numDescriptors);

			for (int i = 0; i < numDescriptors; ++i) {
				classDescriptors[imageData.label][i] += descriptors[i];
			}
			numarImaginiPerClasa[imageData.label]++;
		}
	}

	for (int i = 0; i < numarClase; ++i) {
		if (numarImaginiPerClasa[i] > 0) {
			for (int j = 0; j < numDescriptors; ++j) {
				classDescriptors[i][j] /= numarImaginiPerClasa[i];
			}
		}
	}

	return classDescriptors;
}

/////////////////////////////////////////////////////////////////////////////////////
// Functie pentru a calcula histograma de culori pentru o imagine
std::vector<double> calculateColorHistogram(const Mat& img, int histSize = 256) {
	std::vector<double> histogram(histSize * 3, 0.0); // 3 canale (RGB)
	std::vector<Mat> bgr_planes;
	split(img, bgr_planes);

	// Parametrii pentru histograma
	float range[] = { 0, 256 };
	const float* histRange = { range };
	bool uniform = true, accumulate = false;

	Mat b_hist, g_hist, r_hist;

	// Calculam histogramele pentru fiecare canal
	calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

	// Copiem histogramele in vectorul histogram
	for (int i = 0; i < histSize; ++i) {
		histogram[i] = b_hist.at<float>(i);
		histogram[i + histSize] = g_hist.at<float>(i);
		histogram[i + 2 * histSize] = r_hist.at<float>(i);
	}

	return histogram;
}

ImageData generateHistogramLabel(ImageData image, std::vector<std::vector<double>>& classHistograms, int histSize) {
	int newLabel = -1;
	double distanceMin = DBL_MAX;
	ImageData newImageData;
	newImageData.path = image.path;

	if (!image.img.empty()) {
		std::vector<double> histogram = calculateColorHistogram(image.img, histSize);

		for (int i = 0; i < classHistograms.size(); ++i) {
			double distance = 0.0;
			for (int j = 0; j < histSize * 3; ++j) {
				distance += pow(histogram[j] - classHistograms[i][j], 2);
			}
			distance = sqrt(distance);

			if (distance < distanceMin) {
				distanceMin = distance;
				newLabel = i;
			}
		}
	}

	newImageData.label = newLabel;
	return newImageData;
}

std::vector<std::vector<double>> calculateClassHistograms(std::vector<ImageData>& images, int histSize) {
	const int numarClase = 11;
	std::vector<std::vector<double>> classHistograms(numarClase, std::vector<double>(histSize * 3, 0.0));
	std::vector<int> numarImaginiPerClasa(numarClase, 0);

	for (const auto& imageData : images) {
		if (!imageData.img.empty()) {
			std::vector<double> histogram = calculateColorHistogram(imageData.img, histSize);

			for (int i = 0; i < histSize * 3; ++i) {
				classHistograms[imageData.label][i] += histogram[i];
			}
			numarImaginiPerClasa[imageData.label]++;
		}
	}

	for (int i = 0; i < numarClase; ++i) {
		if (numarImaginiPerClasa[i] > 0) {
			for (int j = 0; j < histSize * 3; ++j) {
				classHistograms[i][j] /= numarImaginiPerClasa[i];
			}
		}
	}

	return classHistograms;
}

void accHistogramClass(std::vector<ImageData> images, std::vector<std::vector<double>>& classHistograms, int histSize) {
	const int size = 11;
	int matrix[size][size];
	std::vector<ImageData> newVector;
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			matrix[i][j] = 0;
		}
	}
	for (ImageData imageData : images) {
		ImageData newImageData = generateHistogramLabel(imageData, classHistograms, histSize);
		newVector.push_back(newImageData);
		int x = imageData.label;
		int y = newImageData.label;
		matrix[x][y]++;
	}
	double d = accuracy(images, newVector);
	printf("Accuracy: %.2lf%%\n", d * 100);

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

	for (int linie = 0; linie < 11; ++linie) {
		printf("\n|%10s |", numeCategorie(linie));
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

///////////////////////////////////////////////////////////////

void accFinal(std::vector<ImageData> images, std::vector<MediiColori> medii, std::vector<std::vector<double>> classDescriptors, std::vector<std::vector<double>>& classHistograms) {
	const int size = 11;
	int matrix[size][size];
	std::vector<ImageData> newVector;
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			matrix[i][j] = 0;
		}
	}
	for (ImageData imageData : images) {
		ImageData newImageData;
		switch (imageData.label) {
			case 0:
				newImageData = generateSomethingLabel(imageData, medii, 2);
				break;
			case 1:
				newImageData = generateFourierLabel(imageData, classDescriptors, 4);
				break;
			case 2:
				newImageData = generateHistogramLabel(imageData, classHistograms, 256);
				break;
			case 3:
				newImageData = generateHistogramLabel(imageData, classHistograms, 256);
				break;
			case 4:
				newImageData = generateSomethingLabel(imageData, medii, 2);
				break;
			case 5:
				newImageData = generateSomethingLabel(imageData, medii, 3);
				break;
			case 6:
				newImageData = generateSomethingLabel(imageData, medii, 2);
				break;
			case 7:
				newImageData = generateSomethingLabel(imageData, medii, 2);
				break;
			case 8:
				newImageData = generateSomethingLabel(imageData, medii, 1);
				break;
			case 9:
				newImageData = generateSomethingLabel(imageData, medii, 1);
				break;
			case 10:
				newImageData = generateSomethingLabel(imageData, medii, 1);
				break;
		}
		newVector.push_back(newImageData);
		int x = imageData.label;
		int y = newImageData.label;
		matrix[x][y]++;
	}
	double d = accuracy(images, newVector);
	printf("Accuracy: %.2lf%%\n", d * 100);

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

	for (int linie = 0; linie < 11; ++linie) {
		printf("\n|%10s |", numeCategorie(linie));
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
	char username[100]; 
	DWORD username_len = 100; 

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
	std::vector<MediiColori> medii = calculMedieCuloriPerClasa(trainImages);
	std::vector<std::vector<double>> classDescriptors = calculateClassDescriptors(trainImages, 4);
	std::vector<std::vector<double>> classHistograms = calculateClassHistograms(trainImages, 256);
	int op;
	do
	{
		system("cls");
		destroyAllWindows();
		printf("Menu:\n");
		printf(" 1 - Test number images\n");
		printf(" 2 - Test compare random label\n");
		printf(" 3 - Test compare RGB color label\n");
		printf(" 4 - Test compare Fourier label\n");
		printf(" 5 - Test compare HSV color label\n");
		printf(" 6 - Test compare RGB+HSV color label\n");
		printf(" 7 - Test compare histogram label\n");
		printf(" 8 - Test Final!\n");
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
			accPerClass(testImages, medii, 1);
			break;
		case 4:
		{
			int numDescriptors = 4;
			accFourierClass(testImages, classDescriptors, numDescriptors);
			break;
		}
		case 5:
			accPerClass(testImages, medii, 2);
			break;
		case 6:
			accPerClass(testImages, medii, 3);
			break;
		case 7:
			accHistogramClass(testImages, classHistograms, 256);
			break;
		case 8:
			accFinal(testImages, medii, classDescriptors, classHistograms);
		}
		system("pause");
	} while (op != 0);
	return 0;
}