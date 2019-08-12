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

// Qt Libraries
#include <QLineEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QDialog>

// Local Libraries
#include "extendedlineedit.h"
#include "dialogtexteditor.h"


ExtendedLineEdit::ExtendedLineEdit(QWidget* parent) : QWidget(parent)
{
    // create the layout
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setMargin(0);
    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);

    // add the editor
    QLineEdit* editor = new QLineEdit("", this);
    editor->setObjectName("editor_text");
    gridLayout->addWidget(editor, 0, 0, 1, 1);
    connect(editor, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));

    // add buttons
    QPushButton* btn_edit = new QPushButton("...", this);
    btn_edit->setObjectName("btnEdit");
    btn_edit->setFixedWidth(20);
    gridLayout->addWidget(btn_edit, 0, 3, 1, 1);
    connect(btn_edit, SIGNAL(clicked()), this, SLOT(editText()));


    QString ss = "QPushButton{background-color: Silver; border: 0px solid silver;min-height: 16px;}";
    btn_edit->setStyleSheet(ss);
}

void ExtendedLineEdit::editText()
{
    DialogTextEditor dlg;
    dlg.setText(this->text());
    if(dlg.exec())
    {
        this->setText(dlg.text());
        emit editingFinished();
    }
}

QString ExtendedLineEdit::text()
{
    QLineEdit* editor = this->findChild<QLineEdit*>("editor_text");
    return editor->text();
}

void ExtendedLineEdit::setText(const QString& text)
{
    this->findChild<QLineEdit*>("editor_text")->setText(text);
}
