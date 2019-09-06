#include "formcontainers.h"
#include "ui_formcontainers.h"

FormContainers::FormContainers(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormContainers)
{
    ui->setupUi(this);
}

FormContainers::~FormContainers()
{
    delete ui;
}
