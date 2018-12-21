#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QDialog>
class QLineEdit;
class QSlider;
class MyDialog : public QDialog
{
  Q_OBJECT
public:
  explicit MyDialog(QWidget *parent = 0);
signals:
public slots:
  void setLineEditValue(int value);
private:
  QLineEdit *lineEdit;
  QSlider *slider;
};
#endif // MYDIALOG_H
