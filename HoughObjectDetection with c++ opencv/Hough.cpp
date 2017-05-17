#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/traits.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <list>

using namespace cv;

Mat src, srcGray, srcEdge;
int maxLines;

struct AccumulatorOutput 
{
	Mat H;//Mat variable type is a matrix
	Mat RhoConverter;
	Mat ThetaConverter;
} accumulator_output;

//functions
AccumulatorOutput hough_lines_acc(Mat BW, float RhoResolution, float ThetaResolution);
Mat hough_peaks(Mat H, int numpeaks, int Threshold, int NHoodSize);
void drawLines(Mat src, Mat peaks);


int main(int, char** argv)//argv can be used instead  of the image input
{
	/// Load an image
	src = imread("../data/0318171715.jpg", IMREAD_COLOR);//<- image input
	//gets the grayscale image for the canny edge finder
	srcGray = imread("../data/0318171715.jpg", IMREAD_GRAYSCALE);

	if (src.empty())
	{
		return -1;
	}

	std::cout << "Enter Max Lines to create: ";
	std::cin >> maxLines;//The amount of lines that will be drawn on the image

	std::cout << std::endl << "Working..." << std::endl;

	Canny(src, srcEdge, 80, 300, 3);
	std::cout << std::endl << "Canny edge image created" << std::endl;

	try
	{
		bool justEdgeImage = false;
		if (justEdgeImage)
		{
			namedWindow("Image with lines and circles", WINDOW_AUTOSIZE);//for testing
			imshow("Image with lines and circles", srcEdge);
		}
		else
		{
			accumulator_output = hough_lines_acc(srcEdge, 0.5, 0.5);//Gets the distrebution of where possable lines are
			std::cout << std::endl << "Accumulator has finished." << std::endl;
			Mat peaks = hough_peaks(accumulator_output.H, maxLines, 20, 20);//finds the places with the highest possibility of being a line
			std::cout << std::endl << "Found the " << maxLines << " best peaks to be used to create the line." << std::endl;
			namedWindow("Image with lines and circles", WINDOW_AUTOSIZE);
			drawLines(src, peaks);
			std::cout << std::endl << "All Done, Image Opened." << std::endl;
		}
	}
	catch (cv::Exception e)
	{
		std::cout << e.what();
	}
	catch (...)
	{
		std::cout << "Unknown exception";
	}

	waitKey(0);
	return 0;
}

AccumulatorOutput hough_lines_acc(Mat BW, float RhoResolution, float ThetaResolution)
{
	int rhoBins = int(2 * ((sqrt(pow(BW.cols, 2) + pow(BW.rows, 2)) / RhoResolution)) + 1);//gets to total rho bins using the diagonal distance times 2
	int thetaBins = int(180 / ThetaResolution) + 1;// itis onle 0 - 180 not including 180

	Mat H = Mat::zeros(rhoBins, thetaBins, CV_64F); //CV_64F is the double variable type
	Mat theta = Mat::zeros(thetaBins,1, CV_64F);//used for conversions
	Mat rho = Mat::zeros(rhoBins,1, CV_64F);	//used for conversions

	for (int i = 0; i < rhoBins; i++)//These are used for conversions
	{
		rho.at<double>(i,0) = (double(i) - rhoBins / 2) * RhoResolution;
	}
	for (int i = 0; i < thetaBins; i++)
	{
		theta.at<double>(i,0) = (double(i)*ThetaResolution) - ThetaResolution;
	}

	for (int y = 0; y < BW.rows; y++)
	{
		for (int x = 0; x < BW.cols; x++)
		{
			if (BW.at<uchar>(y, x) != 0)//only uses the images that are edges
			{
				for (float tryTheta = 0; tryTheta < 180; tryTheta += ThetaResolution)
				{
					double rhoFound = (x * cos(tryTheta)) - (y * sin(tryTheta));
					int rhoFound_m = int(ceil(rhoFound / RhoResolution + rhoBins / 2));
					int tryTheta_m = int(ceil(tryTheta / ThetaResolution)) + 1;
					H.at<double>(rhoFound_m, tryTheta_m)++;//give this bin a vote for becoming a line
				}
			}
		}
	}
	AccumulatorOutput accumulatorOutput;
	accumulatorOutput.H = H;
	accumulatorOutput.RhoConverter = rho;
	accumulatorOutput.ThetaConverter = theta;
	return accumulatorOutput;
}

Mat hough_peaks(Mat H, int numpeaks, int Threshold, int NHoodSize)
{
	Mat H_m = H;//I use _m to represent that the variable was modified for the original

	Mat peaks = Mat::zeros(numpeaks, 2, CV_32SC1);//CV_32SC1 is a signed int variable type, there was not unsigned equivalent
	std::vector<std::pair<int,int>> possablePoints;//vector is used because it is easier to get values at a certain point

	NHoodSize /= 2;

	for (int y = 0; y < H_m.rows; y++)
	{
		for (int x = 0; x < H_m.cols; x++)
		{
			if (H_m.at<double>(y, x) > Threshold)
			{
				std::pair<int, int> point = std::pair<int, int>(x, y);
				possablePoints.push_back(point);//adds the point to the vector like a list
			}
		}
	}

	
	for (int i = 0; i < numpeaks; i++)
	{
		int greatestPeakPosX = 0;//stores the position of the line in H
		int greatestPeakPosY = 0;
		for (int j = 0; j < possablePoints.size(); j++)
		{
			if (H_m.at<double>(possablePoints[j].second, possablePoints[j].first) > Threshold)
			{
				if (H_m.at<double>(possablePoints[j].second, possablePoints[j].first) > H_m.at<double>(greatestPeakPosY, greatestPeakPosX))
				{
					greatestPeakPosX = possablePoints[j].first;//first second : x y : col row
					greatestPeakPosY = possablePoints[j].second;
				}
			}
		}
					
		peaks.at<int>(i, 0) = greatestPeakPosX;
		peaks.at<int>(i, 1) = greatestPeakPosY;
		H_m.at<double>(greatestPeakPosY, greatestPeakPosX) = 0;

		for (int nHoodX = -NHoodSize; nHoodX < NHoodSize; nHoodX++)//removes all the points around the line to encourage more different lines
		{
			for (int nHoodY = -NHoodSize; nHoodY < NHoodSize; nHoodY++)
			{
				if ((greatestPeakPosX + nHoodX) > 0 && (greatestPeakPosX + nHoodX) < H_m.cols && (greatestPeakPosY + nHoodY) > 0 && greatestPeakPosY + nHoodY < H_m.rows)
				{
					H_m.at<double>(greatestPeakPosY + nHoodY, greatestPeakPosX + nHoodX) = 0;
				}
			}
		}
	}
	return peaks;
}

void drawLines(Mat src, Mat peaks)
{
	Point start;
	Point end;
	for (int i = 0; i < peaks.rows; i++)//uses the polar repusentation of a line
	{
		int peakstheta = peaks.at<int>(i, 0);//rho peak at i in the matrix of peaks
		int peaksRho = peaks.at<int>(i, 1);

		if (accumulator_output.ThetaConverter.at<double>(peakstheta,0) == 90)//if the line is vertical
		{
			start = Point(0, int(-1 * (1/sin(accumulator_output.ThetaConverter.at<double>(peakstheta,0))) * accumulator_output.RhoConverter.at<double>(peaks.at<int>(i, 0),0)));
			end = Point(src.cols, int(-1 * (1 / sin(accumulator_output.ThetaConverter.at<double>(peakstheta,0))) * accumulator_output.RhoConverter.at<double>(peaksRho, 0)));
		}
		else if (accumulator_output.ThetaConverter.at<double>(peakstheta, 0) == 0)//if the line is horizontal
		{
			start = Point(int(accumulator_output.RhoConverter.at<double>(peaksRho,0)), 0);
			end = Point(int(accumulator_output.RhoConverter.at<double>(peaksRho, 0)), src.rows);
		}
		else//every other position, this is done because the cot and csc were used
		{
			start = Point(0, int(-1 * (1 / sin(accumulator_output.ThetaConverter.at<double>(peakstheta, 0))) * accumulator_output.RhoConverter.at<double>(peaksRho, 0)));
			double m = (1 / tan(accumulator_output.ThetaConverter.at<double>(peakstheta, 0)));
			end = Point(src.cols, int(m*(src.cols) + start.y));
		}

		int thickness = 1;
		int lineType = 8;
		line(src, start, end, Scalar(0, 255, 0), thickness, lineType);
	}

	imshow("Image with lines and circles", src);//"Image with lines and circles" is the window name
}