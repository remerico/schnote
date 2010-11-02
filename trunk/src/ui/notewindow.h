#ifndef NOTEWINDOW_H
#define NOTEWINDOW_H

#include <QtGui>
#include "widgets/notetextedit.h"
#include "../data/notelistmodel.h"
#include "mainwindow.h"

class NoteWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit NoteWindow(MainWindow *parent = 0);
    NoteWindow(Note note, MainWindow *parent = 0);


private:
    void keyPressEvent(QKeyEvent *);
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent *);

    bool isTextChanged;

    void setupGui();

private:
    NoteListModel* _noteModel;
    Note _note;
    NoteTextEdit* textEdit;    
    QWidget* centralWidget;
    QGridLayout* gridLayout;
    QDialogButtonBox* buttonBox;
    QLineEdit* tags;

public slots:
    void textChanged();
    void createButtonBox(Qt::Orientation orientation);

    void saveNote();
    void deleteNote();

};

#endif // NOTEWINDOW_H
