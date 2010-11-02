#ifndef NOTETEXTEDIT_H
#define NOTETEXTEDIT_H

#include <QtGui>

class NoteTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit NoteTextEdit(QWidget *parent = 0);

private:
    void paintEvent(QPaintEvent *e);

signals:

public slots:

};

#endif // NOTETEXTEDIT_H
