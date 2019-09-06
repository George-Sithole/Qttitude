#include <QVBoxLayout>
#include <QMessageBox>
#include <QFile>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "formbuttons.h"
#include "formitemviews.h"
#include "formitemwidgets.h"
#include "formcontainers.h"
#include "forminputwidgets.h"
#include "formdisplaywidgets.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    FormButtons* form_buttons = new FormButtons(this);
    ui->dockWidgetButtons->setWidget(form_buttons);

    FormItemViews* form_item_views = new FormItemViews(this);
    ui->dockWidgetItemViews->setWidget(form_item_views);

    FormItemWidgets* form_item_widgets = new FormItemWidgets(this);
    ui->dockWidgetItemWidgets->setWidget(form_item_widgets);

    FormContainers* form_containers = new FormContainers(this);
    ui->dockWidgetContainers->setWidget(form_containers);

    FormInputWidgets* form_input_widgets = new FormInputWidgets(this);
    ui->dockWidgetInputWidgets->setWidget(form_input_widgets);

    FormDisplayWidgets* form_display_widgets = new FormDisplayWidgets(this);
    ui->dockWidgetDisplayWidgets->setWidget(form_display_widgets);

    this->tabifyDockWidget(ui->dockWidgetButtons, ui->dockWidgetItemViews);
    this->tabifyDockWidget(ui->dockWidgetItemViews, ui->dockWidgetItemWidgets);
    this->tabifyDockWidget(ui->dockWidgetItemWidgets, ui->dockWidgetContainers);
    this->tabifyDockWidget(ui->dockWidgetContainers, ui->dockWidgetInputWidgets);
    this->tabifyDockWidget(ui->dockWidgetInputWidgets, ui->dockWidgetDisplayWidgets);

    ui->dockWidgetButtons->raise();

    QFile file("stylesheet.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        this->setStyleSheet(file.readAll());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::about()
{
    QString message = "<html>"
                      "<head/>"
                      "<body>"
                      "<p>Copyright @ 2019 George Sithole</p>"
                      "<p>This is free software distributed under the terms of the GNU Lesser General Public License, LGPL v3.</p>"
                      "<p>For more information about this application visit <a href=www.geovariant.com>www.geovariant.com</a></P>"
                      "<p>Want to help? <a href=www.geovariant.com>Make a donation</a></p>"
                      "<p>The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</p>"
                      "</body>"
                      "</html>";

    QMessageBox msg_box;
    msg_box.setWindowTitle("About");
    msg_box.setText("<b>Version 0.1.0</b>");
    msg_box.setTextFormat(Qt::RichText);
    msg_box.setInformativeText(message);
    msg_box.exec();
}

void MainWindow::on_actionAbout_triggered()
{
    this->about();
}
