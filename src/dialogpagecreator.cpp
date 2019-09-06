// Qt Libraries
#include <QFile>
#include <QBrush>

// Local Libraries
#include "dialogpagecreator.h"
#include "ui_dialogpagecreator.h"


DialogPageCreator::DialogPageCreator(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPageCreator)
{
    ui->setupUi(this);

    m_model.setColumnCount(1);

    // read files
    QStringList sl;
    QFile file(":/stylesheeteditor/pages.txt");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        sl = QString(file.readAll()).split('\n', QString::SkipEmptyParts);
    }

    for(QString s: sl){
        QStringList sp = s.split(',', QString::SkipEmptyParts);
        QStandardItem* item = new QStandardItem(sp[0].trimmed());
        QColor color(sp[1]);
        QBrush brush(color);
        item->setBackground(brush);
        item->setEditable(false);
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        m_model.appendRow(item);
    }

    ui->listView->setModel(&m_model);
}

QStringList DialogPageCreator::pageNames()
{
    QStringList page_names;
    for(int i = 0; i < m_model.rowCount(); ++i)
    {
        QStandardItem* item = m_model.item(i);
        if(item->checkState() == Qt::Checked) {
            page_names << item->text();
        }
    }
    return page_names;
}

DialogPageCreator::~DialogPageCreator()
{
    delete ui;
}

void DialogPageCreator::on_chkSelect_stateChanged(int state)
{
    Qt::CheckState cstate = state == Qt::Checked ? Qt::Checked : Qt::Unchecked;
    for(int i = 0; i < m_model.rowCount(); ++i){
        m_model.item(i)->setCheckState(cstate);
    }
}
