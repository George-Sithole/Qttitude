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

#ifndef STYLESHEETEDITORWIDGET_H
#define STYLESHEETEDITORWIDGET_H

// Qt Libraries
#include <QWidget>

// Local Libraries


class QStandardItemModel;
class QStandardItem;
class QCompleter;
class QStringListModel;
class QMainWindow;

class Highlighter;
class TextEditor;
class Workspace;


namespace Ui {
class StyleSheetEditorWidget;
}

class StyleSheetEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StyleSheetEditorWidget(Workspace* workspace, QWidget *parent = nullptr);
    ~StyleSheetEditorWidget();

    enum Mode{ReadUnknown, ReadVariables, ReadSnippets, ReadStyleSheets};
    enum SchemeOrder{Reverse_Ordering, HSV_Ordering, HVS_Ordering, SVH_Ordering,
                     SHV_Ordering, VHS_Ordering, VSH_Ordering, Random_Ordering};

    void saveStyleSheet(const QString& filename = "");

    void setMainWindow(QMainWindow* main_window);

    void reset();

    QJsonArray variablesJson();

    void addVariablesFromJson(const QJsonValue& variables_value);

    QJsonArray snippetsJson();

    void addSnippetsFromJson(const QJsonValue& snippets_value);

    QJsonArray pagesJson();

    void addPagesFromJson(const QJsonValue& pages_value);

    void generateRandomColorScheme();

    void generateColorSchemeFromImageFile();

    QString generateStyleSheet();

signals:
    void styleSheetReady(QString);

public slots:
    void setLivePreview(const bool& state);

private slots:

    void on_btnAddVar_clicked();

    void on_btnRemoveVar_clicked();

    void processVarsModelDataChanged(const QModelIndex &topLeft,
                                     const QModelIndex &bottomRight,
                                     const QVector<int> &roles = QVector<int> ());

    void processSnippetModelDataChanged(const QModelIndex &topLeft,
                                        const QModelIndex &bottomRight,
                                        const QVector<int> &roles = QVector<int> ());


    void on_btnClear_clicked();

    void on_btnApplyStyleSheet_clicked();

    void on_btnAddPage_clicked();

    void on_btnRemovePage_clicked();

    void on_btnPageUp_clicked();

    void on_btnPageDown_clicked();

    void on_treeViewPage_clicked(const QModelIndex &index);

    void processTextChanged();

    void on_treeViewVars_doubleClicked(const QModelIndex &index);

    void on_treeViewSnippets_doubleClicked(const QModelIndex &index);

    void on_btnVarUp_clicked();

    void on_btnVarDown_clicked();

    void on_checkBoxSelect_clicked();

    void on_btnRandomShuffle_clicked();

    void on_comboBoxOrdering_activated(const QString &arg1);

    void on_btnAddSnippet_clicked();

    void on_btnSnippetUp_clicked();

    void on_btnSnippetDown_clicked();

    void on_btnRemoveSnippet_clicked();

protected:
    void addVariable(const QString& var_name, const QString& var_value, const bool& ischecked = true);
    void addSnippet(const QString& snippet_name, const QString& snippet_value, const bool& ischecked = true);
    QString addPage(const QString& pagename, const QString& qss ="", const bool& ischecked = true);

    void setupCompleter();


    QStringList getMainWindowObjects();

    QList<QPair<QStandardItem*,QStandardItem*>> getColorItems();

    inline int toHex(const int& a, const int& b, const int& c)
    { return ((a & 0xffu) << 16) | ((b & 0xffu) << 8) | (c & 0xffu);}
    int hsv(QColor& c){return toHex( c.hue(), c.saturation(), c.value() );}
    int hvs(QColor& c){return toHex( c.hue(), c.value(), c.saturation() );}
    int shv(QColor& c){return toHex( c.saturation(), c.hue(), c.value() );}
    int svh(QColor& c){return toHex( c.saturation(), c.value(), c.hue() );}
    int vhs(QColor& c){return toHex( c.value(), c.hue(), c.saturation() );}
    int vsh(QColor& c){return toHex( c.value(), c.saturation(), c.hue() );}

    void updateCompleterWordList();

    void rearrangeColorScheme();

    bool eventFilter(QObject* obj, QEvent* event);

    QString replaceWithSnippet(const QString& text);
    QString replaceWithVariables(const QString& text);

private:
    Ui::StyleSheetEditorWidget *ui;

    QStandardItemModel* m_vars_model;

    QStandardItemModel* m_snippet_model;

    QStandardItemModel* m_page_model;

    QStringListModel* m_completer_model;

    Highlighter* m_highlihter;

    QString m_qss_filename; // TODO delete

    int m_read_mode;

    bool m_live_preview;

    QCompleter* m_completer;

    TextEditor* m_text_editor;

    QString m_old_var_name;

    QString m_old_snippet_name;

    QMainWindow* m_main_window;

    QStringList m_words;

    Workspace* m_workspace;

};

#endif // STYLESHEETEDITORWIDGET_H
