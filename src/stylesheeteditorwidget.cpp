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
#include <functional>
#include <vector>

// Qt Libraries
#include <QFile>
#include <QInputDialog>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QSyntaxHighlighter>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QUuid>
#include <QDebug>
#include <QCompleter>
#include <QMainWindow>
#include <QMessageBox>
#include <QSlider>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QListView>


// Local Libraries
#include "stylesheeteditorwidget.h"
#include "ui_stylesheeteditorwidget.h"
#include "highlighter.h"
#include "stylesheeteditoritemdelegate.h"
#include "texteditor.h"
#include "colorschemegenerator.h"
#include "dialogcolorspec.h"
#include "workspace.h"
#include "dialogpagecreator.h"



StyleSheetEditorWidget::StyleSheetEditorWidget(Workspace* workspace, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StyleSheetEditorWidget),
    m_workspace(workspace)
{
    ui->setupUi(this);

    // setup the variables model
    m_vars_model = new QStandardItemModel(0, 2, this);
    m_vars_model->setHeaderData(0, Qt::Horizontal, "Variable");
    m_vars_model->setHeaderData(1, Qt::Horizontal, "Value");

    connect(m_vars_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(processVarsModelDataChanged(QModelIndex,QModelIndex,QVector<int>)));

    ui->treeViewVars->setModel(m_vars_model);

    StyleSheetEditorItemDelegate* delegate = new StyleSheetEditorItemDelegate(this);
    ui->treeViewVars->setItemDelegate(delegate);

    // setup the snippets model
    this->m_snippet_model = new QStandardItemModel(0, 2, this);
    this->m_snippet_model->setHeaderData(0, Qt::Horizontal, "Snippet");
    this->m_snippet_model->setHeaderData(1, Qt::Horizontal, "Value");

    connect(this->m_snippet_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(processSnippetModelDataChanged(QModelIndex,QModelIndex,QVector<int>)));

    ui->treeViewSnippets->setModel(m_snippet_model);

    ui->treeViewSnippets->setItemDelegate(delegate);

    // setup the text editor
    this->m_text_editor = new TextEditor(this);
    this->m_text_editor->setFrameStyle(QFrame::NoFrame);
    QFont font = m_text_editor->font();
    font.setFamily("Courier");
    font.setPointSize(10);
    this->m_text_editor->setFont(font);
    this->m_text_editor->setEnabled(false);
    this->m_text_editor->setAcceptDrops(true);
    this->m_text_editor->installEventFilter(this);

    // setup the syntax highlighter
    this->m_highlihter = new Highlighter(m_text_editor->document());

    // set the tab stop of the editor
    int w = this->m_text_editor->fontMetrics().width("abcd");
    this->m_text_editor->setTabStopWidth(w);

    QGridLayout* layout = new QGridLayout(ui->widgetEditor);
    layout->setMargin(0);
    layout->addWidget(m_text_editor, 0, 0, 1, 1);

    connect(this->m_text_editor, SIGNAL(textChanged()), this, SLOT(processTextChanged()));

    // setup the style sheets pages model
    this->m_page_model = new QStandardItemModel(0, 1, this);
    this->m_page_model->setHeaderData(0, Qt::Horizontal, "Page");
    this->m_page_model->setHeaderData(1, Qt::Horizontal, "Style sheet");

    ui->treeViewPage->setModel(this->m_page_model);
    ui->treeViewPage->setItemDelegate(delegate);
    ui->treeViewPage->setColumnHidden(1, true);


    // setup preview
//    this->m_live_preview = ui->checkBoxLive->checkState() == Qt::Checked ? true : false;

    // initialize main window pointer
    this->m_main_window = Q_NULLPTR;

    // set up the completer
    this->m_completer = Q_NULLPTR;
}

StyleSheetEditorWidget::~StyleSheetEditorWidget()
{
    delete ui;
}

void StyleSheetEditorWidget::setLivePreview(const bool& state)
{
    this->m_live_preview = state;
    if(this->m_live_preview) {
        emit this->styleSheetReady(this->generateStyleSheet());
    }
}

QString StyleSheetEditorWidget::generateStyleSheet()
{
    // combine the text from the pages
    QString text;
    for(int i = 0; i < m_page_model->rowCount(); ++i)
    {
        QStandardItem* page_item = m_page_model->item(i, 0);
        QStandardItem* qss_item = m_page_model->item(i, 1);
        QString page_name = page_item->text();
        if(page_item->checkState() == Qt::Checked)
        {
            QString id = page_item->data(Qt::UserRole + 1).toString();
            text += QString("/* %0 */\n\n%1\n\n").arg(page_name).arg(qss_item->text());
        }
    }

    // replace snippet names with the snippet value
    for(int i = 0; i < m_snippet_model->rowCount(); ++i)
    {
        QString snippet_name = m_snippet_model->index(i, 0).data(Qt::DisplayRole).toString().trimmed();
        QString snippet_value = m_snippet_model->index(i, 1).data(Qt::DisplayRole).toString().trimmed();

        // replace variable name with variable value
        QRegExp reg_exp(QString("\\b%0\\b").arg(snippet_name));
        text = text.replace(reg_exp, snippet_value);
    }

    // replace variable names with variable value
    for(int i = 0; i < m_vars_model->rowCount(); ++i)
    {
        QString var_name = m_vars_model->index(i, 0).data(Qt::DisplayRole).toString().trimmed();
        QString var_value = m_vars_model->index(i, 1).data(Qt::DisplayRole).toString().trimmed();

        // replace variable name with variable value
        QRegExp reg_exp(QString("\\b%0\\b").arg(var_name));
        text = text.replace(reg_exp, var_value);
    }

    return text.trimmed();
}

QString StyleSheetEditorWidget::replaceWithSnippet(const QString& text)
{
    QString local_text = text;
    for(int i = 0; i < m_snippet_model->rowCount(); ++i)
    {
        QString snippet_name = m_snippet_model->index(i, 0).data(Qt::DisplayRole).toString().trimmed();
        QString snippet_value = m_snippet_model->index(i, 1).data(Qt::DisplayRole).toString().trimmed();

        // replace variable name with variable value
        QRegExp reg_exp(QString("\\b%0\\b").arg(snippet_name));
        local_text = local_text.replace(reg_exp, snippet_value);
    }
    return local_text;
}

QString StyleSheetEditorWidget::replaceWithVariables(const QString& text)
{
    QString local_text = text;
    for(int i = 0; i < m_vars_model->rowCount(); ++i)
    {
        QString var_name = m_vars_model->index(i, 0).data(Qt::DisplayRole).toString().trimmed();
        QString var_value = m_vars_model->index(i, 1).data(Qt::DisplayRole).toString().trimmed();

        // replace variable name with variable value
        QRegExp reg_exp(QString("\\b%0\\b").arg(var_name));
        local_text = local_text.replace(reg_exp, var_value);
    }
    return local_text;
}

void StyleSheetEditorWidget::on_btnAddVar_clicked()
{
    if(m_vars_model == Q_NULLPTR)
    {
        std::cout << "StyleSheetEditorWidget: The variables model is null" << std::endl;
        return;
    }

    QString var_name = "var_name";
    this->addVariable(var_name, "");
}

void StyleSheetEditorWidget::addVariable(const QString& var_name, const QString& var_value, const bool& ischecked)
{
    QStandardItem* item_var = new QStandardItem(var_name);
    item_var->setCheckable(true);
    item_var->setCheckState(ischecked ? Qt::Checked : Qt::Unchecked);
    QStandardItem* item_value = new QStandardItem(var_value);

    m_vars_model->appendRow({item_var, item_value});

    // update the completer
    if(!m_words.contains(var_name))
        m_words.append(var_name);

    this->updateCompleterWordList();
}

void StyleSheetEditorWidget::addSnippet(const QString& snippet_name, const QString& snippet_value, const bool& ischecked)
{
    QStandardItem* item_snippet = new QStandardItem(snippet_name);
    item_snippet->setCheckable(true);
    item_snippet->setCheckState(ischecked ? Qt::Checked : Qt::Unchecked);
    QStandardItem* item_value = new QStandardItem(snippet_value);

    m_snippet_model->appendRow({item_snippet, item_value});

    // update the completer
    if(!m_words.contains(snippet_name))
        m_words.append(snippet_name);

    this->updateCompleterWordList();
}

QString StyleSheetEditorWidget::addPage(const QString& pagename, const QString& qss, const bool& ischecked)
{
    QString id = QUuid::createUuid().toString();
    QStandardItem* page_item = new QStandardItem(pagename);
    page_item->setCheckable(true);
    page_item->setCheckState(ischecked ? Qt::Checked : Qt::Unchecked);
    page_item->setData(id, Qt::UserRole + 1);

    QStandardItem* qss_item = new QStandardItem(qss);
    m_page_model->appendRow({page_item, qss_item});

    // set the current index of the list view to the new page
    ui->treeViewPage->setCurrentIndex(page_item->index());
    ui->treeViewPage->setColumnHidden(1, true);

    // set the text editor
    m_text_editor->blockSignals(true);
    m_text_editor->setPlainText(qss);
    m_text_editor->blockSignals(false);

    // consistency check - the editor should be enable there are one or more rows
    if(this->m_page_model->rowCount() > 0){
        this->m_text_editor->setEnabled(true);
    }

    return id;
}

void StyleSheetEditorWidget::on_btnRemoveVar_clicked()
{
    // get checked rows
    QList<int> items;
    for(int i = 0; i < m_vars_model->rowCount(); ++i)
    {
        QStandardItem* var_name_item = m_vars_model->item(i, 0);
        if(var_name_item->checkState() == Qt::Checked)
        {
            items << i;

            // remove the variable from the completer's word list
            int wi = m_words.indexOf(var_name_item->text());
            m_words.removeAt(wi);
        }
    }

    // remove the rows from the back of m_vars_model
    // removing rows from the front will result in the changing of row indeces
    std::sort(items.begin(), items.end(), std::greater<int>());
    for(int row_index: items)
    {
        m_vars_model->removeRow(row_index);
    }

    // update the completer
    this->updateCompleterWordList();
}

void StyleSheetEditorWidget::processTextChanged()
{
    // update the hash table
    QModelIndex index = ui->treeViewPage->currentIndex();
    if(index.isValid())
    {
        QStandardItem* qss_item = m_page_model->item(index.row(), 1);
        qss_item->setText(this->m_text_editor->toPlainText());
    }

    if(this->m_live_preview)
    {
        emit this->styleSheetReady(this->generateStyleSheet());
    }
}

QJsonArray StyleSheetEditorWidget::variablesJson()
{
    QJsonArray variable_arr;
    for(int i = 0; i < m_vars_model->rowCount(); ++i)
    {
        QStandardItem* var_item = m_vars_model->item(i, 0);
        QString var_name = m_vars_model->index(i, 0).data(Qt::DisplayRole).toString();
        QString var_value = m_vars_model->index(i, 1).data(Qt::DisplayRole).toString();

        QJsonObject variable_obj;
        variable_obj["name"] = var_name;
        variable_obj["checked"] = var_item->checkState() == Qt::Checked ? true : false;
        variable_obj["value"] = var_value;

        variable_arr.append(variable_obj);
    }

    return variable_arr;
}

void StyleSheetEditorWidget::addVariablesFromJson(const QJsonValue& variables_value)
{
    QJsonArray variables_arr = variables_value.toArray();
    foreach(QJsonValue variable_value, variables_arr)
    {
        QJsonObject variable_obj = variable_value.toObject();
        this->addVariable(variable_obj["name"].toString(), variable_obj["value"].toString(), variable_obj["checked"].toBool());
    }
}

QJsonArray StyleSheetEditorWidget::snippetsJson()
{
    QJsonArray snippet_arr;
    for(int i = 0; i < m_snippet_model->rowCount(); ++i)
    {
        QStandardItem* snippet_item = m_vars_model->item(i, 0);
        QString snippet_name = m_snippet_model->index(i, 0).data(Qt::DisplayRole).toString();
        QString snippet_value = m_snippet_model->index(i, 1).data(Qt::DisplayRole).toString();


        QJsonObject snippet_obj;
        snippet_obj["name"] = snippet_name;
        snippet_obj["checked"] = snippet_item->checkState() == Qt::Checked ? true : false;
        snippet_obj["value"] = snippet_value;

        snippet_arr.append(snippet_obj);
    }

    return snippet_arr;
}

void StyleSheetEditorWidget::addSnippetsFromJson(const QJsonValue& snippets_value)
{
    QJsonArray snippets_arr = snippets_value.toArray();
    foreach(QJsonValue snippet_value, snippets_arr)
    {
        QJsonObject snippet_obj = snippet_value.toObject();
        this->addSnippet(snippet_obj["name"].toString(), snippet_obj["value"].toString(), snippet_obj["checked"].toBool());
    }
}

QJsonArray StyleSheetEditorWidget::pagesJson()
{
    QJsonArray page_arr;
    for(int i = 0; i < m_page_model->rowCount(); ++i)
    {
        QStandardItem* page_item = m_page_model->item(i, 0);
        QStandardItem* qss_item = m_page_model->item(i, 1);

        QString page_name = page_item->text();
        QString id = page_item->data(Qt::UserRole + 1).toString();
        QString qss = qss_item->text();

        QJsonObject page_obj;
        page_obj["name"] = page_name;
        page_obj["checked"] = page_item->checkState() == Qt::Checked ? true : false;
        page_obj["qss"] = qss;

        page_arr.append(page_obj);
    }

    return page_arr;
}

void StyleSheetEditorWidget::addPagesFromJson(const QJsonValue& snippets_value)
{
    QJsonArray pages_arr = snippets_value.toArray();
    foreach(QJsonValue page_value, pages_arr)
    {
        QJsonObject page_obj = page_value.toObject();
        this->addPage(page_obj["name"].toString(), page_obj["qss"].toString(), page_obj["checked"].toBool());
    }
}

bool StyleSheetEditorWidget::saveStyleSheet(const QString& filename)
{
//    QString qssfilename = QString("%0.qss").arg(QFileInfo(m_filename).baseName());
//    this->saveStyleSheet(qssfilename);

    // test if the local filename is empty
    QString local_filename = filename;
    if(local_filename.isEmpty()) {
        local_filename = QFileDialog::getSaveFileName(this, tr("Stylesheet File"), this->m_qss_filename, tr("Style File, *.qss (*.qss)"));
        if(local_filename.isEmpty()) {
            qDebug() << "Error: saveStyleSheet";
            return false;
        } else {
            this->m_qss_filename = local_filename;
        }
    }

    // save qss
    QFile file(local_filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(this->generateStyleSheet().toLatin1());
        return true;
    } else {
        qDebug() << "Error: saveStyleSheet. Failed to save file " << local_filename;
    }

    file.close();

    return false;
}


void StyleSheetEditorWidget::on_btnClear_clicked()
{
    m_text_editor->clear();
}

void StyleSheetEditorWidget::processVarsModelDataChanged(const QModelIndex &topLeft,
                                                         const QModelIndex &bottomRight,
                                                         const QVector<int> &roles)
{
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)

    // when the variable name changes
    if(topLeft.column() == 0)
    {
        // check if the variable name has changed
        QString curr_var_name = topLeft.data().toString().trimmed();
        if( !m_old_var_name.isEmpty() && (curr_var_name != m_old_var_name) )
        {
            // remove the old variable name from the words list
            int wi = m_words.indexOf(m_old_var_name);
            m_words.removeAt(wi);

            // add the new variable
            if(!m_words.contains(curr_var_name))
                m_words.append(curr_var_name);

            // update the completer
            this->updateCompleterWordList();

            // update the variable names in the style sheets
            for(int row = 0; row < this->m_page_model->rowCount(); ++row){
                QStandardItem* qss_item = m_page_model->item(row, 0);
                QString text = qss_item->text();
                text.replace(m_old_var_name, curr_var_name);
                qss_item->setText(text);
            }

            // update the editor
//            QString id = ui->treeViewPage->currentIndex().data(Qt::UserRole + 1).toString();
            QStandardItem* qss_item = m_page_model->item(ui->treeViewPage->currentIndex().row(), 1);
            m_text_editor->setText(qss_item->text());
        }

        // generate a style sheet if the live preview is on
        if(m_live_preview)
            emit this->styleSheetReady(this->generateStyleSheet());
    }
    // ... when the color changes
    else if(topLeft.column() == 1)
    {
        // block signals to the model to avoid a recursive call to this function
        //m_vars_model->blockSignals(true);

        // update the decoration role of the color item
        //QColor color = topLeft.data(Qt::DisplayRole).value<QColor>();
        //QStandardItem* item_value = m_vars_model->itemFromIndex(topLeft);
        //item_value->setData(color, Qt::DecorationRole);

        //m_vars_model->blockSignals(false);
    }
}

void StyleSheetEditorWidget::processSnippetModelDataChanged(const QModelIndex &topLeft,
                                                            const QModelIndex &bottomRight,
                                                            const QVector<int> &roles)
{
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)

    // when the variable name changes
    if(topLeft.column() == 0)
    {
        // check if the variable name has changed
        QString curr_snippet_name = topLeft.data().toString().trimmed();
        if( !m_old_var_name.isEmpty() && (curr_snippet_name != m_old_var_name) )
        {
            // remove the old variable name from the words list
            int wi = m_words.indexOf(m_old_var_name);
            m_words.removeAt(wi);

            // add the new variable
            if(!m_words.contains(curr_snippet_name))
                m_words.append(curr_snippet_name);

            // update the completer
            this->updateCompleterWordList();

            // update the variable names in the style sheets
            for(int row = 0; row < this->m_page_model->rowCount(); ++row){
                QStandardItem* qss_item = m_page_model->item(row, 0);
                QString text = qss_item->text();
                text.replace(m_old_snippet_name, curr_snippet_name);
                qss_item->setText(text);
            }

            // update the editor
            QStandardItem* qss_item = m_page_model->item(ui->treeViewPage->currentIndex().row(), 1);
            m_text_editor->setText(qss_item->text());
        }

        // generate a style sheet if the live preview is on
        if(m_live_preview)
            emit this->styleSheetReady(this->generateStyleSheet());
    }
    // ... when the color changes
    else if(topLeft.column() == 1)
    {
        // block signals to the model to avoid a recursive call to this function
        //m_vars_model->blockSignals(true);

        // update the decoration role of the color item
        //QColor color = topLeft.data(Qt::DisplayRole).value<QColor>();
        //QStandardItem* item_value = m_vars_model->itemFromIndex(topLeft);
        //item_value->setData(color, Qt::DecorationRole);

        //m_vars_model->blockSignals(false);
    }
}

void StyleSheetEditorWidget::on_btnApplyStyleSheet_clicked()
{
    QString ss = this->generateStyleSheet();
    emit this->styleSheetReady(ss);
}

void StyleSheetEditorWidget::on_btnAddPage_clicked()
{
    if(m_page_model == Q_NULLPTR)
    {
        std::cout << "StyleSheetEditorWidget: The page model is null" << std::endl;
        return;
    }

    bool ok;
    QString page_name = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                              tr("Page name:"), QLineEdit::Normal,
                                              "Page name", &ok);
    if (ok && !page_name.isEmpty())
    {
        this->addPage(page_name);
    }
}

void StyleSheetEditorWidget::on_btnRemovePage_clicked()
{
    QModelIndex index = ui->treeViewPage->currentIndex();
    QString id = m_page_model->data(index, Qt::UserRole + 1).toString();

    m_page_model->removeRow(index.row());

    // consistency check - the editor should be disabled if there are zero rows
    if(this->m_page_model->rowCount() == 0) {
        this->m_text_editor->clear();
        this->m_text_editor->setEnabled(false);
    }

    // apply if live is true
    if(this->m_live_preview) {
        emit this->styleSheetReady(this->generateStyleSheet());
    }
}

void StyleSheetEditorWidget::on_btnPageUp_clicked()
{
    QModelIndex index = ui->treeViewPage->currentIndex();
    if(index.isValid())
    {
        QList<QStandardItem*> items;
        if(index.row() > 0)
        {
            items = m_page_model->takeRow(index.row());
            m_page_model->insertRow(index.row() - 1, items);
        }
        ui->treeViewPage->setCurrentIndex(items[0]->index());
    }

    // generate a style sheet if the live preview is on
    if(m_live_preview)
        emit this->styleSheetReady(this->generateStyleSheet());
}

void StyleSheetEditorWidget::on_btnPageDown_clicked()
{
    QModelIndex index = ui->treeViewPage->currentIndex();
    if(index.isValid())
    {
        QList<QStandardItem*> items;
        if(index.row() < (m_page_model->rowCount() - 1))
        {
            items = m_page_model->takeRow(index.row());
            m_page_model->insertRow(index.row() + 1, items);
        }
        ui->treeViewPage->setCurrentIndex(items[0]->index());
    }

    // generate a style sheet if the live preview is on
    if(m_live_preview)
        emit this->styleSheetReady(this->generateStyleSheet());
}

void StyleSheetEditorWidget::on_treeViewPage_clicked(const QModelIndex &index)
{
    QStandardItem* qss_item = m_page_model->item(index.row(), 1);

    m_text_editor->blockSignals(true);
    m_text_editor->setPlainText(qss_item->text());
    m_text_editor->blockSignals(false);

    // generate a style sheet if the live preview is on
    if(m_live_preview)
        emit this->styleSheetReady(this->generateStyleSheet());
}

void StyleSheetEditorWidget::setupCompleter()
{
    // delete the completer
    if(m_completer != Q_NULLPTR)
    {
        delete m_completer;
        m_completer = Q_NULLPTR;
    }

    // create the completer
    m_completer = new QCompleter(this);

    // read the words from the files
    QStringList filename;
    filename << ":/stylesheeteditor/widgets.txt"
             << ":/stylesheeteditor/pseudo-states.txt"
             << ":/stylesheeteditor/properties.txt"
             << ":/stylesheeteditor/sub-controls.txt"
             << ":/stylesheeteditor/icons.txt";

    // ... add object names to the list of words
    m_words = this->getMainWindowObjects();;

    for(int i = 0; i < filename.count(); ++i)
    {
        QFile file(filename[i]);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            while (!file.atEnd())
            {
                QString line = file.readLine();
                QStringList sp = line.split(',', QString::SkipEmptyParts);
                QString word = sp[0].trimmed();
                m_words << word;
            }
        }
        else
        {
            std::cout << "File counldn't be opened: " << filename[i].toStdString() << std::endl;
        }
        file.close();
    }
    std::cout << "Num words: " << m_words.count() << std::endl;

    m_completer_model = new QStringListModel(m_words, m_completer);
    m_completer_model->sort(0);

    // setup the completer
    m_completer->setModel(m_completer_model);
    //m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setWrapAround(false);
    m_text_editor->setCompleter(m_completer);
}

void StyleSheetEditorWidget::updateCompleterWordList()
{
    m_completer_model->setStringList(m_words);
    m_completer_model->sort(0);
}

void StyleSheetEditorWidget::on_treeViewVars_doubleClicked(const QModelIndex &index)
{
    if(index.column() == 0)
        m_old_var_name = index.data().toString().trimmed();
}

void StyleSheetEditorWidget::on_treeViewSnippets_doubleClicked(const QModelIndex &index)
{
    if(index.column() == 0)
        m_old_snippet_name = index.data().toString().trimmed();
}

void StyleSheetEditorWidget::on_btnVarUp_clicked()
{
    QModelIndex index = ui->treeViewVars->currentIndex();
    if(index.isValid())
    {
        if(index.row() > 0)
        {
            QList<QStandardItem*> items = m_vars_model->takeRow(index.row());
            m_vars_model->insertRow(index.row() - 1, items);
        }
    }
}

void StyleSheetEditorWidget::on_btnVarDown_clicked()
{
    QModelIndex index = ui->treeViewVars->currentIndex();
    if(index.isValid())
    {
        if(index.row() < (m_vars_model->rowCount() - 1))
        {
            QList<QStandardItem*> items = m_vars_model->takeRow(index.row());
            m_vars_model->insertRow(index.row() + 1, items);
        }
    }
}

void StyleSheetEditorWidget::generateColorSchemeFromImageFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Image File"),
                                                    "./", tr("Images, *.png *.xpm *.jpg (*.png *.xpm *.jpg)"));

    if(filename.isEmpty())
       return;


    bool ok;
    int num_colors = QInputDialog::getInt(this, tr("Input the number colors in the scheme"), tr("Number of colors"), 16, 0, 1024, 1, &ok);
    if(!ok)
        return;

    int num_samples = 1000;
    int color_ordering = ColorSchemeGenerator::VSH;

    // get the scheme
    QMap<int, QColor> scheme = ColorSchemeGenerator::generate(filename, num_colors, num_samples, color_ordering);
    QList<int> order = scheme.keys();
    std::sort(order.begin(), order.end(), std::greater<int>());

    // check if duplicate varaible names should be overwritten
    typedef QPair<QStandardItem*,QStandardItem*> ItemPair;
    QList<ItemPair> items = this->getColorItems();
    QStringList dup_var_names;
    QList<QStandardItem*> dup_var_value_items;
    for(ItemPair item_pair: items)
    {
        if(item_pair.first->text().trimmed().left(10) == "var_color_")
        {
            dup_var_names << item_pair.first->data(Qt::EditRole).toString();
            dup_var_value_items << item_pair.second;
        }
    }

    int replace_duplicates = QMessageBox::Yes;
    if(dup_var_names.count() > 0)
    {
        QMessageBox msgBox;
        msgBox.setText("There are duplicate variable names.");
        msgBox.setInformativeText("Do you want to overwrite the value of the duplicate variables?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        replace_duplicates = msgBox.exec();
    }

    // add the color scheme to the m_vars_model.
    // the value of duplicate variables will be replaced if replace_duplicates == 1

    QString var_name;
    QColor var_color;
    int ci = 0;

    // ... don't replace duplicate get the last color variable index
    if(replace_duplicates == QMessageBox::No)
    {
        // ... get last color index
        for(QString var_name: dup_var_names)
        {
            QStringList sp = var_name.split('_', QString::SkipEmptyParts); // e.g., var_color_23
            QString sindex = sp.last();
            ci = ci < sindex.toInt() ? sindex.toInt() : ci;
        }

        for(auto key: order)
        {
            var_name = QString("var_color_%0").arg(ci++);
            var_color = scheme[key];
            this->addVariable(var_name, var_color.name());

            // add the variable name to the word list
            if(!m_words.contains(var_name))
                m_words.append(var_name);
        }

        // update the completer
        this->updateCompleterWordList();
    }
    // ... replace duplicate color variables
    else
    {
        for(auto key: order)
        {
            var_name = QString("var_color_%0").arg(ci++);
            var_color = scheme[key];

            // look for duplicate variable
            int i = dup_var_names.indexOf(var_name);

            // ... duplicate found, therefore replace value of duplicate
            if(i >= 0)
            {
                dup_var_value_items[i]->setData(var_color.name(), Qt::DisplayRole);
            }
            // ... no duplicate available so add variable
            else
            {
                this->addVariable(var_name, var_color.name());

                // add the variable name to the word list
                if(!m_words.contains(var_name))
                    m_words.append(var_name);
            }
        }

        // update the completer
        this->updateCompleterWordList();
    }

    this->rearrangeColorScheme();
}

QStringList StyleSheetEditorWidget::getMainWindowObjects()
{
    if(m_main_window == Q_NULLPTR)
        return QStringList();

    // get unique object names in the main window
    QSet<QString> object_name;
    for(QWidget* child: m_main_window->findChildren<QWidget*>())
    {
        QString name = child->objectName().trimmed();
        if(!name.isEmpty())
        {
            object_name << QString("%0#%1").arg(child->metaObject()->className()).arg(name);
        }
    }

    // convert to sorted list
    QStringList object_name_list = object_name.toList();
    object_name_list.sort();
    return object_name_list;
}

void StyleSheetEditorWidget::setMainWindow(QMainWindow* main_window)
{
    m_main_window = main_window;

    // set the editor's completer
    this->setupCompleter();
}

QList<QPair<QStandardItem*,QStandardItem*>> StyleSheetEditorWidget::getColorItems()
{
    typedef QPair<QStandardItem*,QStandardItem*> ItemPair;

    QList<ItemPair> items;
    for(int i = 0; i < m_vars_model->rowCount(); ++i)
    {
        QStandardItem* var_name_item  = m_vars_model->item(i, 0);
        QStandardItem* var_value_item = m_vars_model->item(i, 1);
        QString var_value = var_value_item->data(Qt::DisplayRole).toString().trimmed();

        if(var_value.left(1) == "#")
        {
            items << ItemPair(var_name_item, var_value_item);
        }
        else if(QColor::colorNames().contains(var_value, Qt::CaseInsensitive))
        {
            items << ItemPair(var_name_item, var_value_item);
        }
    }
    return items;
}

void StyleSheetEditorWidget::rearrangeColorScheme()
{
    // setup functions
    int color_ordering = ui->comboBoxOrdering->currentIndex(); //HSV_Ordering;
    std::function<int(StyleSheetEditorWidget&, QColor&)> order_func;
    switch(color_ordering)
    {
    case HSV_Ordering: order_func = &StyleSheetEditorWidget::hsv; break;
    case HVS_Ordering: order_func = &StyleSheetEditorWidget::hvs; break;
    case SVH_Ordering: order_func = &StyleSheetEditorWidget::svh; break;
    case SHV_Ordering: order_func = &StyleSheetEditorWidget::shv; break;
    case VHS_Ordering: order_func = &StyleSheetEditorWidget::vhs; break;
    case VSH_Ordering: order_func = &StyleSheetEditorWidget::vsh; break;
    default: order_func = &StyleSheetEditorWidget::vhs;
    }

    // get variables that are colors
    QList<QStandardItem*> hash_items;
    QHash<int, QColor> hash_colors;
    QList<int> order_list;
    for(int i = 0; i < m_vars_model->rowCount(); ++i)
    {
        QStandardItem* var_name_item  = m_vars_model->item(i, 0);
        QStandardItem* var_value_item = m_vars_model->item(i, 1);
        QString var_value = var_value_item->data(Qt::DisplayRole).toString().trimmed();

        if(var_name_item->checkState() == Qt::Checked)
        {
            bool is_color = false;
            if(var_value.left(1) == "#")
                is_color = true;
            else if(QColor::colorNames().contains(var_value, Qt::CaseInsensitive))
                is_color = true;

            if(is_color)
            {
                hash_items << var_value_item;

                QColor color(var_value);
                int order = order_func(*this, color);
                hash_colors[order] = color;
                order_list << order;
            }
        }
    }

    // now rearrange the colors
    // ... if only two colors have been checked
    if(order_list.count() == 2)
    {
        int order[] = { order_list[0], order_list[0] };
        QString color_name[] = { hash_colors[order[0]].name(), hash_colors[order[1]].name() };

        hash_items[0]->setData(color_name[1], Qt::DisplayRole);
        hash_items[1]->setData(color_name[0], Qt::DisplayRole);
    }
    // ... if more than two colors have been checked
    if(order_list.count() > 2)
    {
        if(color_ordering == Random_Ordering)
        {
            std::random_shuffle(order_list.begin(), order_list.end());
        }
        else if(color_ordering == Reverse_Ordering)
        {
            std::reverse(order_list.begin(), order_list.end());
        }
        else
        {
            std::sort(order_list.begin(), order_list.end()); // order in descending order
        }

        for(int i = 0; i < order_list.count(); ++i)
        {
            int order = order_list[i];
            QString color_name = hash_colors[order].name();
            hash_items[i]->setData(color_name, Qt::DisplayRole);
        }
    }
}

void StyleSheetEditorWidget::on_checkBoxSelect_clicked()
{
    Qt::CheckState state = Qt::Unchecked;
    if(ui->checkBoxSelect->checkState() == Qt::Checked)
    {
        state = Qt::Checked;
    }

    for(int i = 0; i < m_vars_model->rowCount(); ++i)
    {
        QString var_value = m_vars_model->item(i, 1)->data(Qt::DisplayRole).toString().trimmed();

        bool is_color = false;
        if(var_value.left(1) == "#")
            is_color = true;
        else if(QColor::colorNames().contains(var_value, Qt::CaseInsensitive))
            is_color = true;

        if(is_color)
            m_vars_model->item(i, 0)->setCheckState(state);
    }
}

void StyleSheetEditorWidget::on_btnRandomShuffle_clicked()
{
    // get variables that are colors
    QList<QStandardItem*> hash_items;
    QHash<int, QColor> hash_colors;
    QList<int> order_list;
    int order = 0;
    for(int i = 0; i < m_vars_model->rowCount(); ++i)
    {
        QStandardItem* var_name_item  = m_vars_model->item(i, 0);
        QStandardItem* var_value_item = m_vars_model->item(i, 1);
        QString var_value = var_value_item->data(Qt::DisplayRole).toString().trimmed();

        if(var_name_item->checkState() == Qt::Checked)
        {
            bool is_color = false;
            if(var_value.left(1) == "#")
                is_color = true;
            else if(QColor::colorNames().contains(var_value, Qt::CaseInsensitive))
                is_color = true;

            if(is_color)
            {
                hash_items << var_value_item;

                QColor color(var_value);
                hash_colors[order] = color;
                order_list << order;
                ++order;
            }
        }
    }

    // now rearrange the colors
    // ... if only two colors have been checked
    if(order_list.count() == 2)
    {
        int order[] = { order_list[0], order_list[0] };
        QString color_name[] = { hash_colors[order[0]].name(), hash_colors[order[1]].name() };

        hash_items[0]->setData(color_name[1], Qt::DisplayRole);
        hash_items[1]->setData(color_name[0], Qt::DisplayRole);
    }

    // ... if more than two colors have been checked
    if(order_list.count() > 2)
    {
        std::random_shuffle(order_list.begin(), order_list.end());

        for(int i = 0; i < order_list.count(); ++i)
        {
            int order = order_list[i];
            QString color_name = hash_colors[order].name();
            hash_items[i]->setData(color_name, Qt::DisplayRole);
        }
    }

    // apply if live is true
    if(this->m_live_preview)
    {
        emit this->styleSheetReady(this->generateStyleSheet());
    }
}

void StyleSheetEditorWidget::generateRandomColorScheme()
{
    DialogColorSpec* dlg = new DialogColorSpec(m_workspace);
    std::vector<int> hr, sr, vr;
    QMap<int, QColor> scheme;
    if(dlg->exec())
    {
//        hr = dlg->hue_range();
//        sr = dlg->saturation_range();
//        vr = dlg->value_range();
        scheme = dlg->colorScheme();
    }
    else
    {
        return;
//        hr = {0, 255};
//        sr = {0, 255};
//        vr = {0, 255};
    }
    delete dlg;
    dlg = Q_NULLPTR;

    // get the scheme
    QList<int> order = scheme.keys();

    // check if duplicate varaible names should be overwritten
    typedef QPair<QStandardItem*,QStandardItem*> ItemPair;
    QList<ItemPair> items = this->getColorItems();
    QStringList dup_var_names;
    QList<QStandardItem*> dup_var_value_items;
    for(ItemPair item_pair: items)
    {
        if(item_pair.first->text().trimmed().left(10) == "var_color_")
        {
            dup_var_names << item_pair.first->data(Qt::EditRole).toString();
            dup_var_value_items << item_pair.second;
        }
    }

    int replace_duplicates = QMessageBox::Yes;
    if(dup_var_names.count() > 0)
    {
        QMessageBox msgBox;
        msgBox.setText("There are duplicate variable names.");
        msgBox.setInformativeText("Do you want to overwrite the value of the duplicate variables?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        replace_duplicates = msgBox.exec();
    }

    // add the color scheme to the m_vars_model.
    // the value of duplicate variables will be replaced if replace_duplicates == 1

    QString var_name;
    QColor var_color;
    int ci = 0;

    // ... don't replace duplicate get the last color variable index
    if(replace_duplicates == QMessageBox::No)
    {
        // ... get last color index
        for(QString var_name: dup_var_names)
        {
            QStringList sp = var_name.split('_', QString::SkipEmptyParts); // e.g., var_color_23
            QString sindex = sp.last();
            ci = ci < sindex.toInt() ? sindex.toInt() : ci;
        }

        for(auto key: order)
        {
            var_name = QString("var_color_%0").arg(++ci);
            var_color = scheme[key];
            this->addVariable(var_name, var_color.name());

            // add the variable name to the word list
            if(!m_words.contains(var_name))
                m_words.append(var_name);
        }

        // update the completer
        this->updateCompleterWordList();
    }
    // ... replace duplicate color variables
    else
    {
        for(auto key: order)
        {
            var_name = QString("var_color_%0").arg(ci++);
            var_color = scheme[key];

            // look for duplicate variable
            int i = dup_var_names.indexOf(var_name);

            // ... duplicate found, therefore replace value of duplicate
            if(i >= 0)
            {
                dup_var_value_items[i]->setData(var_color.name(), Qt::DisplayRole);
            }
            // ... no duplicate available so add variable
            else
            {
                this->addVariable(var_name, var_color.name());

                // add the variable name to the word list
                if(!m_words.contains(var_name))
                    m_words.append(var_name);
            }
        }

        // update the completer
        this->updateCompleterWordList();
    }

    // apply if live is true
    if(this->m_live_preview)
    {
        emit this->styleSheetReady(this->generateStyleSheet());
    }
}

void StyleSheetEditorWidget::on_comboBoxOrdering_activated(const QString &arg1)
{
    Q_UNUSED(arg1)

    this->rearrangeColorScheme();

    // apply if live is true
    if(this->m_live_preview)
    {
        emit this->styleSheetReady(this->generateStyleSheet());
    }
}

bool StyleSheetEditorWidget::eventFilter(QObject* obj, QEvent* event)
{
//    std::cout << "Filter" << std::endl;
    if(event->type() == QEvent::Drop)
    {
        std::cout << "Dropped" << std::endl;
//        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
//        qDebug("Ate key press %d", keyEvent->key());
        return true;
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void StyleSheetEditorWidget::on_btnAddSnippet_clicked()
{
    if(m_snippet_model == Q_NULLPTR)
    {
        std::cout << "StyleSheetEditorWidget: The snippet model is null" << std::endl;
        return;
    }

    QString snippet_name = "snippet_name";
    this->addSnippet(snippet_name, "");
}

void StyleSheetEditorWidget::on_btnSnippetUp_clicked()
{
    QModelIndex index = ui->treeViewSnippets->currentIndex();
    if(index.isValid())
    {
        if(index.row() > 0)
        {
            QList<QStandardItem*> items = m_snippet_model->takeRow(index.row());
            m_snippet_model->insertRow(index.row() - 1, items);
        }
    }
}

void StyleSheetEditorWidget::on_btnSnippetDown_clicked()
{
    QModelIndex index = ui->treeViewSnippets->currentIndex();
    if(index.isValid())
    {
        if(index.row() < (m_snippet_model->rowCount() - 1))
        {
            QList<QStandardItem*> items = m_snippet_model->takeRow(index.row());
            m_snippet_model->insertRow(index.row() + 1, items);
        }
    }
}

void StyleSheetEditorWidget::on_btnRemoveSnippet_clicked()
{
    // get checked rows
    QList<int> items;
    for(int i = 0; i < m_snippet_model->rowCount(); ++i)
    {
        QStandardItem* snippet_name_item = m_snippet_model->item(i, 0);
        if(snippet_name_item->checkState() == Qt::Checked)
        {
            items << i;

            // remove the snippet from the completer's word list
            int wi = m_words.indexOf(snippet_name_item->text());
            m_words.removeAt(wi);
        }
    }

    // remove the rows from the back of m_snippet_model
    // removing rows from the front will result in the changing of row indeces
    std::sort(items.begin(), items.end(), std::greater<int>());
    for(int row_index: items)
    {
        m_snippet_model->removeRow(row_index);
    }

    // update the completer
    this->updateCompleterWordList();
}

void StyleSheetEditorWidget::reset()
{
    // clear and reset the data models
    this->m_vars_model->clear();
    this->m_vars_model->setColumnCount(2);
    this->m_vars_model->setHeaderData(0, Qt::Horizontal, "Variable");
    this->m_vars_model->setHeaderData(1, Qt::Horizontal, "Value");

    this->m_snippet_model->clear();
    this->m_snippet_model->setColumnCount(2);
    this->m_snippet_model->setHeaderData(0, Qt::Horizontal, "Snippet");
    this->m_snippet_model->setHeaderData(1, Qt::Horizontal, "Value");

    this->m_page_model->clear();
    this->m_page_model->setColumnCount(2);
    this->m_page_model->setHeaderData(0, Qt::Horizontal, "Page");
    this->m_page_model->setHeaderData(1, Qt::Horizontal, "Style sheet");

    ui->treeViewPage->setColumnHidden(1, true);

    // reset the text editor
    this->m_text_editor->clear();
}

void StyleSheetEditorWidget::on_btnAddBoilerPlatePages_clicked()
{
    DialogPageCreator dlg;
    if(dlg.exec()){
        for(QString name: dlg.pageNames()){
            this->addPage(name, "", true);
        }
    }
}
