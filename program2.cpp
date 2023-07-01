#include <iostream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "USAGE: " << argv[0] << " " << "<image_path>" << std::endl;
        return 0;
    }
    else {
        cv::Mat image = cv::imread(argv[1], cv::IMREAD_COLOR);
        cv::Mat image_copy = image.clone();
        
        if (!image.data) {
            std::cout << "Error while opening file " << argv[1] << std::endl;
            return 0;
        }

        std::cout << "image width: " << image.size().width << std::endl;
        std::cout << "image height: " << image.size().height << std::endl;
        std::cout << "image channels: " << image.channels() << std::endl;

        // Display original image
        cv::imshow("image", image);

        // Convert to grayscale
        cv::Mat imageGray;
        cv:cvtColor(image, imageGray, cv::COLOR_BGR2GRAY);
        cv::imshow("grayscale image", imageGray);

        // Experimental section

        // Blur image to reduce noise
        // Bigger kernel size == more blurr
        cv::Mat imageBlurred;
        cv::GaussianBlur(imageGray, imageBlurred, cv::Size(5, 5), 0);
        cv::imshow("blurred image", imageBlurred);


        // Find edges
        cv::Mat imageEdges;
        const double cannyThreshold1 = 100;
        const double cannyThreshold2 = 200;
        const int cannyAperture = 3;
        cv::Canny(imageBlurred, imageEdges, cannyThreshold1, cannyThreshold2, cannyAperture);
        cv::imshow("edges", imageEdges);


        // Erode and Dilate to remove edge noise
        int morphologySize = 1;
        cv::Mat edgesDilated;
        cv::dilate(imageEdges, edgesDilated, cv::Mat(), cv::Point(-1,-1), morphologySize);
        cv::Mat edgesEroded;
        cv::dilate(edgesDilated, edgesEroded, cv::Mat(), cv::Point(-1,-1), morphologySize);
        cv::imshow("eroded and dilated", edgesEroded);


        // Find contours
        // For our test image we will have 9 contours representing the 9 coins
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(edgesEroded, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);


        // Draw contours onto a blank black image
        cv::Mat imageContours = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
        cv::RNG rand(12345);
        for(int i = 0; i < contours.size(); i++) {
            cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0, 256), rand.uniform(0, 256));
            cv::drawContours(imageContours, contours, i, color);
        }
        cv::imshow("image contours", imageContours);


        // // Compute the minimum area bounding rectangles
        // // We have a vector of RotatedRect Objects with the size of how many contours we have
        // std::vector<cv::RotatedRect> minAreaRectangles(contours.size());
        // for(int i = 0; i < contours.size(); i++) {
        //     // compute a minimum area bounding rectangle for the contour
        //     minAreaRectangles[i] = cv::minAreaRect(contours[i]);
        // }
        
        // // Draw the rectangles on a new Mat named imageRectangles
        // cv::Mat imageRectangles = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
        // for(int i = 0; i < contours.size(); i++) {
        //     // Generating a random color for each computed min area rectangle
        //     cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));

        //     // Object that stores an (x,y) in float and we have an array of 4 of these objects
        //     // for each rectangle point
        //     cv::Point2f rectanglePoints[4];

        //     // Filling the array rectanglePoints with the 4 points of the current
        //     // min area rectangle 
        //     minAreaRectangles[i].points(rectanglePoints);
        //     for(int j = 0; j < 4; j++) {
        //         // Connecting all the points with lines on to the image
        //         cv::line(imageRectangles, rectanglePoints[j], rectanglePoints[(j+1) % 4], color);
        //     }
        // }
        // cv::imshow("image rectangles", imageRectangles);

        cv::Scalar red(0,0,255);
        cv::Scalar yellow(0,255,255);
        cv::Scalar blue(255,0,0);
        cv::Scalar green(0,255,0);

        // Fit ellipses to contours containing sufficient inliners
        // Defining a vector of RotatedRect objects named minEllipses
        std::vector<cv::RotatedRect> minEllipses(contours.size());
        for(int i = 0; i < contours.size(); i++) {
            
            // compute an ellipse only if the contour has more than 5 points (the minimum for ellipse fitting)
            if(contours.at(i).size() > 5) {
                minEllipses[i] = cv::fitEllipse(contours[i]);
            }
        }

        const int minEllipseInliers = 50;
        for(int i = 0; i < contours.size(); i++) {
            
            // draw any ellipse with sufficient inliers
            if(contours.at(i).size() > minEllipseInliers) {

                double height = minEllipses[i].size.height;
                double width = minEllipses[i].size.width;
                double ellipseArea = CV_PI * (height/2.0) * (width/2.0);
                std::cout << "area for ellipse " << i << ": " << ellipseArea << std::endl;

                // Quarter
                if(ellipseArea > 9500.00) {
                    cv::ellipse(image_copy, minEllipses[i], green, 1);
                }
                // Nickel
                else if (ellipseArea > 7500.00) {
                    cv::ellipse(image_copy, minEllipses[i], yellow, 1);
                }
                // Penny or Dime
                else {
                    cv::ellipse(image_copy, minEllipses[i], red, 1);
                }
            }
        }
        cv::imshow("image ellipse", image_copy);
        cv::waitKey(0);

    }

}