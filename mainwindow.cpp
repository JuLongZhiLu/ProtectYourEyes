#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScreen>
#include <QGuiApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>


#ifdef QT_DEBUG
    #define DEBUG_CODE(code) code // Debug模式下有代码
#else
    #define DEBUG_CODE(code)   // Release模式下替换为空
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createTrayIcon();
    createTimers();

    // 使得程序运行时图标不在任务栏中显示
    this->setWindowFlags(Qt::Tool);

    // 初始化负责黑屏的窗口
    blackScreen = new QWidget(this);
    blackScreen->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    blackScreen->setStyleSheet("background-color: black;");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createTrayIcon()
{
    trayMenu = new QMenu(this);
    QAction *restoreAction = trayMenu->addAction("显示窗口");
    QAction *settingsAction = trayMenu->addAction("设置");
    QAction *quitAction = trayMenu->addAction("退出");

    connect(restoreAction, &QAction::triggered, this, &MainWindow::onRestoreAction);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsAction);
    connect(quitAction, &QAction::triggered, this, &MainWindow::onQuitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/res/eyes.png")); // 需要准备一个图标
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}

void MainWindow::createTimers()
{
    workTimer = new QTimer(this);
    blackTimer = new QTimer(this);
    
    connect(workTimer, &QTimer::timeout, this, &MainWindow::onTimerTimeout);
    connect(blackTimer, &QTimer::timeout, this, &MainWindow::hideBlackScreen);
    
    DEBUG_CODE(qDebug()<<"休息开始倒计时");
    workTimer->start(workInterval);
}

void MainWindow::onTimerTimeout()
{
    DEBUG_CODE(qDebug()<<"开始黑屏");
    showBlackScreen();
    DEBUG_CODE(qDebug()<<"休息停止倒计时");
    workTimer->stop();
    DEBUG_CODE(qDebug()<<"黑屏开始倒计时");
    blackTimer->start(blackDuration);
}

void MainWindow::showBlackScreen()
{
    blackScreen->showFullScreen();
}

void MainWindow::hideBlackScreen()
{
    DEBUG_CODE(qDebug()<<"黑屏结束");
    blackScreen->hide();
    DEBUG_CODE(qDebug()<<"休息开始倒计时");
    workTimer->start(workInterval);
    DEBUG_CODE(qDebug()<<"黑屏停止倒计时");
    blackTimer->stop();
}

void MainWindow::onRestoreAction()
{
    this->show();
}

void MainWindow::onQuitAction()
{
    QApplication::quit();
}

void MainWindow::onSettingsAction()
{
    QDialog *settingsDialog = new QDialog(nullptr);
    settingsDialog->setWindowTitle("设置");
    settingsDialog->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动释放内存
    
    QSpinBox *intervalSpinBox = new QSpinBox(settingsDialog);
    intervalSpinBox->setRange(1, 120);
    intervalSpinBox->setValue(workInterval / 60000);
    
    QSpinBox *durationSpinBox = new QSpinBox(settingsDialog);
    durationSpinBox->setRange(1, 10);
    durationSpinBox->setValue(blackDuration / 60000);
    
    QPushButton *saveButton = new QPushButton("保存", settingsDialog);
    
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("工作间隔（分钟）：", intervalSpinBox);
    formLayout->addRow("黑屏时长（分钟）：", durationSpinBox);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(settingsDialog);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(saveButton);
    
    connect(saveButton, &QPushButton::clicked, [=]() {
        workInterval = intervalSpinBox->value() * 60000;
        blackDuration = durationSpinBox->value() * 60000;
        updateTimers();
        settingsDialog->accept();
    });
    
    settingsDialog->open();
}

void MainWindow::updateTimers()
{
    workTimer->stop();
    blackTimer->stop();
    workTimer->start(workInterval);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}
