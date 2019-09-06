#include "formdisplaywidgets.h"
#include "ui_formdisplaywidgets.h"

FormDisplayWidgets::FormDisplayWidgets(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormDisplayWidgets)
{
    ui->setupUi(this);
}

FormDisplayWidgets::~FormDisplayWidgets()
{
    delete ui;
}
