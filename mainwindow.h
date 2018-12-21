#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "oglmanager.h"
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QDoubleSpinBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    void setSliderValue(int value);
    void setdoubleSpinBoxValue(GLfloat value);
    ~MainWindow();
protected:
    void keyPressEvent(QKeyEvent *event);   //
    void keyReleaseEvent(QKeyEvent *event);
private:
    Ui::MainWindow *ui;
    OGLManager *oglManager;

    QLineEdit *ui_linedit;
    QSlider *ui_slider;
    QPushButton *ui_addkey_button;
    QPushButton *ui_run_button;
    QDoubleSpinBox *ui_double_splineBox;

private slots:
    void updateOGL(){
      oglManager->update();
    }
    void on_checkBox_2_clicked(bool checked);
    void on_checkBox_clicked(bool checked);
    void on_pushButton_3_clicked();
    void on_doubleSpinBox_valueChanged(double arg1);
    void showframe();
    void on_AddKey_Button_clicked();
    void on_Run_Button_clicked();
    void on_verticalSlider_valueChanged(int value);
};

#endif // MAINWINDOW_H
