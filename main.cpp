﻿#include <opencv2/opencv.hpp>
#include<fstream>
#include<chrono>
#include<omp.h>
#include<random>
#include "pso.h"

using namespace cv;
using namespace std;


//additional data to be passed to the optimization function
struct PSO_data
{
	Mat img1, img2;
	//add something if necessary
};


static double cost_Function(double *vec, int dim, void *params)
{
	//read passed parameters
	PSO_data* ddPSO = (PSO_data*)params;
	Mat img1 = ddPSO->img1;
	Mat img2 = ddPSO->img2;

	//rotate,scaleand shift the original image img1 so as to match the transformed one img2
	Mat img3;
	Mat M = getRotationMatrix2D(Point2f(img1.cols / 2, img1.rows / 2), vec[0], vec[1]);
	M.at<double>(0, 2) += vec[2];
	M.at<double>(1, 2) += vec[3];
	warpAffine(img1, img3, M, Size(), INTER_LINEAR, BORDER_CONSTANT);

	Mat Mxor;
	bitwise_xor(img2, img3, Mxor);
	//visualize results step by step, set number of threads to 1 in the main function!!!
	//imshow("roznica2", Mxor);
	//waitKey(1);
	
	//this is to be minimized
	double cost = countNonZero(Mxor); //number of non zero pixels (theoretically should be 0)
	//double cost = sum(Mxor)[0]; //or perhaps sum of all pixels' values (also should be 0)
	
	return cost;
}

int main()
{
	//optimization task: a white rectangle is rotated, scaled and shifted arbitrarily
	//having a base rectangle find the angle of rotation, scale and x-y shift so as to match these rectangles
	//simple imgage manipulation is used

	//base white rectangle on ablack background
	Mat img1(Size(800, 800), CV_8U, Scalar(0));
	rectangle(img1, Rect(200, 300, 300, 100), Scalar(255), -1);

	//create rotation matrix with scaling
	Mat M = getRotationMatrix2D(Point2f(img1.cols/2, img1.rows/2), 47.89, 1.567);
	//add x-y shift
	M.at<double>(0, 2) += 56.78;
	M.at<double>(1, 2) += -7.89;
	//the values above are to be found by the optimization routine


	//transform the original image
	Mat img2;
	warpAffine(img1, img2, M, Size(), INTER_LINEAR, BORDER_CONSTANT);
	imshow("original", img1);
	imshow("transformed", img2);
	waitKey(1);
	
	//PSO optimization in the main function
	//use 12 cores to speed up optimization
	omp_set_num_threads(12); 
	//set up optimization task - lower and upper bounds of all variables - that is enough
	//pso_settings_t *settings = pso_settings_new({ -90,.1,-300,-300 }, { 90,2,300,300 }); //rotation, scale, x shift, y shift
	//or define more parameters during initialization
	pso_settings_t *settings = pso_settings_new({ -90,.1,-300,-300 }, { 90,2,300,300 },10,25,1000,1,50);
	
	printf("PSO swarm size=%d\n", settings->size); //all PSO parameters can be adjusted for a specific task, just set settings->size to a different value

	// initialize GBEST solution and allocate memory for the best position buffer
	pso_result_t solution;
	solution.gbest = (double *)malloc(settings->dim * sizeof(double));
	
	//prepare additional data to be passed to the optimization function
	PSO_data function_params{img1, img2};
	
	//run the optimization!
	pso_solve(*cost_Function, (void*)&function_params, &solution, settings);

	cout << "SOLUTION:\n";
	cout<< solution.gbest[0]<<endl;
	cout << solution.gbest[1]<<endl;
	cout << solution.gbest[2] << endl;
	cout << solution.gbest[3] << endl;


	//verify
	Mat img3;
	M = getRotationMatrix2D(Point2f(img1.cols / 2, img1.rows / 2), solution.gbest[0], solution.gbest[1]);
	M.at<double>(0, 2) += solution.gbest[2];
	M.at<double>(1, 2) += solution.gbest[3];
	warpAffine(img1, img3, M, Size(), INTER_LINEAR, BORDER_CONSTANT);

	Mat Mxor;
	bitwise_xor(img2, img3, Mxor);
	imshow("difference", Mxor);
	
	
	waitKey();

	return 0;
}
