#include <opencv2/highgui/highgui.hpp>
#include <opencv2/photo/photo.hpp>

// IMPLEMENTATION OF XIAO'S IMAGE COLOUR TRANSFER METHOD,
// WITH ADDITIONAL RUGGEDISATION BY TERRY JOHNSON.

// This is a C++/OpenCV implementation of the processing
// that was first presented as a Matlab implementation here.
// https://github.com/hangong/Xiao06_color_transfer

// Copyright © Terry Johnson June 2020
// https://github.com/TJCoding


// References:
// Xiao, Xuezhong, and Lizhuang Ma. "Color transfer in correlated color
// space."
// In Proceedings of the 2006 ACM international conference on
// Virtual reality continuum and its applications, pp. 305-309. ACM, 2006.

// Runs under OpenCV 2.4.13.7
// For some later versions, CV_BGRA2BGR -> cv:COLOR_BGRA2BGR.


cv::Mat Xiao06(cv::Mat imgs, cv::Mat imgt);
void MatchColumns(cv::Mat& U_s, cv::Mat& A_s, cv::Mat U_t, cv::Mat& A_t);
bool Ruggedise;


int main()
{

// ##########################################################################
// #######################  PROCESSING SELECTIONS  ##########################
// ##########################################################################

    // Specify the image files that are to be processed,
    // where 'source image' provides the colour scheme that
    // is to be applied to 'target image'.

    std::string targetname = "images/Flowers_target.jpg";
    std::string sourcename = "images/Flowers_source.jpg";

    // Specify ruggedised processing or else standard processing.
    Ruggedise=true;

// ###########################################################################
// ###########################################################################
// ###########################################################################

    // Read in the files.
    cv::Mat target = cv::imread(targetname,1);
    cv::Mat source = cv::imread(sourcename,1);

    target=Xiao06(source,target);

    // Display and save the final image.
    cv::imshow("processed image",target);
    cv::imwrite("images/processed.jpg",target);
    cv::waitKey(0);

	return 0;
}

cv::Mat Xiao06(cv::Mat imgs,cv::Mat imgt)
{
    cv::Mat cov, means;
    cv::Mat U_t, A_t, U_s, A_s, unused;
	cv::Mat R_s = cv::Mat::eye(4, 4, CV_64FC1);
	cv::Mat R_t = R_s.clone();
    cv::Mat result;
	cv::Rect ROI(0, 0, 3, 3);

	imgs.convertTo(imgs, CV_32FC3, 1/255.0);
	imgt.convertTo(imgt, CV_32FC3, 1/255.0);

	cv::calcCovarMatrix(imgs.reshape(1, imgs.cols * imgs.rows),
                  cov, means, CV_COVAR_ROWS | CV_COVAR_NORMAL);
    cv::Mat T_s=(cv::Mat_<double>(4,4)
                 << 1,0,0,   means.at<double>(0, 0),
                    0,1,0,   means.at<double>(0, 1),
                    0,0,1,   means.at<double>(0, 2),
                    0,0,0,                       1);
	cv::SVD::compute(cov, A_s, U_s, unused);

    cv::calcCovarMatrix(imgt.reshape(1, imgt.cols * imgt.rows),
                  cov, means, CV_COVAR_ROWS | CV_COVAR_NORMAL);
    cv::Mat T_t=(cv::Mat_<double>(4,4)
                 << 1,0,0,  -means.at<double>(0, 0),
                    0,1,0,  -means.at<double>(0, 1),
                    0,0,1,  -means.at<double>(0, 2),
                    0,0,0,                       1);
	cv::SVD::compute(cov, A_t, U_t, unused);

    // Ruggedise (if flag set in 'Main') and also
    // compute the square root of the A_ matrices,
    // (which is omitted from the original paper).
	MatchColumns(U_s, A_s, U_t, A_t);

	U_s.copyTo(R_s(ROI));
    invert(U_t, R_t(ROI));

    cv::Mat S_s=(cv::Mat_<double>(4,4)
                 <<       (A_s.at<double>(0, 0)),   0,0,0,
                    0,    (A_s.at<double>(1, 0)),     0,0,
                    0,0,  (A_s.at<double>(2, 0)),       0,
                                                  0,0,0,1);
    cv::Mat S_t=(cv::Mat_<double>(4,4)
                 <<       1/(A_t.at<double>(0, 0)),  0,0,0,
                    0,    1/(A_t.at<double>(1, 0)),    0,0,
                    0,0,  1/(A_t.at<double>(2, 0)),      0,
                                                   0,0,0,1);

	transform(imgt, result, T_s*R_s*S_s*S_t*R_t*T_t);

	// Remove the additional channel that was added by
	// the last operation.
	cv::cvtColor(result,result,CV_BGRA2BGR);

	result.convertTo(result, CV_8UC3, 255);

	return result;
}

void MatchColumns(cv::Mat& U_s, cv::Mat& A_s, cv::Mat U_t, cv::Mat& A_t)
{
// This routine matches columns in the source image rotation matrix to those
// in the target image rotation matrix, if ruggedisation is selected.
//
// (Additionally, it computes the square root of the singular value matrices.)

// Each rotation matrix is derived by undertaking a singular value
// decomposition of the respective cross covariance matrices.

// The outcome of the decomposition is presented in the order of the
// descending singular values.  This often leads to compatible rotation
// matrices but that is not guaranteed.  Sometimes the colour ordering
// of one rotation matrix may be different from the other.
// Additionally even when the matrices do correspond in orientation, they
// need not correspond in direction. The direction of one may be the
// negative of the other.

// Rotations of the individual colour axes are given by the columns of the
// rotation matrices.  In the following processing, all rearrangements are
// considered of the columns in the source rotation matrix 'U_s', to find
// the arrangement that best matches the target rotation matrix 'U_t'.
// Matching is measured by taking the vector dot products of the
// corresponding matrix columns and finding the arrangement with the
// largest sum of absolute dot product values.  Absolute values are used
// to accommodate axis pairs that have similar orientations but different
// directions.

// Once the best match has been found this is taken as the correct source
// image rotation matrix.  Columns are negated where the vector cross
// product has a negative value, to ensure direction compatibility.  The
// singular value matrix for the target image is reordered to match the
// reordering of the rotation matrix.

// This processing method and routine copyright Dr T E Johnson 2020.
// terence.johnson@gmail.com

   float DotSum, maxval=0.0;
   cv::Mat MDotSum;
   int c0,c1,c2, bestperm;

  if(Ruggedise)
  {
    int perm[6][3]={0,1,2,0,2,1,1,0,2,
                    1,2,0,2,0,1,2,1,0};
     for(int i=0;i<6;i++)
     {
       c0=perm[i][0];
       c1=perm[i][1];
       c2=perm[i][2];
       MDotSum=cv::abs(U_s.col(c0).t()*U_t.col(0))+
               cv::abs(U_s.col(c1).t()*U_t.col(1))+
               cv::abs(U_s.col(c2).t()*U_t.col(2));
       DotSum=(float)MDotSum.at<double>(0,0);
       if(DotSum>maxval)
       {
           maxval=DotSum;
           bestperm=i;
        }
     }
     c0=perm[bestperm][0];
     c1=perm[bestperm][1];
     c2=perm[bestperm][2];

     cv::Mat a_s=A_s.clone();
     cv::Mat u_s=U_s.clone();

     MDotSum=u_s.col(c0).t()*U_t.col(0);
     U_s.col(0)=u_s.col(c0)*copysign(1,MDotSum.at<double>(0,0));
     MDotSum=u_s.col(c1).t()*U_t.col(1);
     U_s.col(1)=u_s.col(c1)*copysign(1,MDotSum.at<double>(0,0));
     MDotSum=u_s.col(c2).t()*U_t.col(2);
     U_s.col(2)=u_s.col(c2)*copysign(1,MDotSum.at<double>(0,0));

     A_s.at<double>(0,0)=a_s.at<double>(c0,0);
     A_s.at<double>(1,0)=a_s.at<double>(c1,0);
     A_s.at<double>(2,0)=a_s.at<double>(c2,0);
  }
  // Compute the square root of A_ matrices.
  // (which is omitted from the original paper).
  cv::sqrt(A_s,A_s);
  cv::sqrt(A_t,A_t);
}

