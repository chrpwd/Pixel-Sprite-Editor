/*
 * view.h
 * The View class creates a Sprite Editor that a user can use to create sprite animations with sequences of frames.
 * It includes a frame editing area, a preview window, several different tools to help you edit the frames, and buttons to
 * add new frames and move between frames.
 * For step-by-step instructions of how to use the editor, see the User Guide.
 *
 * Kira Parker
 * Torin McDonald
 * Melody Chang
 * Christian Purdy
 * Brayden Carlson
 *
 * A7: Sprite Editor Implementation (11/13/17)
*/

#ifndef VIEW_H
#define VIEW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QTimer>
#include <QColor>
#include <QPair>
#include <vector>
#include<QStandardPaths>
#include <QMessageBox>

#include "frame.h"
#include "model.h"


using namespace std;

namespace Ui{
class View;
}

class View : public QMainWindow{
    Q_OBJECT

public:
    explicit View(Model& model, QWidget *parent = 0);
    ~View();

private:
    enum Tool{
        Draw, Fill, Erase, Rectangle, Circle
    };

    Ui::View *ui;
    vector<Frame*> frames_; //list of the frames in the order that they will be played in the animation window
    vector<Frame*> framesForUndo_; // list of the frames in order for the undo
    vector<Frame*> framesForRedo_; // list of the frames in order for the Redo

    unsigned int currentFrame_; //index of the current frame in frames_
    int currentFrameSize_; //number of rows (columns) of the current frame
    unsigned int currentPlaybackFrame_; //the index of the frame being played back in frames_
    int currentPlaybackSpeed_; //number of milliseconds a frame should be displayed for in the animation window
    QTimer playbackTimer_; //used to control how long each frame is displayed in the animation preview window
    QColor currentColor_; //color being used for pixels

    //cursor images for the different tools that can be selected
    QCursor drawCursor_;
    QCursor fillCursor_;
    QCursor eraseCursor_;
    QCursor rectOffCursor_;
    QCursor rectOnCursor_;
    QCursor circleOffCursor_;
    QCursor circleOnCursor_;

    Tool currentTool_; //tool currently selected by the user (draw, fill, erase)

    bool isDrawingShape_; //true if the user is drawing a shape (has clicked for one end of the shape but not for the other)
    QPair<int, int> shapeCoords_; //coordinates of the first side of the shape (the first corner that is clicked on)

    const int MAX_FRAME_SIZE = 40; //maximum size of a frame in pixels (number of rows, number of columns leq MAX_FRAME_SIZE)

    void fillCells(int, int); // Performs a fill with the currently selected color
    void drawRect(int, int); //draws a rectangle (first int is the y coordinate, second int is the x coordinate of a click)
    void drawCircle(int, int); //draws a circle (first int is the y coordinate, second int is the x coordinate of a click)

    void setFrameLabel(); //sets the label at the bottom that says which frame the user is on

    void checkButton(Tool); // Highlights the button for the specified tool. All other tool buttons are unchecked

public slots:
    void changeCellColor(int, int); //changes the color of the cell to the currently selected color
    void changeNumberOfPixels(int); //changes the number of pixels in the frame
    void changeAlpha(int); //changes the opacity of the pixels the user draws
    void createNewFrame(); //creates a new frame in the sprite animation sequence
    void duplicateFrame(); //creates a new frame in the sprite animation sequence with the same content as the previous frame
    void goToNextFrame(); //lets the user advance to the next frame to edit it
    void goToPreviousFrame(); //lets the user edit the previous frame in the aniimation sequence
    void displayUndoFrame(); //change the current frame to the last framesForUndo
    void displayRedoFrame(); //change the current frame to the last framesForRedo
    void changePlaybackSpeed(int newFPS); //changes the speed of the preview based on the input frames per second
    void updatePreview(); //updates the frame in the preview window
    void deleteFrame(); //called when the delete frame button is pressed, deletes the current frame
    void saveProject(); //saves the current project with help from the model
    void loadProject(); //loads the current project with help from the model


public:
    void populateWithWidgetItems(QTableWidget* table); //initially populates the table widget with table widget items
    void saveCurrentFrame(); //saves the current frame
    void saveFramesForUndo(); //saves the changes in frame for undo
    void saveFramesForRedo(); //saves the changes in frame for redo
    void loadFrame(QTableWidget* table, int frameIndex); //loads the current frame into the specified table
    void loadPreviewFrame(QTableWidget* table, int frameIndex); //loads the full current frame into the preview table


signals:
    void saveProjectSignal(vector<Frame*> frames, int currentFrameSize, QString fileName); //emitted to the model to save a  project
    void loadProjectSignal(QString fileName); //emitted to the model to load a project. the model calls finishLoadedProject when it is done

private slots:
    void on_drawToolButton_clicked(); // Changes the current tool to the draw tool
    void on_fillToolButton_clicked(); // Changes the current tool the fill tool
    void on_eraseToolButton_clicked(); // Changes the current tool the eraser tool
    void onCellClicked(int, int); // Called when a cell is clicked on so that it can be edited with the appropriate tool
    void onCellEntered(int, int); // Called when a cell is entered so that it can be edited with the appropriate tool
    void on_gifButton_clicked(); // Called when the user clicks on the "Export to Gif" button
    void on_currentColorTab_clicked(); // Called when the user clicks on the color button to change the color
    void on_currentColorTab_pressed(); // Called when the user clicks on the color button to change the color
    void on_rectToolButton_clicked(); // Called when the user clicks on the create rectangle tool
    void on_circleToolButton_clicked(); // Called when the user clicks on the create circle tool

    void fileFailedToOpen(QString error); //called when a file failed to open during save or load
    void finishLoadingProject(vector<Frame*>); //called when the loaded project needs to be displayed
};

#endif // VIEW_H
