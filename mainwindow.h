#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QCloseEvent>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>

#include "settingswidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onTimerTimeout();
    void onRestoreAction();
    void onQuitAction();
    void onSettingsAction();
    void updateTimers();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QTimer *screenBlackIntervalTimer;
    QTimer *screenBlackDurationTimer;
    QTimer *blackClockTimer;
    QList<QWidget*> blackScreens;  // 存储所有屏幕的黑屏窗口
    QList<QLabel*> countdownLabels; // 存储所有屏幕的倒计时标签
    int screenBlackInterval = 30 * 60 * 1000; // 30分钟
    int screenBlackDuration = 3 * 60 * 1000; // 3分钟
    QString currentLanguage;
    SettingsWidget settingsWidget;

    void createTrayIcon();
    void createTimers();
    void showBlackScreen();
    void hideBlackScreen();
    void updateCountdown();
    bool isAutoStartEnabled();
    void setAutoStart(bool enabled);
    void changeLanguage(const QString& language);
    void loadLanguage();
};
#endif // MAINWINDOW_H
