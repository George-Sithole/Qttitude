#ifndef DIALOGPAGECREATOR_H
#define DIALOGPAGECREATOR_H

#include <QDialog>
#include <QStandardItemModel>


namespace Ui {
class DialogPageCreator;
}


class QStandardItemModel;


class DialogPageCreator : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPageCreator(QWidget *parent = nullptr);
    ~DialogPageCreator();

    QStringList pageNames();

private slots:
    void on_chkSelect_stateChanged(int state);

private:
    Ui::DialogPageCreator *ui;

    QStandardItemModel m_model;
};

#endif // DIALOGPAGECREATOR_H
