#include "formitemviews.h"
#include "ui_formitemviews.h"

FormItemViews::FormItemViews(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormItemViews)
{
    ui->setupUi(this);
}

FormItemViews::~FormItemViews()
{
    delete ui;
}
