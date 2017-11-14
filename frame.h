/*
 * frame.h
 * The Frame class stores the individual pixles (QColors) that make up each frame in the sprite animation sequence.
 *
 * Kira Parker
 * Torin McDonald
 * Melody Chang
 * Christian Purdy
 * Brayden Carlson
*/

#ifndef FRAME_H
#define FRAME_H

#include <QTableWidgetItem>
#include<vector>

using namespace std;

class Frame{
public:
    Frame(vector<vector<QColor>> pixels); //creates a new frame with the given 2d vector of pixels
    vector<vector<QColor>> getPixels(); //gets the 2d vector of pixels for the frame
    void saveFrame(vector<vector<QColor>> currentFrame); //saves a new 2d vector of pixels for the current frame

private:
    vector<vector<QColor>> pixels_; //stores the colors for each frame (i.e. pixels)
};

#endif // FRAME_H
