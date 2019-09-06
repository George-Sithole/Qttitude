#include "forminputwidgets.h"
#include "ui_forminputwidgets.h"

FormInputWidgets::FormInputWidgets(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormInputWidgets)
{
    ui->setupUi(this);
}

FormInputWidgets::~FormInputWidgets()
{
    delete ui;
}
