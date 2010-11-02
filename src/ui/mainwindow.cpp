#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../data/notedb.h"
#include "settingswindow.h"

#include "../simplenote/api.h"
#include "../data/settings.h"
#include <QtDebug>

#include "notewindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUiExtra();

    //TODO: Enable auto-sync
    //connect(NoteDB, SIGNAL(noteAdded(Note)), &api, SLOT(createNote(Note)));
    //connect(NoteDB, SIGNAL(noteUpdated(Note)), &api, SLOT(updateNote(Note)));
    //connect(NoteDB, SIGNAL(noteKeyDeleted(QString)), &api, SLOT(deleteNote(QString)));

    api.setAccount(Settings->getAccount());
    api.setProxy(Settings->getProxy());
    connect(Settings, SIGNAL(accountChanged(SimpleNote::Account)), &api, SLOT(setAccount(SimpleNote::Account)));
    connect(Settings, SIGNAL(proxyChanged(CurlProxy)), &api, SLOT(setProxy(CurlProxy)));

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUiExtra() {

    // Set Maemo stacked windows and auto orientation mode
    #ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5StackedWindow);
        setAttribute(Qt::WA_Maemo5AutoOrientation);
    #endif

    // Create menu
    QMenu* menu = menuBar()->addMenu("&Menu");
    menu->addAction("Settings", this, SLOT(showSettings()));
    menu->addAction("Sync");

    // Create action buttons
    buttonBox = new QDialogButtonBox(this);
    QPushButton* createButton = buttonBox->addButton("Add", QDialogButtonBox::ActionRole);
    QPushButton* deleteButton = buttonBox->addButton("Delete", QDialogButtonBox::ActionRole);
    QPushButton* syncButton = buttonBox->addButton("Sync", QDialogButtonBox::ActionRole);
    connect(createButton, SIGNAL(clicked()), this, SLOT(createNewNote()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteNote()));

    // Setup icons
    createButton->setIcon(QIcon(":/icons/add.png"));
    syncButton->setIcon(QIcon(":/icons/refresh.png"));


    // Setup Note list
    noteModel = new NoteListModel(this);
    ui->listView->setModel(noteModel);
    ui->listView->setItemDelegate(new NoteItemDelegate(this));
    ui->listView->setFocusPolicy(Qt::NoFocus);

    // Connect slots
    connect(ui->listView, SIGNAL(clicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));
    connect(syncButton, SIGNAL(clicked()), this, SLOT(syncNotes()));
    connect(ui->clearSearch, SIGNAL(clicked()), this, SLOT(clearSearch()));
    connect(ui->searchInput, SIGNAL(textChanged(QString)), this, SLOT(search(QString)));


    qRegisterMetaType<NoteIndexHash>("NoteIndexHash");
    qRegisterMetaType<NoteList>("NoteList");
    qRegisterMetaType<NoteIndexHash>("NoteIndexList");

    connect(&api, SIGNAL(syncNotesFinished()), noteModel, SLOT(refreshData()));

    ui->searchWidget->hide();

}

void MainWindow::keyReleaseEvent(QKeyEvent *e) {

    if (e->key() == Qt::Key_Escape) {
        if (ui->searchInput->hasFocus()) {
            clearSearch();
        }
    }
    else if (!ui->searchInput->hasFocus() && e->key() != Qt::Key_Backspace) {
        showSearch(true);
        ui->searchInput->setFocus(Qt::PopupFocusReason);
        ui->searchInput->setText(ui->searchInput->text() + e->text());
    }

}

void MainWindow::keyPressEvent(QKeyEvent *e) {

    if (e->key() == Qt::Key_Escape) {
        if (!ui->searchInput->hasFocus()) {
            close();
        }
    }

    else QMainWindow::keyPressEvent(e);

}

void MainWindow::resizeEvent(QResizeEvent *e) {
    createButtonBox( (e->size().width() > e->size().height()) ? Qt::Vertical : Qt::Horizontal );
}

void MainWindow::doubleClicked(const QModelIndex & index) {

    ui->listView->selectionModel()->clear();

    Note note = noteModel->noteData(index);

    qDebug() << "Edit note key: " << note.index.key;

    NoteWindow* noteWindow = new NoteWindow(note, this);
    noteWindow->setWindowTitle(note.index.title);

    noteWindow->show();

}

void MainWindow::createButtonBox(Qt::Orientation orientation) {

    ui->gridLayout->removeWidget(buttonBox);

    buttonBox->setOrientation(orientation);

    if (buttonBox->orientation() == Qt::Horizontal) {
        ui->gridLayout->addWidget(buttonBox, 2, 0);
    }
    else if (buttonBox->orientation() == Qt::Vertical) {
        ui->gridLayout->addWidget(buttonBox, 0, 1);
    }

}

void MainWindow::createNewNote() {
    NoteWindow* noteWindow = new NoteWindow(Note(), this);
    noteWindow->show();
}

void MainWindow::deleteNote() {

    if (ui->listView->selectionModel()->selectedIndexes().count() > 0) {
        if (QMessageBox::question(this, "Delete Note", "Delete this note?", QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok) {
            qDebug() << "Remove note row: " << ui->listView->selectionModel()->selectedIndexes().at(0).row();
            noteModel->removeNote(ui->listView->selectionModel()->selectedIndexes().at(0).row());
        }
    }

}

void MainWindow::showSearch(bool display) {
    if (display) ui->searchWidget->show();
    else ui->searchWidget->hide();
}

void MainWindow::clearSearch() {
    ui->searchInput->clear();
    showSearch(false);
    ui->listView->setFocus();
}

void MainWindow::showSettings() {

    settingswindow* settings = new settingswindow(this);
    settings->setModal(true);
    connect(Settings, SIGNAL(accountChanged(SimpleNote::Account)), &api, SLOT(setAccount(SimpleNote::Account)));

    settings->show();

}

void MainWindow::syncNotes() {

    //api.syncNotes();
    api.getNoteIndex();

}

void MainWindow::search(const QString &search) {

    noteModel->search(search);

    if (search.isEmpty()) {
        showSearch(false);
        ui->listView->setFocus();
    }

}

void MainWindow::on_listView_pressed(QModelIndex index)
{
    ui->listView->selectionModel()->select(index, QItemSelectionModel::Select);
    //qDebug() << "pressed: " << index.row();
}
