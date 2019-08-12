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
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QColorDialog>
#include <QLineEdit>
#include <QEvent>
#include <QRegExpValidator>

// Local Libraries
#include "coloreditorwidget.h"


using namespace std;


ColorEditorWidget::ColorEditorWidget(QWidget* parent) : QWidget(parent)
{
    this->setStyleSheet("background-color: white");

    // create the grid layout
    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->setSpacing(1);

    // add the line edit widget
    m_line_edit = new QLineEdit(QString("%0").arg(m_color.name()));
    QRegExp rx("#[A-Fa-f0-9]{6,8}"); // e.g. matches #123456, #1234567 or #12345678
    QValidator *validator = new QRegExpValidator(rx, this);
    m_line_edit->setValidator(validator);
    m_line_edit->installEventFilter(this);
    connect(m_line_edit, SIGNAL(textEdited(QString)), this, SLOT(processTextEdited(QString)));
    layout->addWidget(m_line_edit, 0, 0, 1, 1);

    // add buttons
    m_btn = new QPushButton("...", this);
    m_btn->setFixedWidth(20);
    layout->addWidget(m_btn, 0, 1, 1, 1);
    connect(m_btn, SIGNAL(clicked()), this, SLOT(selectColor()));
}

QColor ColorEditorWidget::color()
{
    return m_color;
}

void ColorEditorWidget::setColor(QColor c)
{
    // set color
    m_color = c;

    // set line edit color
    if(m_line_edit != Q_NULLPTR)
    {
        m_line_edit->setText(m_color.name());
    }

    // set button color
    if(m_btn != Q_NULLPTR)
    {
        m_btn->setStyleSheet(QString("background-color: %0;").arg(m_color.name()));
        m_btn->setAutoFillBackground(true);
    }
    else
    {
        cout << "Button doesn't exist" << endl;
    }
}

void ColorEditorWidget::selectColor()
{
    QColor c = QColorDialog::getColor(m_color, this, tr("Select Color"), QColorDialog::ShowAlphaChannel);
    if(c.isValid())
    {
        this->setColor(c);
    }
}

void ColorEditorWidget::processTextEdited(QString text)
{
    // set color
    m_color = QColor(text);

    // set button color
    if(m_btn != Q_NULLPTR)
    {
        m_btn->setStyleSheet(QString("background-color: %0;").arg(m_color.name()));
        m_btn->setAutoFillBackground(true);
    }
    else
    {
        cout << "Button doesn't exist" << endl;
    }
}

bool ColorEditorWidget::eventFilter(QObject *obj, QEvent *event)
{
    static bool received_focus = false;

    if (event->type() == QEvent::FocusIn)
    {
        received_focus = true;
    }
    else if (event->type() == QEvent::FocusOut)
    {
        received_focus = false;
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        if(received_focus)
        {
            m_line_edit->selectAll();
            received_focus = false;
            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}
