#ifndef FORMCONTAINERS_H
#define FORMCONTAINERS_H

#include <QWidget>

namespace Ui {
class FormContainers;
}

class FormContainers : public QWidget
{
    Q_OBJECT

public:
    explicit FormContainers(QWidget *parent = nullptr);
    ~FormContainers();

private:
    Ui::FormContainers *ui;
};

#endif // FORMCONTAINERS_H
