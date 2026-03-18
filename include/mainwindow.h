#pragma once

#include <QMainWindow>
#include <QByteArray>

class SerialWorker;
class PlotViewer;
class QLabel;
class QGroupBox;

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
    void onClearPlot();
    void onHexModeToggled(int state);
    void onSendVelCommand();
    void onSendPIDConfig();

private:
    SerialWorker *m_serial;
    PlotViewer   *m_plot;
    QWidget      *m_centerWidget;
    QString       m_incompleteLine;

    // STM32 frame parsing state
    QByteArray    m_rxFrameBuffer;
    bool          m_inFrame = false;

    // STM32 panel widget references
    QGroupBox *m_encGroup      = nullptr;
    QGroupBox *m_velGroup      = nullptr;
    QLabel    *m_leftEncLabel  = nullptr;
    QLabel    *m_rightEncLabel = nullptr;

    // Helpers
    QByteArray buildPacket(const uint8_t *data, int len);
    void       processFrame(const QByteArray &rawBetweenSTXETX);
    void       appendLog(const QString &text);
};
