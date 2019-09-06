#include "formitemwidgets.h"
#include "ui_formitemwidgets.h"

FormItemWidgets::FormItemWidgets(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormItemWidgets)
{
    ui->setupUi(this);
}

FormItemWidgets::~FormItemWidgets()
{
    delete ui;
}
