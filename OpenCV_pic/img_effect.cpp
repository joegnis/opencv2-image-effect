#include <vector>
#include <cmath>
#include <algorithm>
#include "stdafx.h"
#include "img_effect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace cv;

void applySketchEffect(InputArray _src, OutputArray _dst, 
					   Size blur_ksize, double blur_sigmaX, double blur_sigmaY) {
	Mat src = _src.getMat();

	// [re]create the output array so that it has the proper size and type.
	// In case of Mat it calls Mat::create, in case of STL vector it calls vector::resize.
	_dst.create(src.size(), CV_MAKETYPE(src.depth(), 1));
	Mat dst = _dst.getMat();

	// Convert to gray image
	cvtColor(src, dst, CV_BGR2GRAY);
	Mat dst_gray;
	dst.copyTo(dst_gray);

	// Invert color
	bitwise_not(dst, dst);

	// Blur using a Gaussian filter
	GaussianBlur(dst, dst, blur_ksize, blur_sigmaX, blur_sigmaY);

	// Color Dodge blend
	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++) {
			uchar p_gray = dst_gray.at<uchar>(i, j);
			uchar p_dst = dst.at<uchar>(i, j);
			dst.at<uchar>(i, j) = (uchar)((p_dst == 255) ? p_dst : std::min(255, ((p_gray << 8) / (255 - p_dst))));
		}
}


void _cropImageForDownSampling(InputArray _src, OutputArray _dst, int num_down) {
	Mat src = _src.getMat();

	int diff_width = src.size().width % (int)exp2(num_down);
	int diff_height = src.size().height % (int)exp2(num_down);
	int offset_x = diff_width / 2;
	int offset_y = diff_height / 2;

	Rect crop_rect(offset_x, offset_y, src.size().width - diff_width, src.size().height - diff_height);
	_dst.create(crop_rect.size(), src.type());
	Mat dst = _dst.getMat();
	src(crop_rect).copyTo(dst);
}

void applyCartoonEffect(InputArray _src, OutputArray _dst,
						int num_down, int num_bilateral,
						int bilateral_d, int bilateral_sigmaColor, int bilateral_sigmaSpace) {
	Mat cropped_src;
	_cropImageForDownSampling(_src, cropped_src, num_down);

	_dst.create(cropped_src.size(), cropped_src.type());
	Mat dst = _dst.getMat();

	// Perform downsampling, apply bilateral filter (cartoonify) and do upsampling
	Mat dst_bilateral;
	cropped_src.copyTo(dst_bilateral);
	for (int i = 0; i < num_down; i++)
		pyrDown(dst_bilateral, dst_bilateral);

	for (int i = 0; i < num_bilateral; i++) {
		Mat dst_tmp;
		bilateralFilter(dst_bilateral, dst_tmp, bilateral_d, bilateral_sigmaColor, bilateral_sigmaSpace);
		dst_bilateral = dst_tmp;
	}

	for (int i = 0; i < num_down; i++)
		pyrUp(dst_bilateral, dst_bilateral);

	// Convert src to gray image to extract its edges
	Mat dst_edge;
	extractEdge(cropped_src, dst_edge);

	// Combine two processed images
	bitwise_and(dst_bilateral, dst_edge, dst);
}

void extractEdge(InputArray _src, OutputArray _dst) {
	Mat src = _src.getMat();
	CV_Assert(src.channels() == 3 || src.channels() == 1);

	Mat tmp_img(src.size(), CV_MAKETYPE(src.depth(), 1));
	_dst.create(src.size(), src.type());
	Mat dst = _dst.getMat();

	if (src.channels() == 3)
		cvtColor(src, tmp_img, CV_BGR2GRAY);
	medianBlur(tmp_img, tmp_img, 7);
	adaptiveThreshold(tmp_img, tmp_img, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 9, 2);
	if (src.channels() == 3)
		cvtColor(tmp_img, dst, CV_GRAY2BGR);
}

void applyOilPaintingEffect(InputArray _src, OutputArray _dst, uchar radius, uchar max_intensity) {
	CV_Assert(radius > 0 && max_intensity > 0);

	Mat src = _src.getMat();
	_dst.create(src.size(), src.type());
	Mat dst = _dst.getMat();
	int img_width = src.size().width;
	int img_height = src.size().height;

	// Calculate intensity of every pixel beforehand
	Mat_<Vec3b>& src_vec = (Mat_<Vec3b>&)src;
	Mat_<uchar> mat_intensity(src.size());
	for (int i = 0; i < img_height; i++)
		for (int j = 0; j < img_width; j++)
			mat_intensity(i, j) = (((src_vec(i, j)[0] + src_vec(i, j)[1] + src_vec(i, j)[2]) / 3.0) * max_intensity) / 255;

	for (int i = 0; i < img_height; i++)
		for (int j = 0; j < img_width; j++) {
			std::vector<int> counts_intensity(max_intensity + 1, 0);
			std::vector<Vec3i> sums_rgb(max_intensity + 1, Vec3i(0, 0, 0));
			int true_x;
			int true_y;

			// For each pixel, iterate its nearby pixels within radius
			for (int x = i - radius; x < i + radius; x++) {
				true_x = x;
				if (true_x < 0) 
					true_x = -true_x;
				if (true_x >= img_height)
					true_x = 2 * (img_height - 1) - true_x;
				for (int y = j - radius; y < j + radius; y++) {
					true_y = y;
					if (true_y < 0) 
						true_y = -true_y;
					if (true_y >= img_width)
						true_y = 2 * (img_width - 1) - true_y;

					uchar cur_intensity = mat_intensity(true_x, true_y);
					counts_intensity[cur_intensity]++;
					sums_rgb[cur_intensity] += src_vec(true_x, true_y);
				}
			}
			// Get the most repeating intensity
			std::vector<int>::iterator it_max_count = std::max_element(counts_intensity.begin(), counts_intensity.end());
			int max_repeat_intensity = std::distance(counts_intensity.begin(), it_max_count);

			// Calculate the output
			dst.at<Vec3b>(i, j) = sums_rgb[max_repeat_intensity] / *it_max_count;
		}
}

void applyOldPhotoEffect(InputArray _src, OutputArray _dst) {
	Mat src = _src.getMat();

	// [re]create the output array so that it has the proper size and type.
	// In case of Mat it calls Mat::create, in case of STL vector it calls vector::resize.
	_dst.create(src.size(), src.type());
	Mat dst = _dst.getMat();

	int img_height = src.size().height;
	int img_width = src.size().width;
	Mat_<Vec3b>& src_vec = (Mat_<Vec3b>&)src;
	for (int i = 0; i < img_height; i++)
		for (int j = 0; j < img_width; j++) {
			const Vec3b &color = src_vec(i, j);
			Vec3i new_color;

			new_color[0] = 34 * color[0] + 137 * color[1] + 70 * color[2];
			new_color[1] = 43 * color[0] + 176 * color[1] + 89 * color[2];
			new_color[2] = 48 * color[0] + 197 * color[1] + 101 * color[2];

			for (int i_color = 0; i_color < 3; i_color++) {
				new_color[i_color] = new_color[i_color] >> 8;
				if (new_color[i_color] > 255)
					new_color[i_color] = 255;
			}

			dst.at<Vec3b>(i, j) = new_color;
		}
}

void applyNegativePhotoEffect(InputArray _src, OutputArray _dst) {
	Mat src = _src.getMat();

	// [re]create the output array so that it has the proper size and type.
	// In case of Mat it calls Mat::create, in case of STL vector it calls vector::resize.
	_dst.create(src.size(), src.type());
	Mat dst = _dst.getMat();

	int img_height = src.size().height;
	int img_width = src.size().width;
	for (int i = 0; i < img_height; i++)
		for (int j = 0; j < img_width; j++) {
			const Vec3b &color = src.at<Vec3b>(i, j);
			Vec3b &new_color = dst.at<Vec3b>(i, j);

			for (int i_color = 0; i_color < 3; i_color++) {
				new_color[i_color] = 255 - color[i_color];
			}
		}

}

void applyReliefEffect(InputArray _src, OutputArray _dst) {
	Mat src = _src.getMat();
	_dst.create(src.size(), CV_MAKETYPE(src.depth(), 1));
	Mat dst = _dst.getMat();
	int img_width = src.size().width;
	int img_height = src.size().height;

	for (int i = 0; i < img_height; i++) {
		for (int j = 0; j < img_width; j++) {
			// For each pixel, take its two adjacent pixels for calculation
			std::vector<Point> adjacents;
			adjacents.push_back(Point(i - 1, j - 1));
			adjacents.push_back(Point(i + 1, j + 1));
			for (auto it_adj = adjacents.begin(); it_adj != adjacents.end(); it_adj++) {
				const Point &adj = *it_adj;
				Point true_pt(adj);
				if (true_pt.x < 0)
					true_pt.x = -adj.x;
				else if (true_pt.x >= img_height)
					true_pt.x = 2 * (img_height - 1) - adj.x;
				if (true_pt.y < 0)
					true_pt.y = -adj.y;
				if (true_pt.y >= img_width)
					true_pt.y = 2 * (img_width - 1) - adj.y;

				*it_adj = true_pt;
			}

			Vec3b new_color;
			for (int i_color = 0; i_color < 3; i_color++) {
				Vec3b &src_pixel1 = src.at<Vec3b>(adjacents[0].x, adjacents[0].y);
				Vec3b &src_pixel2 = src.at<Vec3b>(adjacents[1].x, adjacents[1].y);
				int new_value = src_pixel1[i_color] - src_pixel2[i_color] + 128;
				if (new_value > 255)
					new_value = 255;
				else if (new_value < 0)
					new_value = 0;
				new_color[i_color] = new_value;
			}

			dst.at<uchar>(i, j) = (uchar)(new_color[0] * 0.11 + new_color[1] * 0.59 + new_color[2] * 0.3);
		}
	}
}

void applySmeltColorEffect(InputArray _src, OutputArray _dst) {
	Mat src = _src.getMat();

	// [re]create the output array so that it has the proper size and type.
	// In case of Mat it calls Mat::create, in case of STL vector it calls vector::resize.
	_dst.create(src.size(), src.type());
	Mat dst = _dst.getMat();

	int img_height = src.size().height;
	int img_width = src.size().width;
	Mat_<Vec3b>& src_vec = (Mat_<Vec3b>&)src;
	for (int i = 0; i < img_height; i++)
		for (int j = 0; j < img_width; j++) {
			const Vec3b &color = src_vec(i, j);
			Vec3i new_color;

			new_color[0] = color[0] * 255 / (color[1] + color[2] + 1);
			new_color[1] = color[1] * 255 / (color[0] + color[2] + 1);
			new_color[2] = color[2] * 255 / (color[0] + color[1] + 1);

			for (int i_color = 0; i_color < 3; i_color++) {
				if (new_color[i_color] > 255)
					new_color[i_color] = 255;
			}

			dst.at<Vec3b>(i, j) = new_color;
		}
}

void applyIceColorEffect(InputArray _src, OutputArray _dst) {
	Mat src = _src.getMat();

	// [re]create the output array so that it has the proper size and type.
	// In case of Mat it calls Mat::create, in case of STL vector it calls vector::resize.
	_dst.create(src.size(), src.type());
	Mat dst = _dst.getMat();

	int img_height = src.size().height;
	int img_width = src.size().width;
	Mat_<Vec3b>& src_vec = (Mat_<Vec3b>&)src;
	for (int i = 0; i < img_height; i++)
		for (int j = 0; j < img_width; j++) {
			const Vec3b &color = src_vec(i, j);
			Vec3i new_color;

			new_color[0] = (color[0] - color[1] - color[2]) * 2 / 3;
			new_color[1] = (color[1] - color[0] - color[2]) * 2 / 3;
			new_color[2] = (color[2] - color[0] - color[1]) * 2 / 3;

			for (int i_color = 0; i_color < 3; i_color++) {
				if (new_color[i_color] < 0)
					new_color[i_color] = -new_color[i_color];
			}

			dst.at<Vec3b>(i, j) = new_color;
		}
}
