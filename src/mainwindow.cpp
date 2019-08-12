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

// Qt Libraries
#include <QGridLayout>
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

// Local Libraries
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stylesheeteditorwidget.h"
#include "dragitemmodel.h"
#include "workspace.h"
#include "project.h"


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
    this->setupStyleSheetServer();

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
        file.open(QFile::ReadOnly);
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
    QStringList fnames = filenames.count() > 0 ? filenames : QFileDialog::getOpenFileNames(this, tr("Open User Interface"), this->m_project->workingDir(), tr("User Interface Files (*.ui)"));

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
    this->m_widgets_model->clear();
    this->m_widgets_model->setColumnCount(2);
    this->m_widgets_model->setHeaderData(0, Qt::Horizontal, "Object");
    this->m_widgets_model->setHeaderData(1, Qt::Horizontal, "Class");

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

QString MainWindow::getAppWidgets(QWidget* parent)
{
    if(parent != Q_NULLPTR)
    {
        QStringList widgets;
        this->getParentObjects(parent, widgets, 1);
        return widgets.join('\n');
    }
    return QString();
}

void MainWindow::getParentObjects(QObject* parent, QStringList& objects, const int& depth)
{
    // write parent
    QString object_name = parent->objectName();
    QString class_name = parent->metaObject()->className();
    objects << QString("%0,%1,%2").arg(depth).arg(object_name).arg(class_name);

    // write the children
    for(QObject* child: parent->children())
    {
        this->getParentObjects(child, objects, depth + 1);
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
//        QString key = m_files_model->index(index.row(), 1).data().toString();
        QString key = item->toolTip();
        QDockWidget* dockwidget = this->m_ui_dockwidgets_map[key];


        // test if the index has been checked
        if(item->checkState() == Qt::Checked){
            // update the tree view containing the widget's layout
            QString layout = this->getAppWidgets(dockwidget->widget());
            this->createWidgetTree(layout);
            dockwidget->show();
        }
        else {
            this->m_widgets_model->clear();
            m_widgets_model->setColumnCount(2);
            m_widgets_model->setHeaderData(0, Qt::Horizontal, "Object");
            m_widgets_model->setHeaderData(1, Qt::Horizontal, "Class");

            dockwidget->hide();
        }
    }
}


void MainWindow::on_treeViewFiles_doubleClicked(const QModelIndex &index)
{
    if(index.isValid())
    {
        QString key = m_files_model->index(index.row(), 1).data().toString();
        QDockWidget* dockwidget = this->m_ui_dockwidgets_map[key];
        dockwidget->show();

        QStandardItem* item = this->m_files_model->item(index.row(), 0);
        item->setCheckState(dockwidget->isHidden() ? Qt::Unchecked : Qt::Checked);
    }
}


// -------------------------------------
//         Widgets functions
// -------------------------------------

void MainWindow::setupStyleSheetServer() // TODO rename this function
{
    m_widgets_model = new DragItemModel(this);
    m_widgets_model->setColumnCount(2);
    m_widgets_model->setHeaderData(0, Qt::Horizontal, "Object");
    m_widgets_model->setHeaderData(1, Qt::Horizontal, "Class");

    ui->treeViewAppWidgets->setModel(m_widgets_model);
    ui->treeViewAppWidgets->setIndentation(16);
}

void MainWindow::updateWidgetsTree(const QString& text)
{
    this->createWidgetTree(text);
}

void MainWindow::createWidgetTree(QString text)
{
    QString widgets;
    widgets = "QAbstractScrollArea;QCheckBox;QColumnView;QComboBox;QDateEdit;"
              "QDateTimeEdit;QDialog;QDialogButtonBox;QDockWidget;QDoubleSpinBox;"
              "QFrame;QGroupBox;QHeaderView;QLabel;QLineEdit;QListView;"
              "QListWidget;QMainWindow;QMenu;QMenuBar;QMessageBox;QProgressBar;"
              "QPushButton;QRadioButton;QScrollBar;QSizeGrip;QSlider;"
              "QSpinBox;QSplitter;QStatusBar;QTabBar;QTabWidget;"
              "QTableView;QTableWidget;QTextEdit;QTimeEdit;QToolBar;"
              "QToolButton;QToolBox;QToolTip;QTreeView;QTreeWidget;QWidget";

    QStringList inclusion = widgets.split(';', QString::SkipEmptyParts);

    QHash<int, QStandardItem*> parent_hash;
    QTextStream ts(&text);
    QString line;
    while (ts.readLineInto(&line))
    {
        QStringList sp = line.split(',', QString::KeepEmptyParts);

        if(sp.count() == 3)
        {
            int depth = sp[0].toInt();
            QString obj_name = sp[1];
            if(obj_name.isEmpty())
                obj_name = "No Name";
            QString obj_class = sp[2];

            if(depth == 1)
            {
                QStandardItem* parent_item = new QStandardItem(obj_name);
                parent_item->setEditable(false);
                QStandardItem* value_item = new QStandardItem(obj_class);
                value_item->setEditable(false);

                this->m_widgets_model->appendRow({parent_item, value_item});
                parent_hash[depth] = parent_item;
            }
            else
            {
//                if(inclusion.contains(obj_class))
                {
                    QStandardItem* child_item = new QStandardItem(obj_name);
                    child_item->setEditable(false);
                    QStandardItem* value_item = new QStandardItem(obj_class);
                    value_item->setEditable(false);

                    QStandardItem* parent_item = parent_hash[depth - 1];
                    parent_item->appendRow({child_item, value_item});

                    parent_hash[depth] = child_item;
                }
            }
        }
    }

    // expand the tree
    ui->treeViewAppWidgets->expandAll();
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

            // find the file item
            QList<QStandardItem*> items;
            for(int row = 0; row < this->m_files_model->rowCount(); ++row){
                QStandardItem* item = this->m_files_model->item(row, 0);
                if(item->toolTip() == title)
                    items.push_back(item);
            }

            // synch the state of the dockwidget to the widgets model
            if(items.count() > 0){
                QStandardItem* item = this->m_files_model->item(items[0]->row(), 0);
                if(dockwidget->isHidden()){
                    item->setCheckState(Qt::Unchecked);

                    // clear the widgets tree view
                    this->m_widgets_model->clear();
                    m_widgets_model->setColumnCount(2);
                    m_widgets_model->setHeaderData(0, Qt::Horizontal, "Object");
                    m_widgets_model->setHeaderData(1, Qt::Horizontal, "Class");
                }
                else {
                    item->setCheckState(Qt::Checked);

                    // update the tree view containing the widget's layout
                    QString layout = this->getAppWidgets(dockwidget->widget());
                    this->createWidgetTree(layout);
                }

                processed = true;
            }
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
    this->m_se_widget->saveStyleSheet();
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
        local_filename = QFileDialog::getSaveFileName(this, tr("Project File"), this->m_project->workingDir(), tr("Project File (*.proj)"));
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

    // clear and reset the data models
    this->m_files_model->clear();
    this->m_files_model->setColumnCount(1);
    this->m_files_model->setHeaderData(0, Qt::Horizontal, "Filename");
//    this->m_files_model->setHeaderData(1, Qt::Horizontal, "Path");

    this->m_widgets_model->clear();
    this->m_widgets_model->setColumnCount(2);
    this->m_widgets_model->setHeaderData(0, Qt::Horizontal, "Object");
    this->m_widgets_model->setHeaderData(1, Qt::Horizontal, "Class");


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
        local_filename = QFileDialog::getOpenFileName(this, tr("Project File"), this->m_project->workingDir(), tr("Style File (*.proj)"));
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
    qDebug() << "Open url";
    QDesktopServices::openUrl(QUrl("https://www.geovariant.com"));
}

void MainWindow::setIcons()
{
    ui->actionNew->setIcon(QIcon(":/icons/file-medical.svg"));
    ui->actionOpen->setIcon(QIcon(":/icons/file-alt.svg"));
    ui->actionOpen->setIcon(QIcon(":/icons/file-download.svg"));
    ui->actionOpen->setIcon(QIcon(":/icons/file-download-as.svg"));
}

