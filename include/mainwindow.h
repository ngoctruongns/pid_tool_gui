#pragma once

#include <QMainWindow>

class SerialWorker;
class PlotViewer;

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFindPorts();
    void onOpenClosePort();
    void onPortListUpdated(const QStringList &ports);
    void onPortOpened(bool ok, const QString &name);
    void onDataReceived(const QByteArray &data);
    void onSendCommand();
    void onClearLogs();
    void onUpdatePID();

private:
    SerialWorker *m_serial;
    PlotViewer *m_plot;
    QWidget *m_centerWidget;
};
