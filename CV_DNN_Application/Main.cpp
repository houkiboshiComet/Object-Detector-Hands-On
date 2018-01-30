// CV_DNN_Application.cpp : アプリケーションのエントリ ポイントを定義します。
//
#include "Paths.h"
#include <iostream>

#include "UserCommunicator.h"
#include "opencv2/dnn.hpp"
#include "opencv2/dnn/shape_utils.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace OpenCVApp;

int main(int argc, char **argv)
{

	try {
		std::string fileName = UserCommunicator::askForFilename();
		
		cv::VideoCapture cap = cv::VideoCapture(fileName);

		cv::Mat image;
		cv::dnn::Net net = cv::dnn::readNetFromDarknet(Paths::MODEL_DIR + "\\tiny-yolo.cfg", Paths::MODEL_DIR + "\\tiny-yolo.weights");
		
		
		while (true) {
			int current = cap.get(cv::CAP_PROP_POS_FRAMES);
			cap.set(cv::CAP_PROP_POS_FRAMES, current + 5);
			cap >> image;
			cv::Mat mat = cv::dnn::blobFromImage(image, 1.0 / 255.0, cv::Size(608, 608), cv::Scalar(), true, false);
			net.setInput(mat, "data");
			cv::Mat result = net.forward();
			for (int i = 0; i < result.rows; i++) {
				if (result.at<float>(i, 4) > 0.5) {
					/*
					std::cout << i << std::endl;
					std::cout << result.at<float>(i, 4) << std::endl;
					std::cout << result.at<float>(i, 0) << std::endl;
					std::cout << result.at<float>(i, 1) << std::endl;
					std::cout << result.at<float>(i, 2) << std::endl;
					std::cout << result.at<float>(i, 3) << std::endl;

					std::cout << std::endl;
					*/
					float x = result.at<float>(i, 0) * image.cols;
					float y = result.at<float>(i, 1) * image.rows;
					float w = result.at<float>(i, 2) * image.cols;
					float h = result.at<float>(i, 3) * image.rows;
					
					cv::rectangle(image, cv::Rect(x - w / 2, y - h / 2, w, h), cv::Scalar(255, 255, cv::FILLED));
					
				}
			}
			cv::imshow("result", image);
			cv::waitKey(1);
		}
		cv::imwrite("result.bmp", image);
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
