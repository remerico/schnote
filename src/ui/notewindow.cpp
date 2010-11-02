#include "notewindow.h"

#ifdef Q_WS_MAEMO_5
#include <QAbstractKineticScroller>
#endif

NoteWindow::NoteWindow(MainWindow *parent) : QMainWindow(parent) {
    _noteModel = parent->getNoteModel();
    setupGui();
}


NoteWindow::NoteWindow(Note note, MainWindow *parent) : QMainWindow(parent) {
    setupGui();

    _note = note;
    _noteModel = parent->getNoteModel();

    textEdit->setPlainText(note.content);
    isTextChanged = false;
}

void NoteWindow::setupGui() {

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation);
#else
    resize(320, 240);
#endif

    isTextChanged = false;

    centralWidget = new QWidget(this);
    gridLayout = new QGridLayout(centralWidget);
    gridLayout->setMargin(0);
    setCentralWidget(centralWidget);

    textEdit = new NoteTextEdit(this);
    gridLayout->addWidget(textEdit);
    connect(textEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));

    tags = new QLineEdit(this);
    gridLayout->addWidget(tags);


    buttonBox = new QDialogButtonBox;
    QPushButton *saveButton = buttonBox->addButton(tr("Save"), QDialogButtonBox::ActionRole);
    QPushButton *deleteButton = buttonBox->addButton(tr("Delete"), QDialogButtonBox::ActionRole);
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveNote()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteNote()));


#ifdef Q_WS_MAEMO_5
    QAbstractKineticScroller *scroller = textEdit->property("kineticScroller").value<QAbstractKineticScroller *>();
    if (scroller) scroller->setEnabled(false);
#endif


    QMenu* menu = menuBar()->addMenu("&Menu");
    menu->addAction("Undo", textEdit, SLOT(undo()), QKeySequence::Undo);
    menu->addAction("Redo", textEdit, SLOT(redo()), QKeySequence::Redo);


}

void NoteWindow::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
        close();
    }
}

void NoteWindow::closeEvent(QCloseEvent *) {
    saveNote();
}

void NoteWindow::textChanged() {
    isTextChanged = true;
}

void NoteWindow::createButtonBox(Qt::Orientation orientation) {

    gridLayout->removeWidget(buttonBox);

    buttonBox->setOrientation(orientation);

    if (buttonBox->orientation() == Qt::Horizontal) {
        gridLayout->addWidget(buttonBox, 1, 0);
    }
    else if (buttonBox->orientation() == Qt::Vertical) {
        gridLayout->addWidget(buttonBox, 0, 1);
    }

}

void NoteWindow::resizeEvent(QResizeEvent *e) {
    createButtonBox( (e->size().width() > e->size().height()) ? Qt::Vertical : Qt::Horizontal );
}

void NoteWindow::deleteNote() {

    if (!_note.content.isEmpty() || !textEdit->toPlainText().isEmpty()) {

        if (QMessageBox::question(this, tr("Delete Note"), tr("Delete this note?"), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok) {
            if (_note.index.id >= 0) _noteModel->removeNoteById(_note.index.id);
            this->close();
        }

    }
    else {
        this->close();
    }

}

void NoteWindow::saveNote() {

    if (isTextChanged) {

        // Edit note
        if (_note.index.id >= 0) {
            qDebug() << "Editing note" << _note.index.id << "...";
            _note.setNote(textEdit->toPlainText());
            _noteModel->editNote(_note);
        }

        // Create new note
        else {
            qDebug() << "Appending new note...";
            _note = Note(textEdit->toPlainText());
            _note.index.id = _noteModel->appendNote(_note);
        }

        isTextChanged = false;
    }

}
