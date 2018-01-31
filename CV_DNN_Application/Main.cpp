// CV_DNN_Application.cpp : アプリケーションのエントリ ポイントを定義します。
//
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <windows.h>
#include <math.h>

#include "opencv2/dnn.hpp"
#include "opencv2/dnn/shape_utils.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

static const std::string MODEL_DIR = "..\\model\\";
static const int X_INDEX = 0;
static const int Y_INDEX = 1;
static const int WIDTH_INDEX = 2;
static const int HEIGHT_INDEX = 3;
static const int CONFIDENCE_INDEX = 4;
static const int CLASSES_INITAIL_INDEX = 5;
static const int FRAME_INTERVAL = 5;


static std::string askForFilename() {
	OPENFILENAMEA ofn;
	char nameBuff[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFilter = "動画ファイル(*.avi,*.mp4,*.wmv)\0*.avi;*.mp4;*.wmv\0";
	ofn.lpstrFile = nameBuff;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetOpenFileNameA(&ofn)) {
		return std::string(nameBuff);
	}
	else {
		return "";
	}
}

static cv::Scalar getRandomColor(int seed, int brightness) {

	double r = brightness;
	double g = brightness;
	double b = brightness;

	srand(seed * 1000);
	int hue = rand() % 360;
	
	double ratio = hue / 360.0;
	switch (hue / 60)
	{
	case 0:
		g *= ratio;
		b *= 0;
		break;
	case 1:
		r *= 1.0 - ratio;
		b *= 0;
		break;
	case 2:
		r *= 0;
		b *= ratio;
		break;
	case 3:
		r *= 0;
		g *= 1.0 - ratio;
		break;
	case 4:
		r *= ratio;
		g *= 0;
		break;
	case 5:
	default:
		g *= 0;
		b *= 1 - ratio;
		break;
	}
	return cv::Scalar(b, g, r);
}

static std::vector<std::string> readClassNames(const std::string& labelname) {

	std::vector<std::string> classNames;
	std::ifstream fp(labelname);

	if (!fp.is_open()) {
		throw std::ios_base::failure("file can't be open.");
	}
	std::string name;

	while (!fp.eof()) {
		std::getline(fp, name);
		if (name.length() > 0) {
			classNames.push_back(name);
		}
	}
	fp.close();
	return classNames;
}

static int getMaxClassIndex(const cv::Mat& outputLayer, int row, int offset, double* targetProb) {

	double maxProb = 0.0;
	int maxIndex = 0;

	for (int i = offset; i < outputLayer.cols; i++) {
		float prob = outputLayer.at<float>(row, i);

		if (prob > maxProb) {
			maxProb = prob;
			maxIndex = i - offset;
		}
	}
	if (targetProb != nullptr) {
		*targetProb = maxProb;
	}
	return maxIndex;
}

int main(int argc, char **argv)
{

	try {
		std::string fileName = askForFilename();
		cv::VideoCapture cap;

		bool useMovieFile = !fileName.empty();

		if (useMovieFile) {
			cap = cv::VideoCapture(fileName);
		}
		else { //< useVideoCamera
			cap = cv::VideoCapture(0);
		}
		cv::Mat image;
		cv::dnn::Net net = cv::dnn::readNetFromDarknet(MODEL_DIR + "\\yolo.cfg", MODEL_DIR + "\\yolo.weights");

		std::vector<std::string> classNames = readClassNames(MODEL_DIR + "\\coco.names");

		while(true) {

			if (useMovieFile && FRAME_INTERVAL > 0) {
				int current = static_cast<int>( cap.get(cv::CAP_PROP_POS_FRAMES) );
				cap.set(cv::CAP_PROP_POS_FRAMES, current + FRAME_INTERVAL);
			}

			cap >> image;
			if (image.empty()) { break; }

			cv::Mat inputLayer = cv::dnn::blobFromImage(image, 1.0 / 255.0, cv::Size(320, 320), cv::Scalar(), true, false);
			net.setInput(inputLayer, "data");
			
			cv::Mat outputLayer = net.forward();

			for (int row = 0; row < outputLayer.rows; row++) {
				double prob;
				int classIndex = getMaxClassIndex(outputLayer, row, CLASSES_INITAIL_INDEX, &prob);
				if (prob > 0.5) {
					float centerX = outputLayer.at<float>(row, X_INDEX) * image.cols;
					float centerY = outputLayer.at<float>(row, Y_INDEX) * image.rows;
					
					int w = static_cast<int>(outputLayer.at<float>(row, WIDTH_INDEX) * image.cols);
					int h = static_cast<int>(outputLayer.at<float>(row, HEIGHT_INDEX) * image.rows);
					int x = static_cast<int>(centerX - w / 2.0);
					int y = static_cast<int>(centerY - h / 2.0);

					std::string name = classNames[classIndex];
					
					cv::Scalar classColor = getRandomColor(classIndex, 255);

					cv::putText(image, name, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 1.2, classColor, 2, CV_AA);
					cv::rectangle(image, cv::Rect(x, y, w, h), classColor);
				}
			}
			cv::imshow("Object Detector", image);
			if (cv::waitKey(1) >= 0) { break; }
		};
	} 
	catch (cv::Exception& e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
	return 0;
}
