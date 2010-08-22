#include "notewindow.h"

NoteWindow::NoteWindow(QWidget *parent) : QMainWindow(parent) {
    setupGui();
}


NoteWindow::NoteWindow(Note note, QWidget *parent) : QMainWindow(parent) {
    setupGui();

    _note = note;
    textEdit->setText(note.data);
    isTextChanged = false;
}

void NoteWindow::setupGui() {

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation);
#endif

    editRow = -1;
    isTextChanged = false;
    resize(320, 240);
    textEdit = new NoteTextEdit(this);

    setCentralWidget(textEdit);

    QMenu* menu = menuBar()->addMenu("&Menu");
    menu->addAction("Undo", textEdit, SLOT(undo()), QKeySequence::Undo);
    menu->addAction("Redo", textEdit, SLOT(redo()), QKeySequence::Redo);

    connect(textEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));

}

void NoteWindow::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
        close();
    }
}

void NoteWindow::closeEvent(QCloseEvent *) {
    if (editRow >= 0) {
        // Edit note
        _note.setNote(textEdit->toPlainText());
        if (isTextChanged) emit noteSaved(_note, editRow);
    }
    else {
        // Create new note
        if (isTextChanged) emit noteSaved(Note(textEdit->toPlainText()));
    }
}

void NoteWindow::textChanged() {
    isTextChanged = true;
}
