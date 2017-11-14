/*
 * view.cpp
 * The implementation of the View class.
 *
 * Kira Parker
 * Torin McDonald
 * Melody Chang
 * Christian Purdy
 * Brayden Carlson
 *
 * A7: Sprite Editor Implementation (11/13/17)
*/

#include "view.h"
#include "ui_view.h"
#include "model.h"
#include <math.h>
#include <QQueue>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include "gif.h"
#include <QDebug>
#include <QByteArray>
#include <QImage>
#include <QPixmap>


View::View(Model& model, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::View){

    ui->setupUi(this);
    // set the default size of the edit frame table so the entry areas are square and the user cannot change the size of the table
    ui->editFrameTable->horizontalHeader()->setDefaultSectionSize(ui->editFrameTable->width()/ui->editFrameTable->rowCount());
    ui->editFrameTable->verticalHeader()->setDefaultSectionSize(ui->editFrameTable->height()/ui->editFrameTable->columnCount());
    ui->editFrameTable->horizontalHeader()->sectionResizeMode(QHeaderView::Fixed);
    ui->editFrameTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);

    currentFrame_ = 101;
    currentFrameSize_ = 16;
    populateWithWidgetItems(ui->editFrameTable);
    createNewFrame();

    //make the preview window table the correct size and make this size fixed
    ui->previewWindowTable->horizontalHeader()->setDefaultSectionSize(ui->previewWindowTable->width()/ui->previewWindowTable->rowCount());
    ui->previewWindowTable->verticalHeader()->setDefaultSectionSize(ui->previewWindowTable->height()/ui->previewWindowTable->columnCount());
    ui->previewWindowTable->horizontalHeader()->sectionResizeMode(QHeaderView::Fixed);
    ui->previewWindowTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
    populateWithWidgetItems(ui->previewWindowTable);

    currentPlaybackSpeed_ = 100;
    currentPlaybackFrame_ = 0;
    updatePreview();

    ui->frameSizeComboBox->addItems({"4", "5", "8", "10", "16", "20", "40"});
    ui->frameSizeComboBox->setCurrentIndex(4);

    //set color preview
    const QString setColor("QPushButton { background-color : %1; }");
    ui->currentColorTab->setStyleSheet(setColor.arg(currentColor_.name()));
    ui->currentColorTab->update();

    //set up cursors and tools
    drawCursor_ = QCursor(QCursor(QPixmap(":/cursor/img/cursor_draw.png"), 0, 32));
    fillCursor_ = QCursor(QCursor(QPixmap(":/cursor/img/cursor_paintbucket.png"), 0, 0));
    eraseCursor_ = QCursor(QCursor(QPixmap(":/cursor/img/cursor_eraser.png"), 0, 0));
    rectOffCursor_ = QCursor(QCursor(QPixmap(":/cursor/img/cursor_rect_off.png"), 0, 0));
    rectOnCursor_ = QCursor(QCursor(QPixmap(":/cursor/img/cursor_rect_on.png"), 0, 0));
    circleOffCursor_ = QCursor(QCursor(QPixmap(":/cursor/img/cursor_circle_off.png"), 0, 0));
    circleOnCursor_ = QCursor(QCursor(QPixmap(":/cursor/img/cursor_circle_on.png"), 0, 0));

    //assign cursors to tools
    ui->drawToolButton->setCursor(drawCursor_);
    ui->fillToolButton->setCursor(fillCursor_);
    ui->eraseToolButton->setCursor(eraseCursor_);
    ui->rectToolButton->setCursor(rectOffCursor_);
    ui->circleToolButton->setCursor(circleOffCursor_);

    ui->drawToolButton->setCheckable(true);
    ui->fillToolButton->setCheckable(true);
    ui->eraseToolButton->setCheckable(true);
    ui->rectToolButton->setCheckable(true);
    ui->circleToolButton->setCheckable(true);

    on_drawToolButton_clicked();

    isDrawingShape_ = false;

    //signals for drawing on the frame
    connect(ui->editFrameTable, SIGNAL(cellClicked(int,int)), this, SLOT(onCellClicked(int,int)));
    connect(ui->editFrameTable, SIGNAL(cellEntered(int, int)), this, SLOT(onCellEntered(int,int)));
    connect(ui->alphaSlider, SIGNAL(sliderMoved(int)), this, SLOT(changeAlpha(int)));

    //signals for moving between frames/creating frames/deleting frames
    connect(ui->frameSizeComboBox, SIGNAL(activated(int)), this, SLOT(changeNumberOfPixels(int)));
    connect(ui->newFrame, SIGNAL(clicked()), this, SLOT(createNewFrame()));
    connect(ui->nextFrame, SIGNAL(clicked()), this, SLOT(goToNextFrame()));    connect(ui->previousFrame, SIGNAL(clicked()), this, SLOT(goToPreviousFrame()));
    connect(ui->duplicateFrameButton, SIGNAL(clicked()), this, SLOT(duplicateFrame()));
    connect(ui->undoButton, SIGNAL(clicked()), this, SLOT(displayUndoFrame()));
    connect(ui->redoButton, SIGNAL(clicked()), this, SLOT(displayRedoFrame()));
    connect(ui->deleteFrame, SIGNAL(clicked()), this, SLOT(deleteFrame()));

    //signals for the playback window
    connect(ui->previewSlider, SIGNAL(sliderMoved(int)), this, SLOT(changePlaybackSpeed(int)));

    //signals for save and load and Gif
    connect(ui->actionSave_Project, SIGNAL(triggered()), this, SLOT(saveProject()));
    connect(ui->actionLoad_Project, SIGNAL(triggered()), this, SLOT(loadProject()));
    connect(ui->actionExport_as_GIF, SIGNAL(triggered()), this, SLOT(on_gifButton_clicked()));

    //connections for the model and the view
    connect(this, &View::saveProjectSignal, &model, &Model::saveProject);
    connect(&model, &Model::fileFailedToOpen, this, &View::fileFailedToOpen);
    connect(this, &View::loadProjectSignal, &model, &Model::loadProject);
    connect(&model, &Model::finishLoadingProject, this, &View::finishLoadingProject);
}

/*
 * called when a file fails to open in save or load. displays an error message to the user.
*/
void View::fileFailedToOpen(QString error){
    QMessageBox::information(this, tr("Unable to open file"),
        error);
}

/*
 * changes the value of alpha for the currently selected color based on the slider in the GUI
*/
void View::changeAlpha(int newAlpha){
    currentColor_.setAlpha(newAlpha);
}

/*
 * called when a cell is clicked on, calls other various helper methods depending on which tool is currently selected
*/
void View::onCellClicked(int x, int y){
    saveFramesForUndo();
    framesForRedo_.clear();
    switch(currentTool_){
        case Draw:
            changeCellColor(x, y);
            break;
        case Fill:
            fillCells(x, y);
            break;
        case Erase:
            changeCellColor(x, y);
            break;
        case Rectangle:
            drawRect(x, y);
            break;
        case Circle:
            drawCircle(x, y);
            break;
    }
}

/*
 * called when a cell is entered, calls other various helper methods depending on which tool is currently selected
*/
void View::onCellEntered(int x, int y){
    saveFramesForUndo();
    framesForRedo_.clear();
    switch(currentTool_){
        case Draw:
            changeCellColor(x, y);
            break;
        case Erase:
            changeCellColor(x, y);
            break;
        default:
            break;
    }
}

/*
 * changes the color of the cell that was clicked on
*/
void View::changeCellColor(int a, int b){
    if(currentTool_ == 2){ // if eraser selected set color to white
        ui->editFrameTable->item(a, b)->setBackground(QColor(255,255,255));
    }
    else{
        ui->editFrameTable->item(a, b)->setBackground(currentColor_);
    }
    saveCurrentFrame();
}

/*
 * performs breadth-first search to fill cells with color when using the fill tool
*/
void View::fillCells(int x, int y){
    saveCurrentFrame();

    vector<vector<QColor>> pixels = frames_[currentFrame_]->getPixels();
    QColor startColor = pixels[x][y];

    if (startColor != QColor(currentColor_)){
        // Breadth-first search
        QQueue<std::pair<int, int>> queue;
        std::pair<int, int> start(x, y);
        queue.enqueue(start);

        while (!queue.isEmpty()){
            std::pair<int, int> current = queue.dequeue();
            // If current is the start color, change current to new color and enqueue all current's neighbors
            if(current.first >= 0
                    && current.second >= 0
                    && current.first <= currentFrameSize_ - 1
                    && current.second <= currentFrameSize_ -1
                    && pixels[current.first][current.second] == startColor){
                ui->editFrameTable->item(current.first, current.second)->setBackground(QColor(currentColor_));
                pixels[current.first][current.second] = QColor(0,255,255);

                queue.enqueue(std::pair<int, int>(current.first-1, current.second));
                queue.enqueue(std::pair<int, int>(current.first+1, current.second));
                queue.enqueue(std::pair<int, int>(current.first, current.second-1));
                queue.enqueue(std::pair<int, int>(current.first, current.second+1));
            }
        }
    }
    saveCurrentFrame();
}

/*
 * draws a rectangle with on corner where the user first clicked and the other corner where the user clicks the second time
*/
void View::drawRect(int y, int x){
    if(!isDrawingShape_){
        isDrawingShape_ = true;
        shapeCoords_.first = y;
        shapeCoords_.second = x;
        ui->editFrameTable->setCursor(rectOnCursor_);
    }
    else{
        int currentY = shapeCoords_.first;
        int currentX = shapeCoords_.second;

        ui->editFrameTable->item(currentY, shapeCoords_.second)->setBackground(currentColor_);
        ui->editFrameTable->item(currentY, x)->setBackground(currentColor_);

        while(y - currentY != 0){
            if(y - currentY > 0){
                currentY++;
            }
            else{
                currentY--;
            }
            ui->editFrameTable->item(currentY, shapeCoords_.second)->setBackground(currentColor_);
            ui->editFrameTable->item(currentY, x)->setBackground(currentColor_);
        }

        while(x - currentX != 0){
            if(x - currentX > 0){
                currentX++;
            }
            else{
                currentX--;
            }
            ui->editFrameTable->item(shapeCoords_.first, currentX)->setBackground(currentColor_);
            ui->editFrameTable->item(y, currentX)->setBackground(currentColor_);
        }

        isDrawingShape_ = false;
        ui->editFrameTable->setCursor(rectOffCursor_);
    }
}

/*
 * Draws a circle/ellipse when the circle drawing tool is selected. one side of the circle is at the user's first click,
 * and the opposite side is at the user's second click.
 * Uses Bresenham's circle/ellipse drawing algorithm
 * Implentation gotten from:
 * https://sites.google.com/site/ruslancray/lab/projects/bresenhamscircleellipsedrawingalgorithm/bresenham-s-circle-ellipse-drawing-algorithm
 */
void View::drawCircle(int y, int x){
    if(!isDrawingShape_){
        isDrawingShape_ = true;
        shapeCoords_.first = y;
        shapeCoords_.second = x;
        ui->editFrameTable->setCursor(circleOnCursor_);
    }

    else if(abs(shapeCoords_.first-y)>1 || abs(shapeCoords_.second-x)>1){
        int xc = (x + shapeCoords_.second) / 2;
        int yc = (y + shapeCoords_.first) / 2;
        int width = abs(x - shapeCoords_.second) / 2;
        int height = abs(y - shapeCoords_.first) / 2;
        int a2 = width * width;
        int b2 = height * height;
        int fa2 = 4 * a2, fb2 = 4 * b2;
        int x, y, sigma;

        for(x = 0, y = height, sigma = 2*b2+a2*(1-2*height); b2*x <= a2*y; x++){
            ui->editFrameTable->item(yc + y, xc + x)->setBackground(currentColor_);
            ui->editFrameTable->item(yc + y, xc - x)->setBackground(currentColor_);
            ui->editFrameTable->item(yc - y, xc + x)->setBackground(currentColor_);
            ui->editFrameTable->item(yc - y, xc - x)->setBackground(currentColor_);
            if (sigma >= 0){
                sigma += fa2 * (1 - y);
                y--;
            }
            sigma += b2 * ((4 * x) + 6);
        }

        for(x = width, y = 0, sigma = 2*a2+b2*(1-2*width); a2*y <= b2*x; y++){
            ui->editFrameTable->item(yc + y, xc + x)->setBackground(currentColor_);
            ui->editFrameTable->item(yc + y, xc - x)->setBackground(currentColor_);
            ui->editFrameTable->item(yc - y, xc + x)->setBackground(currentColor_);
            ui->editFrameTable->item(yc - y, xc - x)->setBackground(currentColor_);
            if (sigma >= 0){
                sigma += fb2 * (1 - x);
                x--;
            }
            sigma += a2 * ((4 * y) + 6);
        }
        isDrawingShape_ = false;
        ui->editFrameTable->setCursor(circleOffCursor_);
        saveCurrentFrame();
    }
}

/*
 * populates the table widget with table widget items, which are what allow you to draw on the table
*/
void View::populateWithWidgetItems(QTableWidget* table){
    for(int row = 0; row<table->rowCount(); row++){
        for(int col = 0; col<table->columnCount(); col++){
            table->setItem(row, col, new QTableWidgetItem());
        }
    }
}

/*
 * changes the number of pixels in the frame to the number specified in the combo box
*/
void View::changeNumberOfPixels(int indexInComboBox){
    int previousFrameSize = ui->editFrameTable->rowCount();
    currentFrameSize_ = atoi(ui->frameSizeComboBox->itemText(indexInComboBox).toStdString().c_str());
    ui->editFrameTable->setRowCount(currentFrameSize_);
    ui->editFrameTable->setColumnCount(currentFrameSize_);
    ui->editFrameTable->horizontalHeader()->setDefaultSectionSize((ui->editFrameTable->width()+ui->editFrameTable->rowCount()-1)/ui->editFrameTable->rowCount());
    ui->editFrameTable->verticalHeader()->setDefaultSectionSize((ui->editFrameTable->height()+ui->editFrameTable->columnCount()-1)/ui->editFrameTable->columnCount());
    ui->editFrameTable->horizontalHeader()->sectionResizeMode(QHeaderView::Fixed);
    ui->editFrameTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);

    //put widget items in these new cells
    for(int row=0; row<ui->editFrameTable->rowCount(); row++){
        for(int col = 0; col<ui->editFrameTable->columnCount(); col++){
            if(row>=previousFrameSize || col >= previousFrameSize){
                ui->editFrameTable->setItem(row,col,new QTableWidgetItem());
            }
        }
    }
    //these are here to fix the resize frame color bug. DO NOT CHANGE
    loadFrame(ui->editFrameTable, currentFrame_);
    saveCurrentFrame();
}

/*
 * creates a new frame (places it after the current frame in the animation sequence) and makes that the frame of current focus
*/
void View::createNewFrame(){
    //save current frame if there is one
    if(currentFrame_ < 101){
        saveCurrentFrame();
    }

    if(currentFrame_ == 99){
        QMessageBox tooManyFramesBox;
        tooManyFramesBox.setText("Cannot exceed 100 frames");
        return;
    }

    //create the new frame
    vector<vector<QColor>> pixels;
    for(int row = 0; row < MAX_FRAME_SIZE; row++){
        vector<QColor> temp;
        for(int col = 0; col < MAX_FRAME_SIZE; col++){
            if(row < ui->editFrameTable->rowCount() && col < ui->editFrameTable->columnCount()){ //check if there is a cell there (depends on frame size)
                ui->editFrameTable->item(row, col)->setBackground(QColor(255,255,255));
                temp.push_back(ui->editFrameTable->item(row,col)->background().color());
            }
            else{
                temp.push_back(QColor(255,255,255)); //cell not displayed because frame size is too small at the moment
            }
        }
        pixels.push_back(temp);
    }
    Frame* frame = new Frame(pixels);

    //insert the new frame after the current frame
    vector<Frame*>::iterator it = frames_.begin();
    if(currentFrame_ == 101){ //there is no frame yet
        frames_.push_back(frame);
        currentFrame_ = 0;
    }
    else{
        frames_.insert(it+currentFrame_+1, frame);
        currentFrame_ += 1;
    }
    framesForUndo_.clear();
    framesForRedo_.clear();
    setFrameLabel();
}

/*
 * deletes the current frame. if there is only one frame, presents a blank frame to the user.
*/
void View::deleteFrame(){
    //make sure the user really wants to delete a frame
    QMessageBox deleteFrameBox;
    deleteFrameBox.setText("Are you sure you want to delete?");
    QAbstractButton* pButtonYes = deleteFrameBox.addButton(tr("YES"), QMessageBox::YesRole);
    deleteFrameBox.addButton(tr("NO"), QMessageBox::NoRole);
    deleteFrameBox.exec();

    if(deleteFrameBox.clickedButton()==pButtonYes){
        vector<Frame*>::iterator it = frames_.begin();
        frames_.erase(it+currentFrame_);
        if(currentFrame_ == 0){
            if(frames_.size() == 0){
                currentFrame_ = 101; //default value for when there is no frame
                createNewFrame();
                currentPlaybackFrame_ = 0;
            }
            else{
                currentFrame_ = 0;
                setFrameLabel();
                if(currentPlaybackFrame_ != 0){
                    currentPlaybackFrame_ -= 1;
                }
                loadFrame(ui->editFrameTable, currentFrame_);
            }
        }
        else{
            if(currentPlaybackFrame_ >= currentFrame_){
                currentPlaybackFrame_ -= 1;
            }
            currentFrame_ = currentFrame_ -1;
            setFrameLabel();
            loadFrame(ui->editFrameTable, currentFrame_);
        }
    }
}

/*
 * creates a new frame that is a duplicate of the previous frame
*/
void View::duplicateFrame(){
    vector<vector<QColor>> pixels = frames_[currentFrame_]->getPixels();
    for(int row = 0; row<ui->editFrameTable->rowCount(); row++){
        for(int col = 0; col<ui->editFrameTable->columnCount(); col++){
            pixels[row][col]=ui->editFrameTable->item(row, col)->background().color();
        }
    }
    Frame* frame = new Frame(pixels);

    vector<Frame*>::iterator it = frames_.begin();
    if(currentFrame_ == 101){ //there is no frame yet
        frames_.push_back(frame);
        currentFrame_ = 0;
    }
    else{
        frames_.insert(it+currentFrame_+1, frame);
        currentFrame_ += 1;
    }
    loadFrame(ui->editFrameTable, currentFrame_);
    setFrameLabel();
    saveCurrentFrame();
}

/*
 * saves the current frame before adding a new frame (and switching focus to that) or just advancing to the next
 * or previous frame
*/
void View::saveCurrentFrame(){
    isDrawingShape_ = false;
    if(!(ui->editAllButton->isChecked())){ //if we only edit one frame
        vector<vector<QColor>> pixels = frames_[currentFrame_]->getPixels();
        for(int row = 0; row<ui->editFrameTable->rowCount(); row++){
            for(int col = 0; col<ui->editFrameTable->columnCount(); col++){
                pixels[row][col]=ui->editFrameTable->item(row, col)->background().color();
            }
        }
        frames_[currentFrame_]->saveFrame(pixels);
    }
    else{//if we edit all frames
        vector<vector<QColor>> pixels = frames_[currentFrame_]->getPixels();
        vector<vector<bool>> pixelsChanged;
        for(int row = 0; row<ui->editFrameTable->rowCount(); row++){
            vector<bool> tempPixelsChanged;
            for(int col = 0; col<ui->editFrameTable->columnCount(); col++){
                if(pixels[row][col] != ui->editFrameTable->item(row,col)->background().color()){ //this pixel was updated
                    tempPixelsChanged.push_back(true);
                }
                else{
                    tempPixelsChanged.push_back(false);
                }
                pixels[row][col]=ui->editFrameTable->item(row, col)->background().color();
            }
            pixelsChanged.push_back(tempPixelsChanged);
        }
        frames_[currentFrame_]->saveFrame(pixels); //save for current frame
        //save update for each other frame
        vector<vector<QColor>> otherPixels;
        for(unsigned int i=0; i<frames_.size(); i++){
            if(i != currentFrame_){
                otherPixels = frames_[i]->getPixels();
                for(unsigned int row = 0; row<pixelsChanged.size(); row++){
                    for(unsigned int col=0; col<pixelsChanged[0].size(); col++){
                        if(pixelsChanged[row][col]){
                            otherPixels[row][col] = pixels[row][col];
                        }
                    }
                }
                frames_[i]->saveFrame(otherPixels);
            }
        }
    }
}

/*
 * saves the current frame changes into the FramesForUndo
*/
void View::saveFramesForUndo(){
    vector<vector<QColor>> pixels = frames_[currentFrame_]->getPixels();
    for(int row = 0; row<ui->editFrameTable->rowCount(); row++){
        for(int col = 0; col<ui->editFrameTable->columnCount(); col++){
            pixels[row][col]=ui->editFrameTable->item(row, col)->background().color();
        }
    }
    Frame* frame = new Frame(pixels);
    framesForUndo_.push_back(frame);
}

/*
 * display the undo frame by changing the current frame to the last frame from the framesForUndo
*/
void View::displayUndoFrame(){
    if(framesForUndo_.size()>0){
        saveFramesForRedo();
        vector<vector<QColor>> pixels = framesForUndo_[framesForUndo_.size()-1]->getPixels();
        frames_[currentFrame_]->saveFrame(pixels);
        loadFrame(ui->editFrameTable, currentFrame_);
        framesForUndo_.pop_back();
    }
}

/*
 * saves the current frame changes into the FramesForRedo
*/
void View::saveFramesForRedo(){
    vector<vector<QColor>> pixels = frames_[currentFrame_]->getPixels();
    for(int row = 0; row<ui->editFrameTable->rowCount(); row++){
        for(int col = 0; col<ui->editFrameTable->columnCount(); col++){
            pixels[row][col]=ui->editFrameTable->item(row, col)->background().color();
        }
    }
    Frame* frame = new Frame(pixels);
    framesForRedo_.push_back(frame);
}

/*
 * display the redo frame by changing the current frame to the last frame from the framesForRedo
*/
void View::displayRedoFrame(){
    if(framesForRedo_.size()>0){
        saveFramesForUndo();
        vector<vector<QColor>> pixels = framesForRedo_[framesForRedo_.size()-1]->getPixels();
        frames_[currentFrame_]->saveFrame(pixels);
        loadFrame(ui->editFrameTable, currentFrame_);
        framesForRedo_.pop_back();
    }
}

/*
 * displays the next frame in the frame vector
*/
void View::goToNextFrame(){
    saveCurrentFrame();
    framesForUndo_.clear();

    if(currentFrame_ == frames_.size()-1){
        currentFrame_ = 0;
    }
    else{

        currentFrame_ += 1;
    }
    loadFrame(ui->editFrameTable, currentFrame_);
    setFrameLabel();
}

/*
 * displays the previous frame in the frame vector
*/
void View::goToPreviousFrame(){
    saveCurrentFrame();
    framesForUndo_.clear();

    if(currentFrame_ == 0){
        currentFrame_ = frames_.size()-1;
    }
    else{

        currentFrame_ -= 1;
    }
    loadFrame(ui->editFrameTable, currentFrame_);
    setFrameLabel();
}

/*
 * load the frame at the index currentFrame_ in frames_ into the table
*/
void View::loadFrame(QTableWidget* table, int frameIndex){
    vector<vector<QColor>> pixels = frames_[frameIndex]->getPixels();
    for(int row = 0; row<currentFrameSize_; row++){
        for(int col = 0; col<currentFrameSize_; col++){
            table->item(row, col)->setBackground(pixels[row][col]);
        }
    }
}

/*
 * load the frame at the index currentFrame_ in frames_ into the table
*/
void View::loadPreviewFrame(QTableWidget* table, int frameIndex){
    vector<vector<QColor>> pixels = frames_[frameIndex]->getPixels();
    for(int row = 0; row<currentFrameSize_; row++){
        for(int col = 0; col<currentFrameSize_; col++){
            table->item(row, col)->setBackground(pixels[row][col]);
        }
        for(int col = currentFrameSize_; col<MAX_FRAME_SIZE; col++){
            QColor item= pixels[row][col];
            QColor newColor; //create new qcolor from rgba
            newColor.setRgb( 192,192,192,  item.alpha());
            table->item(row, col)->setBackground(newColor);
        }
    }
    for(int row = currentFrameSize_; row<MAX_FRAME_SIZE; row++){
        for(int col = 0; col<MAX_FRAME_SIZE; col++){
            QColor item= pixels[row][col];
            QColor newColor; //create new qcolor from rgba
            newColor.setRgb( 192,192,192,   item.alpha());
            table->item(row, col)->setBackground(newColor);
        }
    }

}

/*
 * updates the frame label in the view (says "Frame ___ out of ___" )
*/
void View::setFrameLabel(){
    ui->frameLabel->setText("Frame " + QString::number(currentFrame_+1) + " out of " + QString::number(frames_.size()));
}

/*
 * updates the frame displayed in the preview window
*/
void View::updatePreview(){
    loadPreviewFrame(ui->previewWindowTable, currentPlaybackFrame_);
    if(currentPlaybackFrame_ == frames_.size()-1){
        currentPlaybackFrame_ = 0;
    }
    else{
        currentPlaybackFrame_ += 1;
    }
    playbackTimer_.singleShot(currentPlaybackSpeed_, this, &View::updatePreview);
}

/*
 * updates the playback speed in the animation preview window when the slider is slid
*/
void View::changePlaybackSpeed(int newFPS){
    currentPlaybackSpeed_ = 1000/newFPS;
}

/*
 * called when the user clicks on the draw tool (pencil). sets the current tool to Draw and assigns the appropriate cursor.
*/
void View::on_drawToolButton_clicked(){
    currentTool_ = Draw;
    checkButton(Draw);
    ui->editFrameTable->setCursor(drawCursor_);
}

/*
 * called when the user clicks on the fill tool. sets Fill as the currently selected tool and assigns the appropriate cursor.
*/
void View::on_fillToolButton_clicked(){
    currentTool_ = Fill;
    checkButton(Fill);
    ui->editFrameTable->setCursor(fillCursor_);
}

/*
 * called when the user clicks on the erase tool, sets the curernt too to Erase and the cursor to the appropriate cursor.
*/
void View::on_eraseToolButton_clicked(){
    currentTool_ = Erase;
    checkButton(Erase);
    ui->editFrameTable->setCursor(eraseCursor_);
}

/*
 * called when the user clicks on the draw rectangle tool and sets the currently selected tool to Rectangle. since
 * the user is not currently drawing a shape, isDrawingShape is set to false.
*/
void View::on_rectToolButton_clicked(){
    currentTool_ = Rectangle;
    checkButton(Rectangle);
    ui->editFrameTable->setCursor(rectOffCursor_);
    isDrawingShape_ = false;
}

/*
 * called when the user clicks on the draw circle tool and sets the currently selected tool to Circle. since the
 * user is not currrently drawing a circle, isDrawingShape is false.
*/
void View::on_circleToolButton_clicked(){
    currentTool_ = Circle;
    checkButton(Circle);
    ui->editFrameTable->setCursor(circleOffCursor_);
    isDrawingShape_ = false;
}

/*
 * highlights the button that is associated with the tool the user is currently using
*/
void View::checkButton(Tool t){
    ui->drawToolButton->setChecked(false);
    ui->fillToolButton->setChecked(false);
    ui->eraseToolButton->setChecked(false);
    ui->rectToolButton->setChecked(false);
    ui->circleToolButton->setChecked(false);

    switch (t) {
        case Draw:
            ui->drawToolButton->setChecked(true);
            break;
        case Fill:
            ui->fillToolButton->setChecked(true);
            break;
        case Erase:
            ui->eraseToolButton->setChecked(true);
            break;
        case Rectangle:
            ui->rectToolButton->setChecked(true);
            break;
        case Circle:
            ui->circleToolButton->setChecked(true);
            break;
    }
}

/*
 * called when the user wishes to save a project. saves the project to a .ssp file with the specifications
 * listed in the assignment.
*/
void View::saveProject(){
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Sprite"), "",
        tr("Sprite (*.ssp);;All Files (*)"));
    emit saveProjectSignal(frames_, currentFrameSize_, fileName);
}

/*
 * called when the user wishes to load a new project. reads in the .ssp file the user selects and
 * displays the new project in place of the user's current project.
*/
void View::loadProject(){
    currentFrame_ =0; //index of the current frame in frames_
    currentPlaybackFrame_=0; //the index of the frame being played back in frames_

    QString fileName = QFileDialog::getOpenFileName(this,
           tr("Open Sprite"), "",
           tr("Sprite (*.ssp);;All Files (*)"));
    emit loadProjectSignal(fileName);
}

/*
 * called after the model loads the frame, sets all of the appropriate variables in the view
*/
void View::finishLoadingProject(vector<Frame*> newFrames){
    frames_=newFrames;
    currentFrame_=0;
    setFrameLabel();
    loadFrame(ui->editFrameTable, currentFrame_);
}

/*
 * called when the user creates a gif. uses the gif.h header file to create a gif.
*/
void View::on_gifButton_clicked(){
    GifWriter writer;
    int gifSize = currentFrameSize_ * currentFrameSize_*4;
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Create GIF"), "",
        tr("Sprite (*.gif);;All Files (*)"));
    GifBegin(&writer, fileName.toLocal8Bit().constData(), currentFrameSize_, currentFrameSize_, true);
    //for each frame, convert to QImage
    for(Frame* a : frames_){

        vector<vector<QColor>> pixels = a->getPixels();
        uint8_t *gifImage = new uint8_t[gifSize];
        //fill all pixels of image with default white
        for(int row = 0; row < currentFrameSize_; row++){
            for(int col = 0; col < currentFrameSize_; col++){
                int colorIndex = 0;
        //single index pixel from 2d vector into gifImage,
        //seperate each (RGBA) color and put in its 4 bit index
                QColor color = pixels[row][col];
                gifImage[(row * currentFrameSize_ + col) * 4] = color.red();
                colorIndex++;
                gifImage[(row * currentFrameSize_ + col) * 4 + colorIndex] = color.green();
                colorIndex++;
                gifImage[(row * currentFrameSize_ + col) * 4 + colorIndex] = color.blue();
                colorIndex++;
                gifImage[(row * currentFrameSize_ + col) * 4 + colorIndex] = color.alpha();

            }
        }

        GifWriteFrame(&writer,gifImage, currentFrameSize_, currentFrameSize_, true);
    }
    GifEnd(&writer);
}


/*
 * When Color button is pressed, open Color Dialog,
 * set color member variable to selected color to draw with,
 * tell color window to show color
*/
void View::on_currentColorTab_clicked(){
    currentColor_ = QColorDialog::getColor();
    currentColor_.setAlpha(ui->alphaSlider->value());
    const QString setColor("QPushButton { background-color : %1; }");
    ui->currentColorTab->setStyleSheet(setColor.arg(currentColor_.name()));
    ui->currentColorTab->update();

}

/*
 * When Color button is pressed, open Color Dialog,
 * set color member variable to selected color to draw with,
 * tell color window to show color
*/
void View::on_currentColorTab_pressed(){

    currentColor_ = QColorDialog::getColor();
    currentColor_.setAlpha(ui->alphaSlider->value());
    const QString setColor("QPushButton { background-color : %1; }");
    ui->currentColorTab->setStyleSheet(setColor.arg(currentColor_.name()));
    ui->currentColorTab->update();

}

/*
 * disposes of each frame in the frames vector (not sure if this is necessary)
*/
View::~View(){
    for(unsigned int i=0; i<frames_.size(); i++){
        delete frames_[i];
    }
    delete ui;
}
