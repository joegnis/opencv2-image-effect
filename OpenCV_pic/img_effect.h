#pragma once

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

void extractEdge(cv::InputArray _src, cv::OutputArray _dst);
void applySketchEffect(cv::InputArray, cv::OutputArray,
					   cv::Size blur_ksize, double blur_sigmaX, double blur_sigmaY = 0);

void applyCartoonEffect(cv::InputArray, cv::OutputArray,
						int num_down, int num_bilateral,
						int bilateral_d, int bilateral_sigmaColor, int bilateral_sigmaSpace);

void applyOilPaintingEffect(cv::InputArray _src, cv::OutputArray _dst, uchar radius, uchar max_intensity);

void applyOldPhotoEffect(cv::InputArray _src, cv::OutputArray _dst);
void applyNegativePhotoEffect(cv::InputArray _src, cv::OutputArray _dst);
void applyReliefEffect(cv::InputArray _src, cv::OutputArray _dst);
void applySmeltColorEffect(cv::InputArray _src, cv::OutputArray _dst);
void applyIceColorEffect(cv::InputArray _src, cv::OutputArray _dst);