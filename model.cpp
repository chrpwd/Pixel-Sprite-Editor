/*
 * model.cpp
 * Implementation of the Model class
 *
 * Kira Parker
 * Torin McDonald
 * Melody Chang
 * Christian Purdy
 * Brayden Carlson
 *
 * A7: Sprite Editor Implementation (11/13/17)
*/

#include "model.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QPixmap>
#include <QString>
#include "frame.h"

Model::Model(QObject *parent) : QObject(parent){

}

/*
 * saves the frames_ to the file given by the fileName parameter according to the specifications in the assignment
*/
void Model::saveProject(vector<Frame*> frames_, int currentFrameSize_, QString fileName){
    vector<Frame*>::iterator frame;
    vector< vector<QColor> >::iterator row;
    vector<QColor>::iterator col;

    if(fileName.isEmpty())
        return;
    else{
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly)){
            emit fileFailedToOpen(file.errorString());
          return;
        }

        QTextStream out( &file );

        out <<  currentFrameSize_ << ' ' <<  currentFrameSize_<<endl;
        out << frames_.size()<< endl;

        for(frame=frames_.begin(); frame != frames_.end(); (frame)++){
            vector< vector<QColor> > temp= (*frame)->getPixels();
            for (int row = 0; row < currentFrameSize_; row++) {
                for (int col = 0; col < currentFrameSize_; col++) {
                    QColor item= temp[row][col];
                    out<<item.red() <<' '<<item.green() <<' '<<item.blue() <<' '<<item.alpha() << ' ';
                }
            out<<'\n';
            }
        }
    }
}

/*
 * loads a project saved to the given fileName for the view
*/
void Model::loadProject(QString fileName){
    vector<Frame*> newFrames;
    if(fileName.isEmpty())
            return;
    else{

        QFile file(fileName);

        if(!file.open(QIODevice::ReadOnly)){
            emit fileFailedToOpen(file.errorString());
            return;
        }

        QTextStream in(&file);
        QString line = in.readLine();
        QStringList dimension= line.split(" ");
        QString row= dimension.first();
        QString col= dimension.last();

        unsigned int rowSize=row.toInt();
        unsigned int colSize=col.toInt();
        vector<vector<QColor>> newPixels(MAX_FRAME_SIZE, vector<QColor>(MAX_FRAME_SIZE));
        line=in.readLine();
        unsigned int frameNum= line.toInt();

        for(unsigned int f=0; f<frameNum;f++){ //for each frame
            for(unsigned int i=0; i<rowSize;i++){ //for number of rows in a frame
                line=in.readLine();//next row

                vector<int> rgba;
                QStringList  row = line.split(" ");
                for(unsigned int j=0; j<colSize;j++){//for whole line (columns)
                    for(unsigned int k=0; k<4;k++){ //for every 4 values
                        QString num=row.first(); //get first number
                        rgba.push_back(num.toInt());
                        row.removeFirst();
                    }
                    QColor newColor; //create new qcolor from rgba
                    int a= rgba.back();
                    rgba.pop_back();
                    int b = rgba.back();
                    rgba.pop_back();
                    int g= rgba.back();
                    rgba.pop_back();
                    int r= rgba.back();
                    rgba.pop_back();
                    newColor.setRgb( r,  g,  b,  a);
                    newPixels[i][j]=newColor; //add to this row and col number
                }

            }

            for(unsigned int c = colSize; c<MAX_FRAME_SIZE;c++){
                for(unsigned int r =0; r<colSize; r++){

                    QColor newColor;
                    newColor.setRgb( 255,255,255,255);
                    newPixels[r][c] = newColor;
                }
            }
            for(unsigned int r = rowSize; r<MAX_FRAME_SIZE;r++){
                for(unsigned int c = 0; c<MAX_FRAME_SIZE;c++)
                {
                QColor newColor;
                newColor.setRgb( 255,255,255,255);
                newPixels[r][c] = newColor;
                }
            }
            Frame* newFrame= new Frame(newPixels);
            newFrames.push_back(newFrame);
        }
        emit finishLoadingProject(newFrames);
    }
}
