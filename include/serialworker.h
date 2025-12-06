#pragma once

#include <QObject>
#include <QByteArray>
#include <QStringList>

class QSerialPort;

class SerialWorker : public QObject {
    Q_OBJECT
public:
    explicit SerialWorker(QObject *parent = nullptr);
    ~SerialWorker();

public slots:
    void listPorts();
    void openPort(const QString &name, qint32 baud);
    void closePort();
    void sendData(const QByteArray &data);

signals:
    void portsFound(const QStringList &ports);
    void portOpened(bool ok, const QString &portName);
    void errorOccurred(const QString &err);
    void dataReceived(const QByteArray &data);

private slots:
    void handleReadyRead();

private:
    QSerialPort *m_serial = nullptr;
};
