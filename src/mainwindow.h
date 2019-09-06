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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


// Qt Libraries
#include <QMainWindow>
#include <QProcess>
#include <QMap>
#include <QModelIndex>


namespace Ui {
class MainWindow;
}


class QCloseEvent;
class QStandardItemModel;
class QStandardItem;

class Workspace;
class Project;
class StyleSheetServer;
class DragItemModel;
class StyleSheetEditorWidget;


struct AppObject
{
    QStandardItem* item_name;
    QStandardItem* item_class;
    QWidget* widget = nullptr;
    QLayout* layout = nullptr;
    QString obj_name = "";
    QString obj_class = "";
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void writeSettings();
    void readSettings();

    void closeEvent(QCloseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief reset
     * @return QMessageBox::Save, QMessageBox::Cancel, QMessageBox::Discard
     */
    int reset();

    // ------------------------------------
    // Files functions
    void setupFilesWidgets();

    bool addUiFile(const QString& filename, const bool& visible = true);

    void addUiFiles(const QStringList& filenames = QStringList());

    void removeUiFile(const QString& ui_filename);

    void removeSelectedUiFiles();

    void destroyAllDockWidgets();

    void addWidgetToWidgetsModel(QWidget* parent, QStandardItem* parent_item);

    void addLayoutToWidgetsModel(QLayout* layout, QStandardItem* parent_item);

    void setAppWidgetsTooltip(QWidget* parent);

    void setParentObjectsTooltip(QObject* parent);

    QJsonArray uiFilesJson();

    QJsonObject uiChangesJson();

    void addUiFilesFromJson(const QJsonValue& uifiles_arr);

    void saveStyleSheetProject(const QString& filename = "");

    void openStyleSheetProject(const QString& filename = "");

    void parseWidgetJson(QWidget* widget, QJsonArray& changes_arr);

    void parseLayoutJson(QLayout* layout, QJsonArray& changes_arr);

    // ------------------------------------
    // Widgets functions

    void setupWidgetModel();

    void resetWidgetsModel();

    AppObject selectedAppObject();

    void setUiMarginsSpacingFromJson(QDockWidget* dw, const QJsonArray& widgets_arr);

    /**
     * @brief selectedDockWidget
     * @return The dockwidget that is selected in the ui tree view
     */
    QDockWidget* selectedDockWidget();

    // ------------------------------------
    // Event filter functions
    bool eventFilter(QObject *obj, QEvent *event) override;


private slots:
    void applyStyleSheet(const QString& style_sheet);

    void setIcons();

    // ------------------------------------
    // Files functions
    void on_treeViewFiles_clicked(const QModelIndex &index);

    void on_treeViewFiles_doubleClicked(const QModelIndex &index);

    // ------------------------------------
    // Filing functions

    void on_actionNew_triggered();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_actionSave_As_triggered();

    void on_action_Export_triggered();

    void on_actionExit_triggered();

    void on_actionContent_triggered();

    void on_actionAbout_triggered();

    void on_btnAddUiFile_clicked();

    void on_btnRemoveUiFile_clicked();

    void on_actionColor_Scheme_From_Image_triggered();

    void on_actionCholor_Scheme_Generator_triggered();

    void on_actionLicense_triggered();

    void on_actionOnline_triggered();

    void on_treeViewAppWidgets_clicked(const QModelIndex &index);

    // ------------------------------------
    // Widget functions

    void setActiveWidgetContentsMargin();

    void setActiveWidgetContentBoxSpacing();

    void setActiveWidgetContentFormSpacing();

    void setActiveWidgetContentGridSpacing();

    void setActiveWidgetContentStackedSpacing();

private:
    Ui::MainWindow *ui;

    Workspace* m_workspace;

    DragItemModel* m_widgets_model;

    StyleSheetEditorWidget* m_se_widget;

    QMap<QString, QDockWidget*> m_ui_dockwidgets_map; // the key is the filename of the ui file

    QStandardItemModel* m_files_model;

    Project* m_project;

    QList<QStandardItem*> m_altered_items;
};

#endif // MAINWINDOW_H
