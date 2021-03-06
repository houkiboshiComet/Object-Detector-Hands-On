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
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"

/* "Object-Detector-Hands-On\model\\" を相対パスとして指す定数 */
static const std::string MODEL_DIR = "..\\model\\";

/* フレームレート低下時に動画を間引く際のskip数 */
static const int FRAME_SKIP = 3;

/**
 * @brief Open Fileダイアログを開き、動画ファイルを選択する。
 */
static std::string askForFilename() {
	OPENFILENAMEA ofn;
	char nameBuff[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFilter = "動画ファイル(*.avi,*.mp4,*.wmv,*.mov)\0*.avi;*.mp4;*.wmv;*.mov\0";
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

/**
* @brief クラスラベルを読み取り、クラス名のvectorを作成する。
*/
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


/**
 * @brief	引数のimageが、maxWidth, maxHeightで指定されたサイズに収まらない場合に、
 *			縦横比を維持しながら縮小する。
 */
static void shrinkImageAsNecessary(cv::Mat& image, int maxWidth, int maxHeight) {
	double widthRatio = std::min((double)maxWidth / (double)image.cols, 1.0);
	double heightRatio = std::min((double)maxHeight / (double)image.rows, 1.0);

	double ratio = std::min(widthRatio, heightRatio);

	if (ratio < 1.0) {
		cv::resize(image, image,
			cv::Point(cvRound(image.cols * ratio), cvRound(image.rows * ratio)), cv::INTER_CUBIC);
	}
}

/**
 * @brief 引数をseedにし、テーブルからランダムに色(Scalar)オブジェクトを返す。
 */
static cv::Scalar getRandomColor(int seed) {

	static const std::vector< cv::Scalar > colors = {
		cv::Scalar(255, 0, 0),
		cv::Scalar(0, 255, 0),
		cv::Scalar(0, 0, 255),
		cv::Scalar(255, 255, 0),
		cv::Scalar(0, 255, 255),
		cv::Scalar(255, 0, 255),
		cv::Scalar(255, 100, 100),
		cv::Scalar(100, 255, 100),
		cv::Scalar(100, 100, 255)
	};
	int index = seed % colors.size();

	return colors[index];
}



/**
 * @brief 分類された最も当てはまるクラスのindexを返す。分類に失敗している場合は負の値を返す。
 */
static int getMaxClassIndex(const cv::Mat& outputLayer, int row, double* targetProb) {

	double maxProb = 0.0;
	int maxIndex = -1;
	static const int CLASS_INITIAL_COL = 5;

	for (int i = CLASS_INITIAL_COL; i < outputLayer.cols; i++) {
		float prob = outputLayer.at<float>(row, i);

		if (prob > maxProb) {
			maxProb = prob;
			maxIndex = i - CLASS_INITIAL_COL;
		}
	}
	if (targetProb != nullptr) {
		*targetProb = maxProb;
	}

	return maxIndex;
}



/**
 * @brief 動画1フレーム毎に物体検出に必要な操作を行う。
 * @param net 物体検出の学習済モデルを読み込んだネットオブジェクト
 * @param image 動画上の1フレーム画像
 * @param names 分類されるクラスの名称
 */
void tick(cv::dnn::Net& net, cv::Mat& image, std::vector<std::string> classNames) {
	cv::Mat inputLayer = cv::dnn::blobFromImage(image, 1.0 / 255.0, cv::Size(416, 416), cv::Scalar(0, 0, 0), true, false);
	net.setInput(inputLayer, "data");

	cv::Mat outputLayer = net.forward("detection_out");

	for (int row = 0; row < outputLayer.rows; row++) {
		double boxProb = outputLayer.at<float>(row, 4);
		if (boxProb <= 0.4) { /**< box内に物体が存在する確率が、40％以上の時に処理を続行 */
			continue;
		}
		double classProb;
		int classIndex = getMaxClassIndex(outputLayer, row, &classProb);
		if (classIndex >= 0 && classProb > 0.4) {  /**< クラス確率が、40％以上の時に処理を続行 */

			float centerX = outputLayer.at<float>(row, 0) * image.cols;
			float centerY = outputLayer.at<float>(row, 1) * image.rows;

			float w = outputLayer.at<float>(row, 2) * image.cols;
			float h = outputLayer.at<float>(row, 3) * image.rows;
			int startX = cvRound(centerX - w / 2.0);
			int startY = cvRound(centerY - h / 2.0);
			int endX = cvRound(centerX + w / 2.0);
			int endY = cvRound(centerY + h / 2.0);

			std::string name = classNames[classIndex];
			name += " " + std::to_string(cvRound(100 * classProb)) + "%";
			cv::Scalar classColor = getRandomColor(classIndex);

			cv::putText(image, name, cv::Point(startX, startY), cv::FONT_HERSHEY_SIMPLEX, 1.2, classColor, 2, CV_AA);
			cv::rectangle(image, cv::Point(startX, startY), cv::Point(endX, endY), classColor);
		}
	}
}

int main(int argc, char **argv)
{

	try {
		std::string fileName = askForFilename();
		cv::VideoCapture cap;

		bool useMovieFile = !fileName.empty();
		if (useMovieFile) { /**< 動画ファイルを用いる */
			cap = cv::VideoCapture(fileName);
		}
		else { /**< 備え付けのwebカメラを用いる */
			cap = cv::VideoCapture(0);
		}
		/* クラスラベルの読み込み */
		std::vector<std::string> classNames = readClassNames(MODEL_DIR + "\\coco.names");

		/* 1. ニューラルネットワークを形成する。 */
		cv::dnn::Net net = cv::dnn::readNetFromDarknet(MODEL_DIR + "\\yolo.cfg", MODEL_DIR + "\\yolo.weights");

		cv::Mat frame; /**< 動画中の1フレーム */

		for (;;) {
			/* フレームレート低下のため、動画のフレームをまびく処理 */
			if (useMovieFile && FRAME_SKIP > 0) {
				int current = static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES));
				cap.set(cv::CAP_PROP_POS_FRAMES, current + FRAME_SKIP);
			}
			cap >> frame;
			if (frame.empty()) { break; }

			/* 1366x788のサイズに収まらない場合に、frameを縮小 */
			shrinkImageAsNecessary(frame, 1366, 768);

			/* 1フレーム毎に行う処理 */
			tick(net, frame, classNames);

			cv::imshow("Object Detector", frame);
			if (cv::waitKey(1) >= 0) { break; }
		}
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
	return 0;
}
