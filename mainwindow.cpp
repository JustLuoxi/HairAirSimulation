#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  oglManager = new OGLManager(this);
  oglManager->window = this;

  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &MainWindow::updateOGL);
  timer->start(10);

  /******** bg ***********/
  QPalette pal(this->palette());

  pal.setColor(QPalette::Background, QColor(99, 103, 106));
  this->setAutoFillBackground(true);
  this->setPalette(pal);

  // find componnets
  this->ui_linedit = findChild<QLineEdit*>("lineEdit");
  this->ui_slider = findChild<QSlider*>("verticalSlider");
  this->ui_addkey_button = findChild<QPushButton *>("AddKey_Button");
  this->ui_run_button = findChild<QPushButton *>("Run_Button");
  this->ui_double_splineBox = findChild<QDoubleSpinBox *>("doubleSpinBox");

//  qDebug()<<ui_linedit->text();
//  qDebug()<<ui_slider->value();

  connect(ui_slider,SIGNAL(valueChanged(int)),this,SLOT(showframe()));
}

void MainWindow::setSliderValue(int value)
{
    this->ui_slider->setValue(value);
}

void MainWindow::setdoubleSpinBoxValue(GLfloat value)
{
    ui_double_splineBox->setValue(value);
}

MainWindow::~MainWindow()
{
  delete ui;
}
void MainWindow::keyPressEvent(QKeyEvent *event){
  oglManager->handleKeyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event){
  oglManager->handleKeyReleaseEvent(event);
}

void MainWindow::on_checkBox_2_clicked(bool checked){
  if(checked)
    oglManager->isOpenLighting = GL_TRUE;
  else
    oglManager->isOpenLighting = GL_FALSE;
}

void MainWindow::on_checkBox_clicked(bool checked){
  if(checked)
    oglManager->isLineMode = GL_TRUE;
  else
    oglManager->isLineMode = GL_FALSE;

}

void MainWindow::on_pushButton_3_clicked(){
  QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"), ".");
  qDebug() << fileName;
  oglManager->changeObjModel(fileName);
}

void MainWindow::on_doubleSpinBox_valueChanged(double arg1){
  oglManager->modelScaling = arg1;
  oglManager->setModelScale();
  oglManager->setFocus();
}

void MainWindow::showframe()
{
    this->ui_linedit->setText( QString::number( this->ui_slider->value()));
//    qDebug()<<ui_linedit->text();
//    qDebug()<<ui_slider->value();
}


void MainWindow::on_AddKey_Button_clicked()
{
    oglManager->AddKeyFrame(ui_slider->value());
}

void MainWindow::on_Run_Button_clicked()
{
    oglManager->RuntheAnimation();
}

void MainWindow::on_verticalSlider_valueChanged(int value)
{
    oglManager->SetFrame(value);
}
