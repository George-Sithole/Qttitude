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
#include <QStandardItemModel>
#include <QLineEdit>
#include <QPainter>
#include <QStyleOptionViewItem>

// Local Libraries
#include "stylesheeteditoritemdelegate.h"
#include "extendedlineedit.h"
//#include "stylesheeteditor/coloreditorwidget.h"


using namespace std;


StyleSheetEditorItemDelegate::StyleSheetEditorItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

QWidget* StyleSheetEditorItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    Q_UNUSED(option)

//    if(index.column() == 1)
//    {
//        ColorEditorWidget* editor = new ColorEditorWidget(parent);
//        return editor;
//    }

//    // if all 'else fails' default editor
//    return QStyledItemDelegate::createEditor(parent, option, index);

//    QLineEdit* text_editor = new QLineEdit(parent);
    ExtendedLineEdit* text_editor = new ExtendedLineEdit(parent);
//    text_editor->setFrame(false);
    return text_editor;
}

void StyleSheetEditorItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
//    if(index.column() == 1)
//    {
//        ColorEditorWidget *editor = qobject_cast<ColorEditorWidget *>(editor);
//    }
//    else
//    {
//        QStyledItemDelegate::setEditorData(editor, index);
//    }

//    QLineEdit* text_editor = qobject_cast<QLineEdit*>(editor);
    ExtendedLineEdit* text_editor = static_cast<ExtendedLineEdit*>(editor);
    text_editor->setText(index.data().toString());
}

void StyleSheetEditorItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
//    if(index.column() == 1)
//    {
//        ColorEditorWidget *editor = qobject_cast<ColorEditorWidget *>(editor);
////        model->setData(index, qVariantFromValue<PAbstractProperty>(type));
//    }
//    else
//    {
//        QStyledItemDelegate::setModelData(editor, model, index);
//    }

//    QLineEdit* text_editor = qobject_cast<QLineEdit*>(editor);
    ExtendedLineEdit* text_editor = static_cast<ExtendedLineEdit*>(editor);
    model->setData(index, text_editor->text());// qVariantFromValue<PAbstractProperty>(type));
}

void StyleSheetEditorItemDelegate::commitAndCloseEditor()
{
//    if(qobject_cast<QLineEdit*>(sender()))
//    {
//        QLineEdit* editor = qobject_cast<QLineEdit*>(sender());
//        emit commitData(editor);
//        emit closeEditor(editor);
//    }
    if(static_cast<ExtendedLineEdit*>(sender()))
    {
        ExtendedLineEdit* editor = static_cast<ExtendedLineEdit*>(sender());
        emit commitData(editor);
        emit closeEditor(editor);
    }
}

void StyleSheetEditorItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    painter->save();

    if(index.column() == 1)
    {
        QString text = index.data().toString();

        // test if the text is a color. the if tests are split for efficiency
        bool draw_color_box = false;
        if(text.trimmed().left(1) == "#")
            draw_color_box = true;
        else if(QColor::colorNames().contains(text, Qt::CaseInsensitive))
            draw_color_box = true;

        if(draw_color_box)
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(text));
            int w = option.rect.width();
            int h = option.rect.height();
            painter->drawRect(option.rect.x() + w - h - 1, option.rect.y(), h - 1, h - 1);
        }
    }

    painter->restore();
}
