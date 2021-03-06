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


/**
 * @brief Open Fileダイアログを開き、画像ファイルを選択する。
 */
static std::string askForFilename() {
	OPENFILENAMEA ofn;
	char nameBuff[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFilter = "動画ファイル(*.png,*.jpg,*.jpeg,*.bmp)\0*.png;*.jpg;*.bmp\0";
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


static void getRankHigherClasses(const cv::Mat& probabilities, std::vector<int> *higherClass, std::vector<float> *higherProbs, int rankCount) {
	cv::Mat higherClassIndexs;
	cv::sortIdx(probabilities, higherClassIndexs, CV_SORT_EVERY_ROW | CV_SORT_DESCENDING);

	for (int i = 0; i < std::min(rankCount, higherClassIndexs.cols); i++) {
		int classIndex = higherClassIndexs.at<int>(0, i);
		higherClass->push_back(classIndex);
		higherProbs->push_back(probabilities.at<float>(0, classIndex));
	}
}


static void showClassifiedResult(cv::Mat& outputLayer, const std::vector<std::string> classNames) {

	for (int row = 0; row < outputLayer.rows; row++) {
		std::vector<int> rankHigerClassIds;
		std::vector<float> rankHigerClassProbs;

		if (outputLayer.at<float>(row, 4) >= 0.4) {

			int showClassCount = 5;

			/* 分類されたクラス確率の上位5位と、そのクラスのインデックスを取得 */
			getRankHigherClasses(cv::Mat(outputLayer, cv::Rect(5, row, outputLayer.cols - 5, 1)), &rankHigerClassIds, &rankHigerClassProbs, showClassCount);

			if (rankHigerClassProbs[0] < 0.4){
				continue;
			}
			std::cout << "Bounding Box " << row << std::endl;
			std::cout << "Best Classified Classes" << std::endl;

			for (int i = 0; i < showClassCount; i++) {
				
				if (rankHigerClassProbs[i] <= 0) {
					break;
				}
				int classId = rankHigerClassIds[i];
				std::cout << "\t" << "Top " << i + 1 << " Class: #" << classId << " '" << classNames.at(classId) << "'" << std::endl;
				std::cout << "\t" << "Probability: " << rankHigerClassProbs[i] * 100 << "%" << std::endl;
			}
			std::cout << std::endl;
		}
	}
}

/* 出力層をcsv形式でファイル化する */
static const void reportOutputLayerAsCsv(const cv::Mat& outputLayer, const std::vector<std::string>& classNames) {

	/* IMPLEMENT ME */
	//std::ofstream csv("outputLayer.csv");

	//csv << "x,y,width,height,probability,";
	for (auto name : classNames) {
		//csv << name << ",";
	}
	//csv << std::endl;

	for (int row = 0; row < outputLayer.rows; row++) {
		for (int col = 0; col < outputLayer.cols; col++) {
			//csv << 行列要素をここで格納
			//csv << ",";
		}
		//csv << std::endl;
	}
	//csv.close();
}

int main(int argc, char **argv)
{
	try {
		/* 1. ニューラルネットワークを形成する。 */
		cv::dnn::Net net = cv::dnn::readNetFromDarknet(MODEL_DIR + "\\yolo.cfg", MODEL_DIR + "\\yolo.weights");

		/* 入力する画像を読み込む。 */
		std::string fileName = askForFilename();
		cv::Mat image = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);
		
		/* 2. 入力層に画像を入力する。 */
		cv::Mat inputLayer = cv::dnn::blobFromImage(
			image,					/** < 入力する画像 */ 
			1.0 / 255.0,			/** < 0 ～ 255の画素の階調を、0.0 ～ 1.0 の階調に補正 */
			cv::Size(416, 416),		/** < 入力層のサイズ */
			cv::Scalar(0, 0, 0),	/** < 画素を差し引いてする補正するための、色情報(0指定のため、補正なし) */
			true,					/** < BGR(OpenCVの画素の並び方)画像を、RGB画像に変換 */
			false					/** < 入力層と縦横比が違う場合にトリムせず拡縮して調整する */	);
		net.setInput(inputLayer, "data");


		/* 3. 出力層まで順伝播させる */
		cv::Mat outputLayer = net.forward("detection_out");

		/* クラスラベルの読み取り */
		std::vector<std::string> classNames = readClassNames(MODEL_DIR + "\\coco.names");

		showClassifiedResult(outputLayer, classNames);

		/* 出力層をcsv形式でファイル化する */
		reportOutputLayerAsCsv(outputLayer, classNames);

	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
	return 0;
}
