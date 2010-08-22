#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "../data/notelistmodel.h"
#include "widgets/noteitemdelegate.h"
#include "../simplenote/api.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    NoteListModel *noteModel;

    QDialogButtonBox* buttonBox;
    SimpleNote::Api api;

    void setupUiExtra();

    void showSearch(bool display);

    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void resizeEvent(QResizeEvent *);


public slots:
    void selectionChanged(QItemSelection,QItemSelection);
    void doubleClicked(const QModelIndex & index);
    void createButtonBox(Qt::Orientation orientation);

    void createNewNote();
    void deleteNote();
    void clearSearch();
    void syncNotes();

    void showSettings();

    void receivedNoteIndices(NoteIndexHash indexList);
    void receivedNotes(const NoteList& noteList);
};

#endif // MAINWINDOW_H
