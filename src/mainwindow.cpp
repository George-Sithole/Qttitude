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
#include <stdio.h>
#include <string>
#include <vector>

// Qt Libraries
#include <QGridLayout>
#include <QFormLayout>
#include <QStackedLayout>
#include <QFile>
#include <QFileDialog>
#include <QProcess>
#include <QSettings>
#include <QCloseEvent>
#include <QStandardItemModel>
#include <QTextStream>
#include <QUiLoader>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QDesktopServices>
#include <QVector>


// Local Libraries
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stylesheeteditorwidget.h"
#include "dragitemmodel.h"
#include "workspace.h"
#include "project.h"
#include "codegen.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    this->setIcons();

    // create a workspace
    this->m_workspace = new Workspace(this);
    this->setWindowTitle(this->m_workspace->appWindowTitle());

    // create the project object
    this->m_project = new Project(this);

    // add style sheet widget
    QGridLayout* layout = new QGridLayout(ui->widget);
    layout->setMargin(0);

    this->m_se_widget = new StyleSheetEditorWidget(m_workspace,ui->centralWidgetMain);
    this->m_se_widget->setObjectName("StyleSheetEditorWidget");
    this->m_se_widget->setMainWindow(nullptr);
    layout->addWidget(this->m_se_widget, 0, 0, 1, 1);

    // read settings
    this->readSettings();

    // initialize the files tree view
    this->setupFilesWidgets();

    // initialize the server
    this->setupWidgetModel(); // TODO change the name of this function from setupStyleSheetServer to something else
    ui->grpboxMargins->setHidden(true);
    ui->grpboxSpacing->setHidden(true);
    ui->grpboxHVSpacing->setHidden(true);

    // set the stylesheet
    QFile file(":/stylesheeteditor/style_sheet.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString style_sheet = file.readAll();
        ui->centralWidgetMain->setStyleSheet(style_sheet);
        ui->dockWidget->setStyleSheet(style_sheet);
    }
    else
    {
        std::cout << "Style sheet style_sheet.qss not found" << std::endl;
    }
    file.close();

    // initialise the live preview
    ui->actionLive_Preview->setChecked(true);
    this->m_se_widget->setLivePreview(ui->actionLive_Preview->isChecked());

    // initialise the outputs
    ui->actionCpp->setChecked(true);
    ui->actionPython->setChecked(true);

    // setup connections
    connect(this->m_se_widget, SIGNAL(styleSheetReady(QString)), this, SLOT(applyStyleSheet(QString)));
    connect(ui->actionLive_Preview, SIGNAL(triggered(bool)), this->m_se_widget, SLOT(setLivePreview(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier)
    {
        switch (event->key()) {
        case Qt::Key_N: this->reset(); break;
        case Qt::Key_O: this->openStyleSheetProject(); break;
        case Qt::Key_S: this->saveStyleSheetProject(this->m_project->projectFilename()); break; // save
        case Qt::Key_A: this->saveStyleSheetProject(); break; // save as
        case Qt::Key_E: this->m_se_widget->saveStyleSheet(); break; // export
        case Qt::Key_X: this->close(); break; // exit
        default: break;
        }
    }
}

void MainWindow::writeSettings()
{
    QSettings settings("my_application.ini", QSettings::IniFormat);

    settings.beginGroup("MainWindow");
    settings.setValue("size", this->size());
    settings.setValue("pos", this->pos());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("my_application.ini", QSettings::IniFormat);

    settings.beginGroup("MainWindow");
    this->resize(settings.value("size", QSize(400, 400)).toSize());
    this->move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(this->reset() == QMessageBox::Cancel){
        event->ignore(); // don't close the application
    } else {
        event->accept(); // close the application
    }
}

// -------------------------------------
//         Filename treeview
// -------------------------------------

void MainWindow::setupFilesWidgets()
{
    // setup model
    m_files_model = new QStandardItemModel(this);
    m_files_model->setColumnCount(1);
    m_files_model->setHeaderData(0, Qt::Horizontal, "Filename");
//    m_files_model->setHeaderData(1, Qt::Horizontal, "Path");

    ui->treeViewFiles->setModel(m_files_model);

    // install an event filter on the tree view
    ui->treeViewFiles->installEventFilter(this);
//    ui->treeViewAppWidgets->setIndentation(16);
}

bool MainWindow::addUiFile(const QString& filename, const bool& visible)
{
    if (not this->m_ui_dockwidgets_map.contains(filename)) {
        QFile file(filename);

        // load the user interface into a QWidget
        file.open(QFile::ReadOnly | QIODevice::Text);
        QUiLoader loader;
        QWidget* widget = loader.load(&file);
        widget->setStyleSheet(" ");
        this->setAppWidgetsTooltip(widget);
        file.close();

        // create a dock widget
        QDockWidget* dockwidget = new QDockWidget(this);
        dockwidget->setProperty("_george_ui_istypeDockWiget", true); // this is used in the event filter to test when the widget is hidden
        dockwidget->setWidget(widget);
        dockwidget->setWindowTitle(filename);
        dockwidget->setFloating(true);
        dockwidget->setStyleSheet(" ");
        this->addDockWidget(Qt::RightDockWidgetArea, dockwidget);
        dockwidget->installEventFilter(this); // install an event filter to monitor when the widget is hidden

        // add the widget to the dock widget
        this->m_ui_dockwidgets_map[filename] = dockwidget;

        // add a reference to the widget in the tree view
        QStandardItem* item0 = new QStandardItem(QFileInfo(filename).fileName());
        item0->setEditable(false);
        item0->setCheckable(true);
        item0->setCheckState(visible ? Qt::Checked : Qt::Unchecked);
        item0->setToolTip(filename);

//        QStandardItem* item1 = new QStandardItem(filename);
//        item1->setEditable(false);

//        this->m_files_model->appendRow({item0, item1});
        this->m_files_model->appendRow(item0);

        // show or hide the dockwidget
        if (visible) {
            dockwidget->show();
        }
        else {
            dockwidget->hide();
        }

        // initialise the dockwidgets style
        dockwidget->setStyleSheet(" ");
        widget->setStyleSheet(" ");

        // set the widgets connections
//            connect(dockwidget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed(QObject*)));

        return true;
    }
    else {
        std::cout << "The user interfact " << filename.toStdString() << " is already in the map" << std::endl;
    }

    return false;
}

void MainWindow::addUiFiles(const QStringList& filenames)
{
    QStringList fnames = filenames.count() > 0 ? filenames : QFileDialog::getOpenFileNames(this, tr("Open User Interface"), this->m_project->workingDir(), tr("User Interface Files, *.ui (*.ui)"));

    foreach(QString f, fnames)
    {
        this->addUiFile(f);
    }
}

void MainWindow::removeUiFile(const QString& ui_filepath)
{
    // remove the ui widget from the widgets map and then delete the ui widget
    QDockWidget* dockwidget = this->m_ui_dockwidgets_map.take(ui_filepath);
    dockwidget->hide(); // always hide the dockwidget before removing it
    dockwidget->deleteLater(); // delete the widget

    // find the file item in the files model
    QList<QStandardItem*> items;
    for(int row = 0; row < this->m_files_model->rowCount(); ++row){
        QStandardItem* item = this->m_files_model->item(row, 0);
        if(item->toolTip() == ui_filepath)
            items.push_back(item);
    }

    // remove the widget from the files model
    if(items.count() > 0){
        QStandardItem* item = this->m_files_model->item(items[0]->row(), 0);
        this->m_files_model->removeRow(item->row(), this->m_files_model->invisibleRootItem()->index()); // remove the widget from the model
    }
}

void MainWindow::removeSelectedUiFiles()
{
    // clear and reset the data models
    this->resetWidgetsModel();

    // remove the ui file from the tree view
    QModelIndexList idxlist = ui->treeViewFiles->selectionModel()->selectedRows(0);
    for(int i = 0; i < idxlist.count(); ++i){
        QString ui_filepath = idxlist[0].data(Qt::ToolTipRole).toString();
        this->removeUiFile(ui_filepath);
    }
}

void MainWindow::destroyAllDockWidgets()
{
    foreach(QDockWidget* dw, this->m_ui_dockwidgets_map)
    {
        dw->hide();
        dw->deleteLater();
    }
    this->m_ui_dockwidgets_map.clear();
    this->m_files_model->clear();
}

void MainWindow::addWidgetToWidgetsModel(QWidget* parent, QStandardItem* parent_item)
{
    // check that the widget is not null
    if(! parent) {
        return;
    }

    QStandardItem* item_widget_name = new QStandardItem(parent->objectName());
    QStandardItem* item_widget_class = new QStandardItem(parent->metaObject()->className());
    item_widget_name->setEditable(false);
    item_widget_class->setEditable(false);
    parent_item->appendRow({item_widget_name, item_widget_class});

    // test if the widgets margins were changed
    QVariant prop = parent->property("_g_margin_edit");
    if(prop.isValid()){
        item_widget_name->setIcon(QIcon(":/icons/star.svg"));
    }

//item_widget_name->setBackground(QBrush("red"));
    // add the widget's layout to the widgets model
    if(parent->layout()) {
        this->addLayoutToWidgetsModel(parent->layout(), item_widget_name);
    }
}

void MainWindow::addLayoutToWidgetsModel(QLayout* layout, QStandardItem* parent_item)
{
    // check that the widget has a layout
    if(! layout) {
        return;
    }

    QStandardItem* item_layout_name = new QStandardItem(layout->objectName());
    QStandardItem* item_layout_class = new QStandardItem(layout->metaObject()->className());
    item_layout_name->setEditable(false);
    item_layout_class->setEditable(false);
    parent_item->appendRow({item_layout_name, item_layout_class});


    // test if the widgets margins were changed
    QVariant mprop = layout->property("_g_margin_edit");
    QVariant sprop = layout->property("_g_spacing_edit");
    if(mprop.isValid() || sprop.isValid()){
        item_layout_name->setIcon(QIcon(":/icons/star.svg"));
    }

    // loop through the layout
    for(int i = 0; i < layout->count(); ++i)
    {
        QLayoutItem* layout_item = layout->itemAt(i);
        if(layout_item){
            if(! layout_item->isEmpty()) {
                if(layout_item->widget()){
                    this->addWidgetToWidgetsModel(layout_item->widget(), item_layout_name);
                }
                else if (layout_item->layout()) {
                    this->addLayoutToWidgetsModel(layout_item->layout(), item_layout_name);
                }
                else if (layout_item->spacerItem()) {
                    QStandardItem* item_spacer_name = new QStandardItem("Spacer");
                    QStandardItem* item_spacer_class = new QStandardItem("QSpacerItem");
                    item_spacer_name->setEditable(false);
                    item_spacer_class->setEditable(false);
                    item_layout_name->appendRow({item_spacer_name, item_spacer_class});
                }
            }
        }
    }
}

void MainWindow::setAppWidgetsTooltip(QWidget* parent)
{
    if(parent != Q_NULLPTR)
    {
        this->setParentObjectsTooltip(parent);
    }
}

void MainWindow::setParentObjectsTooltip(QObject* parent)
{
    // write parent
    QString object_name = parent->objectName();
    QString class_name = parent->metaObject()->className();
    if(parent->isWidgetType())
    {
        QWidget* widget = qobject_cast<QWidget*>(parent);
        widget->setToolTip("<b>Class</b>: " + class_name +
                           "\n<b>Object Name</b>: " + object_name);
    }

    // write the children
    for(QObject* child: parent->children())
    {
        this->setParentObjectsTooltip(child);
    }
}

void MainWindow::on_treeViewFiles_clicked(const QModelIndex &index)
{
    if(index.isValid())
    {
        QStandardItem* item = m_files_model->item(index.row(), 0);
        QString key = item->toolTip();
        QDockWidget* dockwidget = this->m_ui_dockwidgets_map[key];

        this->resetWidgetsModel();
//        this->addWidgetToWidgetsModel(dockwidget->widget(), this->m_widgets_model->invisibleRootItem());
        this->addWidgetToWidgetsModel(dockwidget->widget(), this->m_widgets_model->invisibleRootItem());
        ui->treeViewAppWidgets->expandAll();
    }
}

void MainWindow::on_treeViewFiles_doubleClicked(const QModelIndex &index)
{
    if(index.isValid())
    {
        QString key = index.data(Qt::ToolTipRole).toString();
        QDockWidget* dockwidget = this->m_ui_dockwidgets_map[key];
        dockwidget->show();

        QStandardItem* item = this->m_files_model->item(index.row(), 0);
        item->setCheckState(dockwidget->isHidden() ? Qt::Unchecked : Qt::Checked);
    }
}

// -------------------------------------
//         Widgets functions
// -------------------------------------

void MainWindow::setupWidgetModel() // TODO rename this function
{
    m_widgets_model = new DragItemModel(this);
    m_widgets_model->setColumnCount(2);
    m_widgets_model->setHeaderData(0, Qt::Horizontal, "Object");
    m_widgets_model->setHeaderData(1, Qt::Horizontal, "Class");

    ui->treeViewAppWidgets->setModel(m_widgets_model);
    ui->treeViewAppWidgets->setIndentation(16);
}

void MainWindow::resetWidgetsModel()
{
    this->m_widgets_model->clear();
    this->m_widgets_model->setColumnCount(2);
    this->m_widgets_model->setHeaderData(0, Qt::Horizontal, "Object");
    this->m_widgets_model->setHeaderData(1, Qt::Horizontal, "Class");
}

// -------------------------------------
//         Event filter functions
// -------------------------------------

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    bool processed = false;

    QList<QByteArray> dp = obj->dynamicPropertyNames();
    bool isDockWidget = dp.indexOf("_george_ui_istypeDockWiget") > -1 ? true: false;

    if (event->type() == QEvent::KeyPress and strcmp(obj->metaObject()->className(), "QTreeView") == 0) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Delete){
            this->removeSelectedUiFiles();
            processed = true;
        }
        else if (keyEvent->key() == Qt::Key_Insert) {
            this->addUiFiles();
            processed = true;
        }
    }
    else if (event->type() == QEvent::Hide and isDockWidget) {
        if(obj != nullptr) // calling deleteLater on a dockwidget triggers the Hide event of the dockwidget. this check ensures that the dockwidget exists before it is used
        {
            QDockWidget* dockwidget = qobject_cast<QDockWidget *>(obj);
            QString title = dockwidget->windowTitle(); // the title contains the ui filename

            // find the file item in the ui file list ...
            for(int row = 0; row < this->m_files_model->rowCount(); ++row){
                QStandardItem* item = this->m_files_model->item(row, 0);
                if(item->toolTip() == title){
                    bool dw_hidden = dockwidget->isHidden();

                    // ... update the check state of the item
                    item->setCheckState( dw_hidden ? Qt::Unchecked : Qt::Checked );
                    processed = true;
                }
            }

//            // get the selected index in the files list view
//            QModelIndexList idxlist = ui->treeViewFiles->selectionModel()->selectedRows(0);
//            QString current_selected_ui_file;
//            if (idxlist.count() > 0) {
//                current_selected_ui_file = idxlist[0].data(Qt::ToolTipRole).toString();
//            }

//            // synch the state of the dockwidget to the widgets model
//            if(items.count() > 0){
//                QStandardItem* item = this->m_files_model->item(items[0]->row(), 0);
//                if(dockwidget->isHidden()){
//                    item->setCheckState(Qt::Unchecked);

//                    // clear the widgets tree view
//                    this->resetWidgetsModel();
//                }
//                else {
//                    item->setCheckState(Qt::Checked);

//                    // update the tree view containing the widget's layout
//                    this->resetWidgetsModel();
//                    this->addWidgetToWidgetsModel(dockwidget->widget(), this->m_widgets_model->invisibleRootItem());
//                    ui->treeViewAppWidgets->expandAll();
//                }

//                processed = true;
//            }
        }
    }

    if(processed) {
        return true;
    }
    else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void MainWindow::applyStyleSheet(const QString& style_sheet)
{
    // update the state of the project
    this->m_project->setIsSaved(false);
    this->setWindowTitle(this->m_workspace->appWindowTitle(this->m_project->projectFilename(), false));

    // apply the stylesheet
    foreach(QDockWidget* dw, this->m_ui_dockwidgets_map)
    {
        dw->widget()->setStyleSheet(style_sheet);
    }
}

// -------------------------------------
//         Filing functions
// -------------------------------------

void MainWindow::on_actionNew_triggered()
{
    this->reset();
}

void MainWindow::on_actionOpen_triggered()
{
//    this->m_se_widget->openStyleSheetProject();
    this->openStyleSheetProject();
}


void MainWindow::on_actionSave_triggered()
{
    this->saveStyleSheetProject(this->m_project->projectFilename());
}

void MainWindow::on_actionSave_As_triggered()
{
    this->saveStyleSheetProject();
}

void MainWindow::on_action_Export_triggered()
{
    // write the style sheet
    if(this->m_se_widget->saveStyleSheet()) {
        // generate styling code for the widgets
        foreach(QDockWidget* dw, this->m_ui_dockwidgets_map)
        {
            if(ui->actionCpp) CodeGen::generate(dw->widget(), "C++");
            if(ui->actionPython) CodeGen::generate(dw->widget(), "Python");
        }
    }
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionContent_triggered()
{

}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msg_box;
    msg_box.setWindowTitle("About");
    msg_box.setText(this->m_workspace->aboutHTML());
    msg_box.setTextFormat(Qt::RichText);
    QPixmap pixmap = QPixmap(":/icons/icon_turqosie.png").scaledToWidth(150);
    msg_box.setIconPixmap(pixmap);
    msg_box.exec();
}

void MainWindow::on_btnAddUiFile_clicked()
{
    this->addUiFiles();
}

void MainWindow::on_btnRemoveUiFile_clicked()
{
    this->removeSelectedUiFiles();
}

void MainWindow::on_actionColor_Scheme_From_Image_triggered()
{
    this->m_se_widget->generateColorSchemeFromImageFile();
}

QJsonArray MainWindow::uiFilesJson()
{
    QJsonArray uifiles_arr;
    for(int i = 0; i < this->m_files_model->rowCount(); ++i)
    {
        QStandardItem* uifile_item = this->m_files_model->item(i, 0);
        QString uifile_name = uifile_item->text(); // this->m_files_model->index(i, 0).data(Qt::DisplayRole).toString();
        QString uifile_path = uifile_item->toolTip(); // this->m_files_model->index(i, 1).data(Qt::DisplayRole).toString();

        QRect rect = this->m_ui_dockwidgets_map[uifile_path]->geometry();

        QJsonObject file_obj;
        file_obj["filename"] = uifile_name;
        file_obj["checked"] = uifile_item->checkState() == Qt::Checked ? true : false;
        file_obj["path"] = uifile_path;
        file_obj["x"] = rect.x();
        file_obj["y"] = rect.y();
        file_obj["w"] = rect.width();
        file_obj["h"] = rect.height();

        uifiles_arr.append(file_obj);
    }

    return uifiles_arr;
}

void MainWindow::addUiFilesFromJson(const QJsonValue& uifiles_value)
{
    QJsonArray uifiles_arr = uifiles_value.toArray();
    foreach(QJsonValue file_value, uifiles_arr)
    {
        QJsonObject uifile_obj = file_value.toObject();
        QString uifile_path = uifile_obj["path"].toString();
        bool visible = uifile_obj["checked"].toBool();
        int x = uifile_obj["x"].toInt();
        int y = uifile_obj["y"].toInt();
        int w = uifile_obj["w"].toInt();
        int h = uifile_obj["h"].toInt();

        if(this->addUiFile(uifile_path, visible)){
            this->m_ui_dockwidgets_map[uifile_path]->setGeometry(x, y, w, h);
        }
    }
}

void MainWindow::saveStyleSheetProject(const QString& filename)
{
    // test if the local filename is empty
    QString local_filename = filename;
    if(local_filename.isEmpty()) {
        local_filename = QFileDialog::getSaveFileName(this, tr("Project File"), this->m_project->workingDir(), tr("Project File, *.proj (*.proj)"));
        if(local_filename.isEmpty()) {
            return;
        }
    }

    this->m_project->setProjectFilename(local_filename);
    QFile file(this->m_project->projectFilename());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QJsonObject root_obj;
        root_obj["version"] = this->m_workspace->version();
        root_obj["uifiles"] = this->uiFilesJson();
        root_obj["variables"] = this->m_se_widget->variablesJson();
        root_obj["snippets"] = this->m_se_widget->snippetsJson();
        root_obj["pages"] = this->m_se_widget->pagesJson();
        root_obj["uichanges"] = this->uiChangesJson();

        QJsonDocument json_doc(root_obj);
        QByteArray json = json_doc.toJson(QJsonDocument::Indented);

        file.write(json);
        qDebug() << "Project saved";
    }
    file.close();

    // update the state of the project
    this->m_project->setIsSaved(true);
    this->setWindowTitle(this->m_workspace->appWindowTitle(this->m_project->projectFilename(), true));
}

int MainWindow::reset()
{
    int result = QMessageBox::Discard;
    if(not this->m_project->isSaved()){
        QMessageBox msgBox;
        msgBox.setText("The project has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        result = msgBox.exec();
    }

    switch (result) {
    case QMessageBox::Save:
        this->saveStyleSheetProject(this->m_project->projectFilename());
        this->m_se_widget->reset();
        break;
    case QMessageBox::Discard:
        this->m_se_widget->reset();
        break;
    case QMessageBox::Cancel:
        return result;
    default:
        break;
    }

    // reset the project
    this->m_project->reset();

    // update the state of the project
    this->m_project->setIsSaved(true);
    this->setWindowTitle(this->m_workspace->appWindowTitle());

//    // clear the altered items list
//    this->m_altered_items.clear();

    // clear and reset the data models
    this->m_files_model->clear();
    this->m_files_model->setColumnCount(1);
    this->m_files_model->setHeaderData(0, Qt::Horizontal, "Filename");
//    this->m_files_model->setHeaderData(1, Qt::Horizontal, "Path");

    this->resetWidgetsModel();

    // destroy all the dockwidgets
    foreach(QDockWidget* dw, this->m_ui_dockwidgets_map)
    {
        dw->hide();
        dw->deleteLater();
    }
    this->m_ui_dockwidgets_map.clear();

    return result;
}

void MainWindow::openStyleSheetProject(const QString& filename)
{
    // test if the local filename is empty
    QString local_filename = filename;
    if(local_filename.isEmpty()) {
        local_filename = QFileDialog::getOpenFileName(this, tr("Project File"), this->m_project->workingDir(), tr("Style File, *.proj (*.proj)"));
        if(local_filename.isEmpty()) {
            return;
        }
        else {
            this->m_project->setProjectFilename(local_filename);
        }
    }

    // reset the stylesheet widget editor in preparation for new data
    this->m_se_widget->reset();

    // read project json file
    QByteArray json;
    QFile file(this->m_project->projectFilename());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        json = file.readAll();
    }
    QJsonDocument json_doc = QJsonDocument::fromJson(json);

    QJsonObject root_obj = json_doc.object();

    // add the ui files
    this->addUiFilesFromJson(root_obj.value("uifiles"));

    // initialise the stylesheet editor widget
    this->m_se_widget->addSnippetsFromJson(root_obj.value("snippets"));
    this->m_se_widget->addVariablesFromJson(root_obj.value("variables"));
    this->m_se_widget->addPagesFromJson(root_obj.value("pages"));

    // update the state of the project
    this->m_project->setIsSaved(true);
    this->setWindowTitle(this->m_workspace->appWindowTitle(this->m_project->projectFilename(), true));

    // apply the stylesheet to the widgets
    QString style_sheet = this->m_se_widget->generateStyleSheet();
    foreach(QDockWidget* dw, this->m_ui_dockwidgets_map) {
        dw->widget()->setStyleSheet(style_sheet);
    }

    // apply margins and spacings
    QJsonObject ui_obj = root_obj.value("uichanges").toObject();
    for(QString ui_name: ui_obj.keys()){
        QJsonArray widgets_arr = ui_obj[ui_name].toArray();
        QDockWidget* dw = this->m_ui_dockwidgets_map[ui_name];
        this->setUiMarginsSpacingFromJson(dw, widgets_arr);
    }
}

void MainWindow::setUiMarginsSpacingFromJson(QDockWidget* dw, const QJsonArray& widgets_arr)
{
    for(QJsonValue v: widgets_arr){
        QJsonObject ui_obj = v.toObject();
        QString widget_name = ui_obj["name"].toString();

        // find widget with widget_name
        QObject* obj = dw->findChild<QObject *>(widget_name);
        if(obj){
            if(obj->inherits("QWidget")){
                // set margins
                if(ui_obj.contains("margins")){
                    QVariantList m = ui_obj["margins"].toArray().toVariantList();
                    QWidget* widget = qobject_cast<QWidget*>(obj);
                    widget->setContentsMargins(m[0].toInt(), m[1].toInt(), m[2].toInt(), m[3].toInt());
                }
            } else if (obj->inherits("QLayout")) {
                // set margins
                if(ui_obj.contains("margins")){
                    QVariantList m = ui_obj["margins"].toArray().toVariantList();
                    QLayout* layout = qobject_cast<QLayout*>(obj);
                    layout->setContentsMargins(m[0].toInt(), m[1].toInt(), m[2].toInt(), m[3].toInt());
                }

                // set spacing
                if(ui_obj.contains("spacing")){
                    QString obj_class = obj->metaObject()->className();
                    QVariantList s = ui_obj["spacing"].toArray().toVariantList();

                    if(obj_class == "QBoxLayout"){
                        QBoxLayout* layout = qobject_cast<QBoxLayout*>(obj);
                        layout->setSpacing(s[0].toInt());
                    } else if(obj_class == "QFormLayout"){
                        QFormLayout* layout = qobject_cast<QFormLayout*>(obj);
                        layout->setHorizontalSpacing(s[0].toInt());
                        layout->setVerticalSpacing(s[1].toInt());
                    } else if(obj_class == "QGridLayout"){
                        QGridLayout* layout = qobject_cast<QGridLayout*>(obj);
                        layout->setHorizontalSpacing(s[0].toInt());
                        layout->setVerticalSpacing(s[1].toInt());
                    } else if(obj_class == "QStackedLayout"){
                        QStackedLayout* layout = qobject_cast<QStackedLayout*>(obj);
                        layout->setSpacing(s[0].toInt());
                    }
                }
            }
        } else {
            qDebug() << "Error: openStyleSheetProject: object is null;";
        }
    }
}

void MainWindow::on_actionCholor_Scheme_Generator_triggered()
{
    this->m_se_widget->generateRandomColorScheme();
}

void MainWindow::on_actionLicense_triggered()
{
    QMessageBox msg_box;
    msg_box.setWindowTitle("License");
    msg_box.setText(this->m_workspace->licenseHTML());
    msg_box.setTextFormat(Qt::RichText);
    msg_box.setDetailedText(this->m_workspace->license());
    QPixmap pixmap = QPixmap(":/icons/icon_turqosie.png").scaledToWidth(150);
    msg_box.setIconPixmap(pixmap);
    msg_box.exec();
}

void MainWindow::on_actionOnline_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.geovariant.com/qttitude.html"));
}

void MainWindow::setIcons()
{
    ui->actionNew->setIcon(QIcon(":/icons/file-medical.svg"));
    ui->actionOpen->setIcon(QIcon(":/icons/file-alt.svg"));
    ui->actionOpen->setIcon(QIcon(":/icons/file-download.svg"));
    ui->actionOpen->setIcon(QIcon(":/icons/file-download-as.svg"));
}

void MainWindow::setActiveWidgetContentsMargin()
{
    AppObject app_object = this->selectedAppObject();

    if(app_object.widget){
        app_object.widget->setProperty("_g_margin_edit", true);
        app_object.widget->setContentsMargins(ui->sldr_margin_left->value(), ui->sldr_margin_top->value(),
                                              ui->sldr_margin_right->value(), ui->sldr_margin_bottom->value());

        // set the item's icon
        app_object.item_name->setIcon(QIcon(":/icons/star.svg"));
    } else if (app_object.layout) {
        app_object.layout->setProperty("_g_margin_edit", true);
        app_object.layout->setContentsMargins(ui->sldr_margin_left->value(), ui->sldr_margin_top->value(),
                                              ui->sldr_margin_right->value(), ui->sldr_margin_bottom->value());

        // set the item's icon
        app_object.item_name->setIcon(QIcon(":/icons/star.svg"));
    }
}

void MainWindow::setActiveWidgetContentBoxSpacing()
{
    AppObject app_object = this->selectedAppObject();
    app_object.layout->setProperty("_g_spacing_edit", true);

    QBoxLayout* layout = qobject_cast<QBoxLayout*>(app_object.layout);
    layout->setSpacing(ui->sldr_spacing->value());

    // set the item's icon
    app_object.item_name->setIcon(QIcon(":/icons/star.svg"));
}

void MainWindow::setActiveWidgetContentFormSpacing()
{
    AppObject app_object = this->selectedAppObject();
    app_object.layout->setProperty("_g_spacing_edit", true);

    QFormLayout* layout = qobject_cast<QFormLayout*>(app_object.layout);
    layout->setHorizontalSpacing(ui->sldr_spacing_horizontal->value());
    layout->setVerticalSpacing(ui->sldr_spacing_vertical->value());

    // set the item's icon
    app_object.item_name->setIcon(QIcon(":/icons/star.svg"));
}

void MainWindow::setActiveWidgetContentGridSpacing()
{
    AppObject app_object = this->selectedAppObject();
    app_object.layout->setProperty("_g_spacing_edit", true);

    QGridLayout* layout = qobject_cast<QGridLayout*>(app_object.layout);
    layout->setHorizontalSpacing(ui->sldr_spacing_horizontal->value());
    layout->setVerticalSpacing(ui->sldr_spacing_vertical->value());

    // set the item's icon
    app_object.item_name->setIcon(QIcon(":/icons/star.svg"));
}

void MainWindow::setActiveWidgetContentStackedSpacing()
{
    AppObject app_object = this->selectedAppObject();
    app_object.layout->setProperty("_g_spacing_edit", true);

    QStackedLayout* layout = qobject_cast<QStackedLayout*>(app_object.layout);
    layout->setSpacing(ui->sldr_spacing->value());

    // set the item's icon
    app_object.item_name->setIcon(QIcon(":/icons/star.svg"));
}

QDockWidget* MainWindow::selectedDockWidget()
{
    QModelIndex index = ui->treeViewFiles->selectionModel()->currentIndex();
    if(index.isValid()){
        QString key = index.data(Qt::ToolTipRole).toString();
        return this->m_ui_dockwidgets_map[key];
    } else {
        return  nullptr;
    }
}

AppObject MainWindow::selectedAppObject()
{
    AppObject app_obj;

    // get the current index of widget in the app widgets tree
    QModelIndex index = ui->treeViewAppWidgets->selectionModel()->currentIndex();
    app_obj.item_name = this->m_widgets_model->itemFromIndex(index.sibling(index.row(), 0));
    app_obj.item_class = this->m_widgets_model->itemFromIndex(index.sibling(index.row(), 1));

    if(!app_obj.item_name){
        qDebug() << "Error: " << index;
    }

    app_obj.obj_name = app_obj.item_name->text();
    app_obj.obj_class = app_obj.item_class->text();

    // find the selected widget or layout in the dockwidget
    QDockWidget* dockwidget = this->selectedDockWidget();
    if(dockwidget){
        QObject* obj = dockwidget->findChild<QObject *>(app_obj.obj_name);
        if(obj->inherits("QWidget")){
            app_obj.widget = qobject_cast<QWidget*>(obj);
        } else if (obj->inherits("QLayout")) {
            app_obj.layout = qobject_cast<QLayout*>(obj);
        }
    }

    return app_obj;
}

void MainWindow::on_treeViewAppWidgets_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);

    AppObject app_object = selectedAppObject();

    // disconnect sliders
    QVector<QSlider*> margin_sliders = {ui->sldr_margin_left, ui->sldr_margin_top, ui->sldr_margin_right, ui->sldr_margin_bottom};
    std::for_each(margin_sliders.begin(), margin_sliders.end(), [&](QSlider* slider){
        disconnect(slider, SIGNAL(valueChanged(int)), this, SLOT(setActiveWidgetContentsMargin()));
    });

    QVector<QSlider*> spacking_sliders = {ui->sldr_spacing, ui->sldr_spacing_horizontal, ui->sldr_spacing_vertical};
    std::for_each(spacking_sliders.begin(), spacking_sliders.end(), [&](QSlider* slider){
        disconnect(slider, SIGNAL(valueChanged(int)), this, SLOT(setActiveWidgetContentsMargin()));
    });

    ui->grpboxMargins->setVisible(true);

    QMargins margins;
    if(app_object.widget)
    {
        margins = app_object.widget->contentsMargins();

        ui->grpboxSpacing->setVisible(false);
        ui->grpboxHVSpacing->setVisible(false);
    } else if (app_object.layout) {
        margins = app_object.layout->contentsMargins();

        if(app_object.obj_class == "QBoxLayout"){
            QBoxLayout* layout = qobject_cast<QBoxLayout*>(app_object.layout);
            ui->sldr_spacing->setValue(layout->spacing());
            ui->grpboxSpacing->setVisible(true);
            ui->grpboxHVSpacing->setVisible(false);

            // set slider connections
            connect(ui->sldr_spacing, SIGNAL(valueChanged(int)), this, SLOT(setActiveWidgetContentBoxSpacing()));
        } else if(app_object.obj_class == "QFormLayout"){
            QFormLayout* layout = qobject_cast<QFormLayout*>(app_object.layout);
            ui->sldr_spacing_horizontal->setValue(layout->horizontalSpacing());
            ui->sldr_spacing_vertical->setValue(layout->verticalSpacing());
            ui->grpboxSpacing->setVisible(false);
            ui->grpboxHVSpacing->setVisible(true);

            // set slider connections
            connect(ui->sldr_spacing_horizontal, SIGNAL(valueChanged(int)), this, SLOT(setActiveWidgetContentFormSpacing()));
            connect(ui->sldr_spacing_vertical, SIGNAL(valueChanged(int)), this, SLOT(setActiveWidgetContentFormSpacing()));
        } else if(app_object.obj_class == "QGridLayout"){
            QGridLayout* layout = qobject_cast<QGridLayout*>(app_object.layout);
            ui->sldr_spacing_horizontal->setValue(layout->horizontalSpacing());
            ui->sldr_spacing_vertical->setValue(layout->verticalSpacing());
            ui->grpboxSpacing->setVisible(false);
            ui->grpboxHVSpacing->setVisible(true);

            // set slider connections
            connect(ui->sldr_spacing_horizontal, SIGNAL(valueChanged(int)), this, SLOT(setActiveWidgetContentGridSpacing()));
            connect(ui->sldr_spacing_vertical, SIGNAL(valueChanged(int)), this, SLOT(setActiveWidgetContentGridSpacing()));
        } else if(app_object.obj_class == "QStackedLayout"){
            QStackedLayout* layout = qobject_cast<QStackedLayout*>(app_object.layout);
            ui->sldr_spacing->setValue(layout->spacing());
            ui->grpboxSpacing->setVisible(true);
            ui->grpboxHVSpacing->setVisible(false);

            // set slider connections
            connect(ui->sldr_spacing, SIGNAL(valueChanged(int)), this, SLOT(setActiveWidgetContentStackedSpacing()));
        }
    }

    ui->sldr_margin_left->setValue(margins.left());
    ui->sldr_margin_top->setValue(margins.top());
    ui->sldr_margin_right->setValue(margins.right());
    ui->sldr_margin_bottom->setValue(margins.bottom());

    // reset slider connections
    std::for_each(margin_sliders.begin(), margin_sliders.end(), [&](QSlider* slider){
        connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setActiveWidgetContentsMargin()));
    });

//    std::for_each(sliders.begin(), sliders.end(), [](QSlider* slider){ slider->blockSignals(false); });
}


QJsonObject MainWindow::uiChangesJson()
{
    QJsonObject uichanges_obj;
    foreach(QDockWidget* dw, this->m_ui_dockwidgets_map)
    {
        // parse the form to get those widgets whose styles (margins and spacing) have been changed
        QJsonArray widget_change_arr;
        this->parseWidgetJson(dw->widget(), widget_change_arr);
        uichanges_obj[dw->windowTitle()] = widget_change_arr;
    }

    return uichanges_obj;
}

void MainWindow::parseWidgetJson(QWidget* widget, QJsonArray& changes_arr)
{
    // check that the widget is not null
    if(! widget) {
        QString();
    }

    // test if the widgets margins were changed
    QJsonObject widget_obj;
    widget_obj["name"] = widget->objectName();

    QVariant prop = widget->property("_g_margin_edit");
    if(prop.isValid()){
        QMargins m = widget->contentsMargins();
        QJsonArray margins_arr = {m.left(), m.top(), m.right(), m.bottom()};

        widget_obj["margins"] = margins_arr;
    }

    // add the widget object to the changes array if there have been changes to the margins
    if(widget_obj.keys().count() > 1) {
        changes_arr.append(widget_obj);
    }

    if(widget->layout()) {
        this->parseLayoutJson(widget->layout(), changes_arr);
    }
}

void MainWindow::parseLayoutJson(QLayout* layout, QJsonArray& changes_arr)
{
    // check that the widget has a layout
    if(! layout) {
        QString();
    }

    // test if the widgets margins were changed
    QJsonObject widget_obj;
    widget_obj["name"] = layout->objectName();

    QVariant prop = layout->property("_g_margin_edit");
    if(prop.isValid()){
        QMargins m = layout->contentsMargins();
        QJsonArray margins_arr = {m.left(), m.top(), m.right(), m.bottom()};
        widget_obj["margins"] = margins_arr;
    }

    // test if the widgets margins were changed
    prop = layout->property("_g_spacing_edit");
    if(prop.isValid()){
        QJsonArray spacing_arr;
        QString layout_class = layout->metaObject()->className();
        if(layout_class == "QBoxLayout"){
            spacing_arr = {qobject_cast<QBoxLayout*>(layout)->spacing()};
        } else if(layout_class == "QFormLayout"){
            spacing_arr = {qobject_cast<QFormLayout*>(layout)->horizontalSpacing(),
                           qobject_cast<QFormLayout*>(layout)->verticalSpacing()};
        } else if(layout_class == "QGridLayout"){
            spacing_arr = {qobject_cast<QGridLayout*>(layout)->horizontalSpacing(),
                           qobject_cast<QGridLayout*>(layout)->verticalSpacing()};
        } else if(layout_class == "QStackedLayout"){
            spacing_arr = {qobject_cast<QStackedLayout*>(layout)->spacing()};
        }
        widget_obj["spacing"] = spacing_arr;
    }

    // add the widget object to the changes array if there have been changes to the margin or the spacing
    if(widget_obj.keys().count() > 1) {
        changes_arr.append(widget_obj);
    }

    // loop through the layout
    for(int i = 0; i < layout->count(); ++i)
    {
        QLayoutItem* layout_item = layout->itemAt(i);
        if(layout_item){
            if(! layout_item->isEmpty()) {
                if(layout_item->widget()){
                    this->parseWidgetJson(layout_item->widget(), changes_arr);
                }
                else if (layout_item->layout()) {
                    this->parseLayoutJson(layout_item->layout(), changes_arr);
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

