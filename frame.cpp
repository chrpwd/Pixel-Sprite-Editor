/*
 * frame.cpp
 * An implementation of the Frame class.
 *
 * Kira Parker
 * Torin McDonald
 * Melody Chang
 * Christian Purdy
 * Brayden Carlson
 *
 * A7: Sprite Editor Implementation (11/13/17)
*/

#include "frame.h"

using namespace std;

/*
 * create a new frame with a 2d vector if pixels
*/
Frame::Frame(vector<vector<QColor>> pixels){
    pixels_ = pixels;
}

/*
 * returns the 2d vector of pixels associated with the frame
*/
vector<vector<QColor>> Frame::getPixels(){
    return pixels_;
}

/*
 * saves a new 2d vector of pixels for the frame
*/
void Frame::saveFrame(vector<vector<QColor>> currentFrame){
    pixels_=currentFrame;
}
