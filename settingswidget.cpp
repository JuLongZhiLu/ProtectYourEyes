#include <QSettings>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QCloseEvent>
#include <QShowEvent>

#include "settingswidget.h"
#include "ui_settingswidget.h"


SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);
    setWindowTitle(tr("Settings"));
    setWindowIcon(QIcon(":/res/eyes.png"));
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::closeEvent(QCloseEvent *event)
{
    this->hide();
    event->ignore();
}

void SettingsWidget::showEvent(QShowEvent *event)
{
    QString exeDir = QCoreApplication::applicationDirPath();
    QString iniPath = QDir::toNativeSeparators(exeDir+"/settings.ini");
    QSettings settings(iniPath, QSettings::IniFormat);

    // Read settings
    int screenBlackInterval = settings.value("Settings/screenBlackInterval").toInt();
    int screenBlackDuration = settings.value("Settings/screenBlackDuration").toInt();
    bool autoStart = settings.value("Settings/autoStart").toBool();

    ui->screen_black_interval_spinBox->setValue(screenBlackInterval/(60 * 1000));
    ui->screen_black_duration_spinBox->setValue(screenBlackDuration/(60 * 1000));
    ui->auto_start_checkBox->setChecked(autoStart);

    QWidget::showEvent(event);
}

void SettingsWidget::on_save_settings_pushButton_clicked()
{
    QString exeDir = QCoreApplication::applicationDirPath();
    QString iniPath = QDir::toNativeSeparators(exeDir+"/settings.ini");
    QSettings settings(iniPath, QSettings::IniFormat);

    settings.setValue("Settings/screenBlackInterval", ui->screen_black_interval_spinBox->value()*60*1000);
    settings.setValue("Settings/screenBlackDuration", ui->screen_black_duration_spinBox->value()*60*1000);
    settings.setValue("Settings/autoStart", ui->auto_start_checkBox->isChecked());
    settings.sync(); // Write to file immediately

    QMessageBox::information(
                nullptr,
                tr("Info"),
                tr("Save Successfully")
            );

    if(ui->auto_start_checkBox->isChecked()==true){
        int result = QMessageBox::question(
            nullptr,
            tr("Restart Required"),
            tr("'Auto Start' option will take effect only after restart.\nDo you want to restart now?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
        );

        // restart the application
        if (result == QMessageBox::Yes) {
            QProcess::startDetached(QApplication::applicationFilePath());
            QApplication::quit();
        }
    }

}
