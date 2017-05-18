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

struct AccumulatorOutput 
{
	Mat* H;//Mat variable type is a matrix

	Mat RhoConverter;//for lines
	Mat ThetaConverter;

	Mat PixelConverter;//for circles
	Mat RadiusConverter;
} accumulator_output_lines, accumulator_output_circles;

//functions for lines
AccumulatorOutput hough_lines_acc(Mat BW, float RhoResolution, float ThetaResolution);
Mat hough_line_peaks(Mat H, int numpeaks, int Threshold, int NHoodSize);
Mat drawLines(Mat src, Mat peaks);
//for circles
AccumulatorOutput hough_circles_acc(Mat BW, float startRadius, float endRadius, float pixelStep, float radiusStep);
Mat hough_circles_peaks(Mat H, int numpeaks, int Threshold, int NHoodSize);
Mat drawCircles(Mat src, Mat peaks);


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

	int maxLines, maxCircles, startRadius, endRadius;

	std::cout << "Enter Max Lines to create: ";
	std::cin >> maxLines;//The number of lines that will be drawn on the image

	std::cout << "Enter Max Circles to create: ";
	std::cin >> maxCircles;//The number of circles that will be drawn on the image
	if (maxCircles > 0)
	{
		std::cout << "Enter the start radius: ";
		std::cin >> startRadius;//The number of circles that will be drawn on the image
		std::cout << "Enter the end radius: ";
		std::cin >> endRadius;//The number of circles that will be drawn on the image
	}


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
			if (maxLines < 1)
			{
				std::cout << std::endl << "Skiped lines moving to circles." << std::endl;
			}
			else
			{
				accumulator_output_lines = hough_lines_acc(srcEdge, 0.5, 0.5);//Gets the distribution of where possible lines are
				std::cout << std::endl << "Accumulator for lines has finished." << std::endl;
				Mat peaks_lines = hough_line_peaks((*accumulator_output_lines.H), maxLines, 20, 20);//finds the places with the highest possibility of being a line
				std::cout << std::endl << "Found the " << maxLines << " best peaks to be used to create the lines." << std::endl;
				src = drawLines(src, peaks_lines);
				std::cout << std::endl << "All Done With lines moving to circles." << std::endl;
			}
			if (maxCircles < 1)
			{
				std::cout << std::endl << "Skiped circles." << std::endl;
			}
			else
			{
				accumulator_output_circles = hough_circles_acc(srcEdge, startRadius, endRadius, 1, 1);//Gets the distribution of where possible circles are
				std::cout << std::endl << "Accumulator for circles has finished." << std::endl;
				Mat peaks_circles = hough_circles_peaks((*accumulator_output_circles.H), maxCircles, 20, 20);//finds the places with the highest possibility of being a circle
				std::cout << std::endl << "Found the " << maxCircles << " best peaks to be used to create the circles." << std::endl;
				src = drawCircles(src, peaks_circles);
				std::cout << std::endl << "All Done With Circles." << std::endl;
			}

			namedWindow("Image with lines and circles", WINDOW_AUTOSIZE);
			imshow("Image with lines and circles", src);

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
	int thetaBins = int(180 / ThetaResolution) + 1;// it is only 0 - 180 not including 180

	Mat* H = new Mat(rhoBins, thetaBins, CV_64F, cv::Scalar(0)); //CV_64F is the double variable type
	Mat theta = Mat::zeros(thetaBins,1, CV_64F);//used for conversions
	Mat rho = Mat::zeros(rhoBins,1, CV_64F);	//used for conversions

	for (int i = 0; i < rhoBins; i++)//These are used for conversions
	{
		rho.at<double>(i, 0) = (double(i) - rhoBins / 2) * RhoResolution;
	}
	for (int i = 0; i < thetaBins; i++)
	{
		theta.at<double>(i, 0) = (double(i)*ThetaResolution) - ThetaResolution;
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
					(*H).at<double>(rhoFound_m, tryTheta_m)++;//give this bin a vote for becoming a line
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

Mat hough_line_peaks(Mat H, int numpeaks, int Threshold, int NHoodSize)
{
	Mat H_m = H;//I use _m to represent that the variable was modified for the original

	Mat peaks = Mat::zeros(numpeaks, 2, CV_32SC1);//CV_32SC1 is a signed int variable type, there was not unsigned equivalent
	std::vector<Point> possablePoints;//vector is used because it is easier to get values at a certain point

	NHoodSize /= 2;

	for (int y = 0; y < H_m.rows; y++)
	{
		for (int x = 0; x < H_m.cols; x++)
		{
			if (H_m.at<double>(y, x) > Threshold)
			{
				Point point = Point(x, y);
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
			if (H_m.at<double>(possablePoints[j].y, possablePoints[j].x) > Threshold)
			{
				if (H_m.at<double>(possablePoints[j].y, possablePoints[j].x) > H_m.at<double>(greatestPeakPosY, greatestPeakPosX))
				{
					greatestPeakPosX = possablePoints[j].x;//first second : x y : col row
					greatestPeakPosY = possablePoints[j].y;
				}
			}
		}
					
		peaks.at<int>(i, 0) = greatestPeakPosX;
		peaks.at<int>(i, 1) = greatestPeakPosY;

		for (int nHoodX = -NHoodSize; nHoodX < NHoodSize; nHoodX++)//removes all the points around the line to encourage more different lines
		{
			for (int nHoodY = -NHoodSize; nHoodY < NHoodSize; nHoodY++)
			{
				if ((greatestPeakPosX + nHoodX) >= 0 && (greatestPeakPosX + nHoodX) < H_m.cols && (greatestPeakPosY + nHoodY) >= 0 && greatestPeakPosY + nHoodY < H_m.rows)
				{
					H_m.at<double>(greatestPeakPosY + nHoodY, greatestPeakPosX + nHoodX) = 0;
				}
			}
		}
	}
	return peaks;
}

Mat drawLines(Mat img, Mat peaks)
{
	Point start;
	Point end;
	for (int i = 0; i < peaks.rows; i++)//uses the polar representation of a line
	{
		int peakstheta = peaks.at<int>(i, 0);//rho peak at i in the matrix of peaks
		int peaksRho = peaks.at<int>(i, 1);

		if (accumulator_output_lines.ThetaConverter.at<double>(peakstheta,0) == 90)//if the line is vertical
		{
			start = Point(0, int(-1 * (1 / sin(accumulator_output_lines.ThetaConverter.at<double>(peakstheta, 0))) * accumulator_output_lines.RhoConverter.at<double>(peaks.at<int>(i, 0), 0)));
			end = Point(img.cols, int(-1 * (1 / sin(accumulator_output_lines.ThetaConverter.at<double>(peakstheta, 0))) * accumulator_output_lines.RhoConverter.at<double>(peaksRho, 0)));
		}
		else if (accumulator_output_lines.ThetaConverter.at<double>(peakstheta, 0) == 0)//if the line is horizontal
		{
			start = Point(int(accumulator_output_lines.RhoConverter.at<double>(peaksRho, 0)), 0);
			end = Point(int(accumulator_output_lines.RhoConverter.at<double>(peaksRho, 0)), img.rows);
		}
		else//every other position, this is done because the cot and csc were used
		{
			start = Point(0, int(-1 * (1 / sin(accumulator_output_lines.ThetaConverter.at<double>(peakstheta, 0))) * accumulator_output_lines.RhoConverter.at<double>(peaksRho, 0)));
			double m = (1 / tan(accumulator_output_lines.ThetaConverter.at<double>(peakstheta, 0)));
			end = Point(img.cols, int(m*(img.cols) + start.y));
		}

		int thickness = 3;
		int lineType = 8;
		line(img, start, end, Scalar(0, 255, 0), thickness, lineType);
	}
	return img;
}

AccumulatorOutput hough_circles_acc(Mat BW, float startRadius, float endRadius, float pixelStep, float radiusStep)
{
	int colBins = int(BW.cols / pixelStep + 2 * endRadius);
	int rowBins = int(BW.rows / pixelStep + 2 * endRadius);
	int radiusBins = int((endRadius - startRadius) / radiusStep);

	int sizes[] = { rowBins, colBins, radiusBins };
	Mat* H = new cv::Mat(3, sizes, CV_64F, cv::Scalar(0));

	Mat pixel = Mat::zeros(((colBins >= rowBins) ? colBins : rowBins), 1, CV_64F);//used for conversions
	Mat radius = Mat::zeros(radiusBins, 1, CV_64F);	//used for conversions

	for (int i = 0; i < ((colBins >= rowBins) ? colBins : rowBins); i++)
	{
		pixel.at<double>(i, 0) = double(i) * pixelStep - endRadius;
	}
	for (int i = 0; i < radiusBins; i++)
	{
		radius.at<double>(i, 0) = double(i) * pixelStep + startRadius;
	}

	for (int y = 0; y < BW.rows; y++)
	{
		for (int x = 0; x < BW.cols; x++)
		{
			if (BW.at<uchar>(y, x) != 0)//only uses the images that are edges
			{
				for (int tryTheta = 0; tryTheta < 360; tryTheta += 1)
				{
					for (float tryRadius = startRadius; tryRadius < endRadius; tryRadius += radiusStep)
					{
						double a = (x + (tryRadius * cos(tryTheta)));
						double b = (y + (tryRadius * sin(tryTheta)));
						int a_m = int((a + endRadius) * pixelStep);
						int b_m = int((b + endRadius) * pixelStep);
						int tryRadius_m = int((tryRadius - startRadius) * radiusStep);
						(*H).at<double>(b_m, a_m, tryRadius_m)++;
					}
				}
			}
		}
	}

	AccumulatorOutput accumulatorOutput;
	accumulatorOutput.H = H;
	accumulatorOutput.PixelConverter = pixel;
	accumulatorOutput.RadiusConverter = radius;
	return accumulatorOutput;
}

Mat hough_circles_peaks(Mat H, int numpeaks, int Threshold, int NHoodSize)
{
	Mat H_m = H;//I use _m to represent that the variable was modified for the original
	int w = H_m.size[0];
	int h = H_m.size[1];
	int l = H_m.size[2];

	Mat peaks = Mat::zeros(numpeaks, 3, CV_32SC1);//CV_32SC1 is a signed int variable type, there was not unsigned equivalent
	std::vector<Point3d> possablePoints;//vector is used because it is easier to get values at a certain point

	NHoodSize /= 2;

	for (int y = 0; y < H_m.size[0]; y++)//can't use H_m.row because it is 3 dimensions
	{
		for (int x = 0; x < H_m.size[1]; x++)
		{
			for (int z = 0; z < H_m.size[2]; z++)//H_m.size[2] is for the 3d demention
			{
				if (H_m.at<double>(y, x, z) > Threshold)
				{
					Point3d* point = new Point3d(x, y, z);
					possablePoints.push_back(*point);//adds the point to the vector like a list

				}
			}
		}
	}


	for (int i = 0; i < numpeaks; i++)
	{
		int greatestPeakPosX = 0;//stores the position of the line in H
		int greatestPeakPosY = 0;
		int greatestPeakPosZ = 0;
		for (int j = 0; j < possablePoints.size(); j++)
		{
			if (int(H_m.at<double>(possablePoints[j].y, possablePoints[j].x, possablePoints[j].z)) > Threshold)
			{
				if (H_m.at<double>(possablePoints[j].y, possablePoints[j].x, possablePoints[j].z) > H_m.at<double>(greatestPeakPosY, greatestPeakPosX, greatestPeakPosZ))
				{
					greatestPeakPosX = int(possablePoints[j].x);
					greatestPeakPosY = int(possablePoints[j].y);
					greatestPeakPosZ = int(possablePoints[j].z);
				}
			}
		}

		peaks.at<int>(i, 0) = greatestPeakPosX;
		peaks.at<int>(i, 1) = greatestPeakPosY;
		peaks.at<int>(i, 2) = greatestPeakPosZ;
		H_m.at<double>(greatestPeakPosY, greatestPeakPosX, greatestPeakPosZ) = 0;

		for (int nHoodX = -NHoodSize; nHoodX < NHoodSize; nHoodX++)//removes all the points around the circle to encourage more different circles
		{
			for (int nHoodY = -NHoodSize; nHoodY < NHoodSize; nHoodY++)
			{
				for (int nHoodZ = -NHoodSize; nHoodZ < NHoodSize; nHoodZ++)
				{
					if ((greatestPeakPosX + nHoodX) >= 0 && (greatestPeakPosX + nHoodX) < H_m.size[1] && (greatestPeakPosY + nHoodY) >= 0 && greatestPeakPosY + nHoodY < H_m.size[0] && (greatestPeakPosZ + nHoodZ) >= 0 && greatestPeakPosZ + nHoodZ < H_m.size[2])
					{
						H_m.at<double>(greatestPeakPosY + nHoodY, greatestPeakPosX + nHoodX, greatestPeakPosZ + nHoodZ) = 0;
					}
				}
			}
		}
	}
	return peaks;
}

Mat drawCircles(Mat src, Mat peaks)
{
	for (int i = 0; i < peaks.rows; i++)
	{
		int radius = int(accumulator_output_circles.RadiusConverter.at<double>(peaks.at<int>(i, 2), 0));
		int a = int(accumulator_output_circles.PixelConverter.at<double>(peaks.at<int>(i, 0), 0));
		int b = int(accumulator_output_circles.PixelConverter.at<double>(peaks.at<int>(i, 1), 0));


		int thickness = 3;
		int lineType = 8;
		circle(src, Point(a, b), radius, Scalar(0, 255, 0), thickness, lineType);
	}
	return src;
}