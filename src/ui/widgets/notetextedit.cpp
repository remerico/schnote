#include "notetextedit.h"


NoteTextEdit::NoteTextEdit(QWidget *parent) : QPlainTextEdit(parent) {
    //setFontPointSize(12);
    //setFont(QFont("Courier", 10));
}

void NoteTextEdit::paintEvent(QPaintEvent *e) {

#define AYAW_GUMANA

#ifdef AYAW_GUMANA

    QPlainTextEdit::paintEvent(e);

#else

    QTextEdit::paintEvent(e);

    int height = fontMetrics().lineSpacing();

    QPainter p(viewport());

    p.drawRect(cursorRect());

    p.setPen(Qt::gray);    

    for(int i = height + 3; i < qMax(document()->size().height(), (qreal)this->height()); i += height) {
        p.drawLine(2, i, viewport()->width() - 4, i);
    }

#endif

}
