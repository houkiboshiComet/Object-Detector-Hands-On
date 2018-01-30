// CV_DNN_Application.cpp : アプリケーションのエントリ ポイントを定義します。
//
#include "Paths.h"
#include <iostream>
#include <fstream>
#include <string>

#include "UserCommunicator.h"
#include "opencv2/dnn.hpp"
#include "opencv2/dnn/shape_utils.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"


using namespace OpenCVApp;

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

	double maxProb = -0.1;
	int maxIndex = -1;

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
		std::string fileName = UserCommunicator::askForFilename();
		cv::VideoCapture cap;

		if (fileName.empty()) {
			cap = cv::VideoCapture(0);
		}
		else {
			cap = cv::VideoCapture(fileName);
		}
		cv::Mat image;
		cv::dnn::Net net = cv::dnn::readNetFromDarknet(Paths::MODEL_DIR + "\\tiny-yolo.cfg", Paths::MODEL_DIR + "\\tiny-yolo.weights");

		std::vector<std::string> classNames = readClassNames(Paths::MODEL_DIR + "\\coco.names");


		do {
			//int current = cap.get(cv::CAP_PROP_POS_FRAMES);
			//cap.set(cv::CAP_PROP_POS_FRAMES, current + 5);
			cap >> image;
			cv::Mat mat = cv::dnn::blobFromImage(image, 1.0 / 255.0, cv::Size(416, 416), cv::Scalar(), true, false);
			net.setInput(mat, "data");

			cv::Mat result = net.forward();

			for (int row = 0; row < result.rows; row++) {
				double prob;
				int classIndex = getMaxClassIndex(result, row, 5, &prob);
				if (prob > 0.01) {
					float centerX = result.at<float>(row, 0) * image.cols;
					float centerY = result.at<float>(row, 1) * image.rows;
					int w = static_cast<int>(result.at<float>(row, 2) * image.cols);
					int h = static_cast<int>(result.at<float>(row, 3) * image.rows);

					int x = static_cast<int>(centerX - w / 2);
					int y = static_cast<int>(centerY - h / 2);

					std::string name = classNames[classIndex];
					cv::putText(image, name, cv::Point(x, y), 0, 1, 5);
					cv::rectangle(image, cv::Rect(x, y, w, h), cv::Scalar(255, 255, cv::FILLED));
				}
			}
			cv::imshow("result", image);
			if (cv::waitKey(1) >= 0) { break; }
		} while (!image.empty());
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
