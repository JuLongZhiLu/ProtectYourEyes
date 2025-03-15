#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QCloseEvent>

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
    QTimer *workTimer;
    QTimer *blackTimer;
    QWidget *blackScreen;
    int workInterval = 30 * 60 * 1000; // 30分钟
    int blackDuration = 3 * 60 * 1000; // 3分钟
    void createTrayIcon();
    void createTimers();
    void showBlackScreen();
    void hideBlackScreen();
};
#endif // MAINWINDOW_H
