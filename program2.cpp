#include <iostream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

#define QUARTER_VAL 0.25
#define DIME_VAL 0.10
#define NICKEL_VAL 0.05
#define PENNY_VAL 0.01

void print_totals(int quarters, int dimes, int nickels, int pennies) {
    double total = 
        (QUARTER_VAL * quarters) + 
        (DIME_VAL * dimes) + 
        (NICKEL_VAL * nickels) + 
        (PENNY_VAL * pennies);
    
    std::cout << "Penny - " << pennies << std::endl;
    std::cout << "Nickel - " << nickels << std::endl;
    std::cout << "Dime - " << dimes << std::endl;
    std::cout << "Quarter - " << quarters << std::endl;
    std::cout << "Total - $" << total << std::endl;
}

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "USAGE: " << argv[0] << " " << "<image_path>" << std::endl;
        return 0;
    }
    else {

        // Original image read in
        cv::Mat image = cv::imread(argv[1], cv::IMREAD_COLOR);

        // Image for displaying final results
        cv::Mat image_result = image.clone();
        
        if (!image.data) {
            std::cout << "Error while opening file " << argv[1] << std::endl;
            return 0;
        }
        

        // Display original image
        cv::imshow("image", image);


        // Blur image to reduce noise
        // Bigger kernel size == more blurr
        cv::Mat imageBlurred;
        cv::GaussianBlur(image, imageBlurred, cv::Size(5, 5), 0);
        //cv::imshow("blurred image", imageBlurred);


        // Convert to grayscale
        cv::Mat imageGray;
        cv:cvtColor(imageBlurred, imageGray, cv::COLOR_BGR2GRAY);
        //cv::imshow("grayscale image", imageGray);


        // Find edges
        cv::Mat imageEdges;
        const double cannyThreshold1 = 100;
        const double cannyThreshold2 = 200;
        const int cannyAperture = 3;
        cv::Canny(imageBlurred, imageEdges, cannyThreshold1, cannyThreshold2, cannyAperture);
        //cv::imshow("edges", imageEdges);


        // Erode and Dilate to remove edge noise
        int morphologySize = 1;
        cv::Mat edgesDilated;
        cv::dilate(imageEdges, edgesDilated, cv::Mat(), cv::Point(-1,-1), morphologySize);
        cv::Mat edgesEroded;
        cv::erode(edgesDilated, edgesEroded, cv::Mat(), cv::Point(-1,-1), morphologySize);
        //cv::imshow("eroded and dilated", edgesEroded);


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
        //cv::imshow("image contours", imageContours);


        // Define colors and totals for coins
        cv::Scalar red(0,0,255);        // Penny
        int pennies = 0;

        cv::Scalar yellow(0,255,255);   // Nickel
        int nickels = 0;

        cv::Scalar blue(255,0,0);       // Dime
        int dimes = 0;

        cv::Scalar green(0,255,0);      // Quarter
        int quarters = 0;


        // Fit ellipses to contours containing sufficient inliners
        // Defining a vector of RotatedRect objects named minEllipses
        std::vector<cv::RotatedRect> minEllipses(contours.size());
        for(int i = 0; i < contours.size(); i++) {
            
            // compute an ellipse only if the contour has more than 5 points (the minimum for ellipse fitting)
            if(contours.at(i).size() > 5) {
                minEllipses[i] = cv::fitEllipse(contours[i]);
            }
        }


        // Make a copy of blurred image for HSV image
        cv::Mat imageHSV;
        cv::cvtColor(imageBlurred, imageHSV, cv::COLOR_BGR2HSV);
        //cv::imshow("imageHSV", imageHSV);


        const int minEllipseInliers = 50;
        for(int i = 0; i < contours.size(); i++) {
            
            // draw any ellipse with sufficient inliers
            if(contours.at(i).size() > minEllipseInliers) {

                // Calculate area of ellipses
                double ellipseArea = CV_PI * (minEllipses[i].size.height / 2.0) * (minEllipses[i].size.width / 2.0);


                // Quarter
                if(ellipseArea > 8250.00) {
                    cv::ellipse(image_result, minEllipses[i], green, 1);
                    quarters++;
                }
                // Nickel
                else if (ellipseArea > 6250.00) {
                    cv::ellipse(image_result, minEllipses[i], yellow, 1);
                    nickels++;
                }
                // Penny or Dime
                else {
                    cv::Mat mask = cv::Mat::zeros(imageHSV.size(), CV_8UC1);
                    cv::ellipse(mask, minEllipses[i], cv::Scalar(255,255,255), -1);
                    cv::Scalar avgHSV = cv::mean(imageHSV, mask);

                    double avgHue = avgHSV[0];

                    // Dime
                    if (avgHue > 18.00) {
                        cv::ellipse(image_result, minEllipses[i], blue, 1);
                        dimes++;
                    }
                    // Penny!
                    else {
                        cv::ellipse(image_result, minEllipses[i], red, 1);
                        pennies++;
                    }

                    // Debugging line
                    //std::cout << "i = " << i << ", " << "avgHSV: " << avgHSV << "area: " << ellipseArea << std::endl;
                }
            }
        }

        print_totals(quarters, dimes, nickels, pennies);
        cv::imshow("image result", image_result);
        cv::waitKey(0);
    }

}