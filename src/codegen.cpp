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
#include <QWidget>
#include <QVariant>
#include <QFormLayout>
#include <QStackedLayout>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>

// Local Libraries
#include "codegen.h"

CodeGen::CodeGen()
{

}

QString CodeGen::codeFilename(const QString& filename, const QString& language)
{
    QFileInfo info(filename);
    if(language == "C++") {
        return info.baseName() + ".h";
    } else if (language == "python") {
        return info.baseName() + ".py";
    }
    return QString();
}

QString CodeGen::formatString(const QString& filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return file.readAll();
    }
    return QString();
}

QString CodeGen::generate(QWidget* widget, QString language)
{
    // generate the codelist
    QStringList codelist;
    CodeGen::parseWidget(widget, codelist, language);

    // get the language formatting
    QString format_filename;
    if(language == "C++") format_filename = ":/codegen/cplusplus_definition.txt";
    else if (language == "Python") format_filename = ":/codegen/python_definition.txt";
    else format_filename = ":/codegen/cplusplus_definition.txt";

    // write the styling code to file
    QDateTime local(QDateTime::currentDateTime());
    QDateTime UTC(local.toTimeSpec(Qt::UTC));
    QString widget_name = widget->objectName();

    QString format_string = CodeGen::formatString(format_filename);
    QString code = QString(format_string).arg(UTC.toString())
                                         .arg(widget_name.toLower())
                                         .arg(widget_name)
                                         .arg(codelist.join("\n"));

    if(not code.isEmpty())
    {
        QString filename;
        if(language == "C++") filename = QString("ui_%0_styling.h").arg(widget_name.toLower());
        else if (language == "Python") filename = QString("ui_%0_styling.py").arg(widget_name.toLower());
        else format_filename = filename = QString("ui_%0_styling.h").arg(widget_name.toLower());

        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(code.toLatin1());
        } else {
            qDebug() << "Error: on_action_Export_triggered. Failed to save file " << filename;
        }
        file.close();
    }

    return code;
}

void CodeGen::parseWidget(QWidget* widget, QStringList& codelist, const QString& language)
{
    // check that the widget is not null
    if(! widget) {
        QString();
    }

    // test if the widgets margins were changed
    QVariant prop = widget->property("_g_margin_edit");
    if(prop.isValid()){
        if(language == "C++") {
            codelist << CodeGen::widgetCplusplus(widget);
        } else if (language == "Python") {
            codelist << CodeGen::widgetPython(widget);
        }
    }

    if(widget->layout()) {
        CodeGen::parseLayout(widget->layout(), codelist, language);
    }
}

void CodeGen::parseLayout(QLayout* layout, QStringList& codelist, const QString& language)
{
    // check that the widget has a layout
    if(! layout) {
        QString();
    }

    // test if the widgets margins were changed
    QVariant prop = layout->property("_g_spacing_edit");
    if(prop.isValid()){
        if(language == "C++") {
            codelist << CodeGen::layoutCplusplus(layout);
        } else if(language == "Python") {
            codelist << CodeGen::layoutPython(layout);
        }
    }

    // loop through the layout
    for(int i = 0; i < layout->count(); ++i)
    {
        QLayoutItem* layout_item = layout->itemAt(i);
        if(layout_item){
            if(! layout_item->isEmpty()) {
                if(layout_item->widget()){
                    CodeGen::parseWidget(layout_item->widget(), codelist, language);
                }
                else if (layout_item->layout()) {
                    CodeGen::parseLayout(layout_item->layout(), codelist, language);
                }
                else if (layout_item->spacerItem()) {
//                    QStandardItem* item_spacer_name = new QStandardItem("Spacer");
//                    QStandardItem* item_spacer_class = new QStandardItem("QSpacerItem");
//                    item_spacer_name->setEditable(false);
//                    item_spacer_class->setEditable(false);
//                    item_layout_name->appendRow({item_spacer_name, item_spacer_class});
                }
            }
        }
    }
}

// -----------------------------------------------------------
// C++

QString CodeGen::widgetCplusplus(QWidget* widget)
{
    QMargins m = widget->contentsMargins();
    QString code = QString("ui->%1->setContentsMargins(%2, %3, %4, %5);").arg(widget->objectName())
                                                                         .arg(m.left()).arg(m.top())
                                                                         .arg(m.right()).arg(m.bottom());
    return QString().fill(' ', 8) + code;
}

QString CodeGen::layoutCplusplus(QLayout* layout)
{
    QString tab8 = QString().fill(' ', 8);

    QMargins m = layout->contentsMargins();
    QStringList codelist;
    codelist << tab8 + QString("ui->%1->setContentsMargins(%2, %3, %4, %5);").arg(layout->objectName())
                                                                             .arg(m.left()).arg(m.top())
                                                                             .arg(m.right()).arg(m.bottom());

    QString obj_class = layout->metaObject()->className();
    if(obj_class == "QBoxLayout"){
        QBoxLayout* blayout = qobject_cast<QBoxLayout*>(layout);
        codelist << tab8 + QString("ui->%1->setSpacing(%2);").arg(layout->objectName()).arg(blayout->spacing());
    } else  if(obj_class == "QFormLayout"){
        QFormLayout* flayout = qobject_cast<QFormLayout*>(layout);
        codelist << tab8 + QString("ui->%1->setHorizontalSpacing(%2);").arg(layout->objectName()).arg(flayout->horizontalSpacing());
        codelist << tab8 + QString("ui->%1->setVerticalSpacing(%2);").arg(layout->objectName()).arg(flayout->verticalSpacing());
    } else  if(obj_class == "QGridLayout"){
        QGridLayout* glayout = qobject_cast<QGridLayout*>(layout);
        codelist << tab8 + QString("ui->%1->setHorizontalSpacing(%2);").arg(layout->objectName()).arg(glayout->horizontalSpacing());
        codelist << tab8 + QString("ui->%1->setVerticalSpacing(%2);").arg(layout->objectName()).arg(glayout->verticalSpacing());
    } else  if(obj_class == "QStackedLayout"){
        QStackedLayout* slayout = qobject_cast<QStackedLayout*>(layout);
        codelist << tab8 + QString("ui->%1->setSpacing(%2);").arg(layout->objectName()).arg(slayout->spacing());
    }

    return codelist.join("\n");
}

// -----------------------------------------------------------
// Python

QString CodeGen::widgetPython(QWidget* widget)
{
    QMargins m = widget->contentsMargins();
    QString code = QString("ui.%1.setContentsMargins(%2, %3, %4, %5)").arg(widget->objectName())
                                                                        .arg(m.left()).arg(m.top())
                                                                        .arg(m.right()).arg(m.bottom());
    return QString().fill(' ', 8) + code;
}

QString CodeGen::layoutPython(QLayout* layout)
{
    QString tab8 = QString().fill(' ', 8);

    QMargins m = layout->contentsMargins();
    QStringList codelist;
    codelist << tab8 + QString("ui.%1.setContentsMargins(%2, %3, %4, %5)").arg(layout->objectName())
                                                                            .arg(m.left()).arg(m.top())
                                                                            .arg(m.right()).arg(m.bottom());

    QString obj_class = layout->metaObject()->className();
    if(obj_class == "QBoxLayout"){
        QBoxLayout* blayout = qobject_cast<QBoxLayout*>(layout);
        codelist << tab8 + QString("ui.%1.setSpacing(%2)").arg(layout->objectName()).arg(blayout->spacing());
    } else  if(obj_class == "QFormLayout"){
        QFormLayout* flayout = qobject_cast<QFormLayout*>(layout);
        codelist << tab8 + QString("ui.%1.setHorizontalSpacing(%2)").arg(layout->objectName()).arg(flayout->horizontalSpacing());
        codelist << tab8 + QString("ui.%1.setVerticalSpacing(%2)").arg(layout->objectName()).arg(flayout->verticalSpacing());
    } else  if(obj_class == "QGridLayout"){
        QGridLayout* glayout = qobject_cast<QGridLayout*>(layout);
        codelist << tab8 + QString("ui.%1.setHorizontalSpacing(%2)").arg(layout->objectName()).arg(glayout->horizontalSpacing());
        codelist << tab8 + QString("ui.%1.setVerticalSpacing(%2)").arg(layout->objectName()).arg(glayout->verticalSpacing());
    } else  if(obj_class == "QStackedLayout"){
        QStackedLayout* slayout = qobject_cast<QStackedLayout*>(layout);
        codelist << tab8 + QString("ui.%1.setSpacing(%2)").arg(layout->objectName()).arg(slayout->spacing());
    }

    return codelist.join("\n");
}
