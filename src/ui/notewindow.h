#ifndef NOTEWINDOW_H
#define NOTEWINDOW_H

#include <QtGui>
#include "widgets/notetextedit.h"
#include "../data/notelistmodel.h"

class NoteWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit NoteWindow(QWidget *parent = 0);
    NoteWindow(Note note, QWidget *parent = 0);

    inline int getEditRow() { return editRow; }
    inline void setEditRow(int row) { editRow = row; }


private:
    void keyPressEvent(QKeyEvent *);
    void closeEvent(QCloseEvent *);

    int editRow;
    bool isTextChanged;

    void setupGui();

private:
    Note _note;
    NoteTextEdit* textEdit;

signals:
    void noteSaved(const Note& note, int row = -1);

public slots:
    void textChanged();

};

#endif // NOTEWINDOW_H
