/****************************************************************************
**
** Copyright (C) 2019 George Sithole
** Contact: http://www.geovariant.com/qttitude/
**
** This is free software distributed under the terms of the GNU General Public License, GPL v3.
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Qttitude nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

// C/C++ Libraries
#include <iostream>

// Qt Libraries
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QMimeData>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMessageBox>

// Local Libraries
#include "texteditor.h"


using namespace std;


TextEditor::TextEditor(QWidget* parent) : QTextEdit(parent), m_completer(nullptr)
{
    this->setAcceptDrops(true);
}

void TextEditor::setCompleter(QCompleter* completer)
{
    if (m_completer)
    {
        QObject::disconnect(m_completer, nullptr, this, nullptr);
    }

    m_completer = completer;

    if(!m_completer)
        return;

    m_completer->setWidget(this);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(m_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

QCompleter* TextEditor::completer() const
{
    return m_completer;
}

void TextEditor::insertCompletion(const QString& completion)
{
    if (m_completer->widget() != this)
        return;

    QTextCursor tc = textCursor();
    int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString TextEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void TextEditor::focusInEvent(QFocusEvent* event)
{
    if (m_completer)
        m_completer->setWidget(this);

    QTextEdit::focusInEvent(event);
}

void TextEditor::keyPressEvent(QKeyEvent* event)
{
    // if the completer is visible ...
    if(m_completer && m_completer->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (event->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            event->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }

    // if CTRL+E was pressed ...
    bool isShortcut = ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_E);
    if (!m_completer || !isShortcut) // do not process the shortcut when we have a completer
    {
        QTextEdit::keyPressEvent(event);
    }

    // if CTRL or Shift keys were pressed ...
    const bool ctrlOrShift = event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!m_completer || (ctrlOrShift && event->text().isEmpty()))
    {
        return;
    }

    // special characters
    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    // hide the completer if it is not a short cut, or has less than n characters or has special characters
    if (!isShortcut && (hasModifier || event->text().isEmpty()|| completionPrefix.length() < 3
                        || eow.contains(event->text().right(1))))
    {
        m_completer->popup()->hide();
        return;
    }

    // pop up the completer
    if (completionPrefix != m_completer->completionPrefix())
    {
        m_completer->setCompletionPrefix(completionPrefix);
        m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
    }
    QRect cr = this->cursorRect();
    cr.setWidth(m_completer->popup()->sizeHintForColumn(0)
                + m_completer->popup()->verticalScrollBar()->sizeHint().width());
    m_completer->complete(cr); // popup it up!
}


void TextEditor::dragEnterEvent(QDragEnterEvent* e)
{
    qDebug() << "void CustomTextEdit::dragEnterEvent(QDragEnterEvent* e)";
    qDebug() << "e->mimeData()->hasText() is" << e->mimeData()->hasText();

    const QMimeData *mimeData = e->mimeData();
    QStringList mimeFormats = mimeData->formats();
    foreach(QString format, mimeFormats)
      qDebug() << format;


    qDebug() << "";


//    QByteArray encoded = mimeData->data("application/x-qabstractitemmodeldatalist");
//    QDataStream stream(&encoded, QIODevice::ReadOnly);

//    while (!stream.atEnd())
//    {
//        int row, col;
//        QMap<int,  QVariant> roleDataMap; // map indexed by Qt::Role
//        stream >> row >> col >> roleDataMap;

//        for(auto v: roleDataMap)
//        {
//            qDebug() << v;
//            qDebug() << row << "/" << col;
//            qDebug() << v.toString();
//        }
//    }

    QTextEdit::dragEnterEvent(e);
}


//void TextEditor::dropEvent(QDropEvent *event)
//{
//    if(event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
//    {
//        QTreeView* treeview = dynamic_cast<QTreeView*>(event->source());


//        QByteArray itemData = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
//        QDataStream stream(&itemData, QIODevice::ReadOnly);

//        int row, col;
//        QMap<int, QVariant> v;
//        stream >> row >> col >> v;

//        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(treeview->model());
//        QStandardItem *item = model->item(row, 0); //tree->model()-> tree->topLevelItem(row);

//        QModelIndex index = treeview->selectionModel()->selectedIndexes()[0];

//        //cout << index.data().toString().toStdString() << endl;
//    }

////    QPoint p = event->pos(); //get position of drop
////    QTextCursor t(textCursor()); //create a cursor for QTextEdit
////    t.setPos(&p);  //try convert QPoint to QTextCursor to compare with position stored in startPos variable - ERROR

////if dropCursorPosition <  startPos then t = endOfDocument
////if dropCursorPosition >= startPos then t remains the same


////    p = t.pos();  //convert the manipulated cursor position to QPoint  - ERROR
////    QDropEvent drop(p,event->dropAction(), event->mimeData(), event->mouseButtons(), event->keyboardModifiers(), event->type());
////    QTextEdit::dropEvent(&drop); // Call the parent function w/ the modified event

//    QTextEdit::dropEvent(event); // Call the parent function w/ the modified event
//}

bool TextEditor::canInsertFromMimeData(const QMimeData* source) const
{
    return true;
//    return source->hasImage() || source->hasUrls() ||
//        QTextEdit::canInsertFromMimeData(source);
}

void TextEditor::insertFromMimeData(const QMimeData* source)
{
    if(source->hasFormat("application/vnd.text.list"))
    {
        QByteArray itemData = source->data("application/vnd.text.list");
        QDataStream stream(&itemData, QIODevice::ReadOnly);

        QStringList v;
        stream >> v;

        QTextCursor cursor = this->textCursor();
        QStringList text_list;
        for(QString vv: v)
        {
            text_list << vv;
        }

        if(v.count() == 1)
        {
            cursor.insertText(text_list.first());
            cursor.insertText("\n{\n\n}\n");
            cursor.insertBlock();
        }
        else if(v.count() > 1)
        {
            QMessageBox msgBox;
            msgBox.setText("Writing Stylesheet");
            msgBox.setInformativeText("Do you want to seperate the widget stylesheets");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            int result = msgBox.exec();

            switch (result)
            {
            case QMessageBox::Yes:
                for(QString text: text_list)
                {
                    cursor.insertText(text);
                    cursor.insertText("\n{\n\n}\n");
                    cursor.insertBlock();
                }
                break;
            case QMessageBox::No:
                QString text = text_list.join(",\n");
                cursor.insertText(text);
                cursor.insertText("\n{\n\n}\n");
                cursor.insertBlock();
                break;
            }
        }
    }
    else if (source->hasImage())
    {
        // static int i = 1;
        // QUrl url(QString("dropped_image_%1").arg(i++));
        // dropImage(url, qvariant_cast<QImage>(source->imageData()));
    }
    else if (source->hasUrls())
    {
        // foreach (QUrl url, source->urls())
        // {
        //    QFileInfo info(url.toLocalFile());
        //    if (QImageReader::supportedImageFormats().contains(info.suffix().toLower().toLatin1()))
        //        dropImage(url, QImage(info.filePath()));
        //    else
        //        dropTextFile(url);
        //}
    }
    else
    {
        QTextEdit::insertFromMimeData(source);
    }
}
