#ifndef FORMITEMVIEWS_H
#define FORMITEMVIEWS_H

#include <QWidget>

namespace Ui {
class FormItemViews;
}

class FormItemViews : public QWidget
{
    Q_OBJECT

public:
    explicit FormItemViews(QWidget *parent = nullptr);
    ~FormItemViews();

private:
    Ui::FormItemViews *ui;
};

#endif // FORMITEMVIEWS_H
