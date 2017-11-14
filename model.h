/*
 * model.h
 * The Model class helps the View class load and save sprite projects.
 *
 * Kira Parker
 * Torin McDonald
 * Melody Chang
 * Christian Purdy
 * Brayden Carlson
 *
 * A7: Sprite Editor Implementation (11/13/17)
*/

#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <vector>
#include "frame.h"

class Model : public QObject{
    Q_OBJECT

public:
    explicit Model(QObject *parent = nullptr);

signals:
    void fileFailedToOpen(QString error); //emitted when a file cannot be opened
    void finishLoadingProject(vector<Frame*>);

public slots:
    void loadProject(QString fileName);
    void saveProject(vector<Frame*> frames, int currentFrameSize, QString fileName); //called when the project needs to be saved

private:
    const int MAX_FRAME_SIZE = 40; //maximum size of a frame in pixels (number of rows, number of columns leq MAX_FRAME_SIZE)
};

#endif // MODEL_H
