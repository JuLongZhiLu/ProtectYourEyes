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
#include <QCheckBox>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QTranslator>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("ProtectYourEyes"));

    QString exeDir = QCoreApplication::applicationDirPath();
    QString iniPath = QDir::toNativeSeparators(exeDir+"/settings.ini");
    QSettings settings(iniPath, QSettings::IniFormat);

    // Read settings with fallback to default values
    screenBlackInterval = settings.value("Settings/screenBlackInterval", 30 * 60 * 1000).toInt();
    screenBlackDuration = settings.value("Settings/screenBlackDuration", 3 * 60 * 1000).toInt();
    bool autoStart = settings.value("Settings/autoStart", false).toBool();
    setAutoStart(autoStart);
    forcedModeOption = settings.value("Settings/forcedModeOption", "Don't use forced mode")
                           .toString();

    // Write default values if file doesn't exist
    if (!QFile::exists(iniPath)) {
        settings.setValue("Settings/screenBlackInterval", screenBlackInterval);
        settings.setValue("Settings/screenBlackDuration", screenBlackDuration);
        settings.setValue("Settings/autoStart", autoStart);
        settings.setValue("Settings/forcedModeOption", "Don't use forced mode");
        settings.sync(); // Write to file immediately
    }

    createTrayIcon();
    createTimers();

    // 使得程序运行时图标不在任务栏中显示
    this->setWindowFlags(Qt::Tool);

    settingsWidget.hide();
}

void MainWindow::updateCountdown()
{
    int remaining = screenBlackDurationTimer->remainingTime() / 1000;
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    foreach (QLabel* label, countdownLabels) {
        label->setText(QString("%1:%2")
                       .arg(minutes, 2, 10, QLatin1Char('0'))
                     .arg(seconds, 2, 10, QLatin1Char('0')));
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createTrayIcon()
{
    trayMenu = new QMenu(this);
    QAction *restoreAction = trayMenu->addAction(tr("About"));
    QAction *settingsAction = trayMenu->addAction(tr("Settings"));

//    QMenu *languageMenu = trayMenu->addMenu("Language");
//    QAction *chineseAction = languageMenu->addAction("中文");
//    QAction *englishAction = languageMenu->addAction("English");

    QAction *quitAction = trayMenu->addAction(tr("Exit"));

    connect(restoreAction, &QAction::triggered, this, &MainWindow::onRestoreAction);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsAction);
    connect(quitAction, &QAction::triggered, this, &MainWindow::onQuitAction);
//    connect(chineseAction, &QAction::triggered, [this](){ changeLanguage("zh_CN"); });
//    connect(englishAction, &QAction::triggered, [this](){ changeLanguage("en_US"); });


    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/res/eyes.png"));
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    // Startup reminder
    trayIcon->showMessage(tr("Info"),tr("running in the system tray.\nPress Esc or Ctrl+Q to exit black.\nScreen Black Interval：%1mins\nScreen Black Duration：%2mins")
                         .arg(screenBlackInterval / 60000)
                         .arg(screenBlackDuration / 60000),
                         QSystemTrayIcon::Information,
                         3000);
}

void MainWindow::createTimers()
{
    screenBlackIntervalTimer = new QTimer(this);
    screenBlackDurationTimer = new QTimer(this);
    blackClockTimer = new QTimer(this);
    
    connect(screenBlackIntervalTimer, &QTimer::timeout, this, &MainWindow::onTimerTimeout);
    connect(screenBlackDurationTimer, &QTimer::timeout, this, &MainWindow::hideBlackScreen);
    connect(blackClockTimer,&QTimer::timeout,this,&MainWindow::updateCountdown);
    
    screenBlackIntervalTimer->start(screenBlackInterval);
}

void MainWindow::onTimerTimeout()
{
    showBlackScreen();
    screenBlackIntervalTimer->stop();
    screenBlackDurationTimer->start(screenBlackDuration);

}

void MainWindow::showBlackScreen()
{
    escPressCount = 0;
    // Get all screens
    QList<QScreen*> screens = QGuiApplication::screens();
    foreach (QScreen* screen, screens) {
        QWidget* screenBlack = new QWidget();
        screenBlack->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        screenBlack->setStyleSheet("background-color: black;");
        screenBlack->setGeometry(screen->geometry());
        screenBlack->showFullScreen();
        blackScreens.append(screenBlack);

        qDebug() << forcedModeOption;
        if (forcedModeOption == "Don't use forced mode") {
            // Add shortcut support
            qDebug() << "forcedModeOption == Don't use forced mode";
            QShortcut* quitBlackScreenShortcut1 = new QShortcut(QKeySequence(Qt::Key_Escape),
                                                                screenBlack);
            connect(quitBlackScreenShortcut1,
                    &QShortcut::activated,
                    this,
                    &MainWindow::hideBlackScreen);
            QShortcut* quitBlackScreenShortcut2 = new QShortcut(QKeySequence("Ctrl+Q"), screenBlack);
            connect(quitBlackScreenShortcut2,
                    &QShortcut::activated,
                    this,
                    &MainWindow::hideBlackScreen);
        } else if (forcedModeOption == "Can't exit early") {
            qDebug() << "forcedModeOption == Can't exit early";
        } else if (forcedModeOption == "Must press ESC 50 times") {
            qDebug() << "forcedModeOption == Must press ESC 50 times";
            QShortcut* escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), screenBlack);
            connect(escShortcut, &QShortcut::activated, this, [this]() {
                escPressCount++;
                if (escPressCount >= 50) {
                    hideBlackScreen(); // exit black screen upon 50 ESC key presses.
                    escPressCount = 0; // reset escPressCount
                } else {
                    // Optional: Display remaining count (updated via QLabel).
                    // updateCountdownLabel(50 - escPressCount);
                }
            });
        }

        // Create countdown labels for each screen
        QLabel* label = new QLabel(screenBlack);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("font-size: 100px; color: white;");
        label->setGeometry(0, 0, screen->geometry().width(), screen->geometry().height());
        label->show();
        countdownLabels.append(label);
    }
    blackClockTimer->start(1000);
}

void MainWindow::hideBlackScreen()
{
    // delete all black screen
    foreach (QWidget* screen, blackScreens) {
        screen->hide();
        screen->deleteLater();
    }
    blackScreens.clear();
    // delete all countdown labels
    foreach (QLabel* label, countdownLabels) {
        label->hide();
        label->deleteLater();
    }
    countdownLabels.clear();
    screenBlackIntervalTimer->start(screenBlackInterval);
    screenBlackDurationTimer->stop();
    blackClockTimer->stop();
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
    settingsWidget.show();
}

void MainWindow::updateTimers()
{
    screenBlackIntervalTimer->stop();
    screenBlackDurationTimer->stop();
    screenBlackIntervalTimer->start(screenBlackInterval);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}

bool MainWindow::isAutoStartEnabled()
{
#ifdef Q_OS_WIN
    QSettings bootUpSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    return bootUpSettings.contains("ProtectYourEyes");
#endif
    return false;
}

void MainWindow::setAutoStart(bool enabled)
{
#ifdef Q_OS_WIN
    QSettings bootUpSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (enabled) {
        bootUpSettings.setValue("ProtectYourEyes", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    } else {
        bootUpSettings.remove("ProtectYourEyes");
    }
#endif

#ifdef Q_OS_LINUX
    QString autostartDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart";
    QDir().mkpath(autostartDir);
    QString desktopFile = autostartDir + "/ProtectYourEyes.desktop";

    if (enabled) {
        QFile file(desktopFile);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream << "[Desktop Entry]\n"
                   << "Type=Application\n"
                   << "Name=ProtectYourEyes\n"
                   << "Exec=" << QCoreApplication::applicationFilePath() << "\n"
                   << "Hidden=false\n"
                   << "X-GNOME-Autostart-enabled=true\n";
            file.close();
        }
    } else {
        QFile::remove(desktopFile);
    }
#endif
}


//TODO BUG:如果是中文切中文，或者英文切英文，会导致程序崩溃
void MainWindow::changeLanguage(const QString& language)
{
    // 保存语言设置
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QSettings settings(configPath + "/settings.ini", QSettings::IniFormat);
    settings.setValue("Settings/language", language);

    //加载对应的翻译文件
    QTranslator translator;
    if(translator.load(":/res/language/" + language + ".qm")) {
        qApp->installTranslator(&translator);
    }

//    delete trayMenu;
//    delete trayIcon;
//    trayMenu = nullptr;
//    trayIcon = nullptr;
//    createTrayIcon();
    // restart the application
    QProcess::startDetached(QApplication::applicationFilePath());
    QApplication::quit();


}

void MainWindow::loadLanguage()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QSettings settings(configPath + "/settings.ini", QSettings::IniFormat);
    QString language = settings.value("Settings/language", "zh_CN").toString();
    changeLanguage(language);
}
