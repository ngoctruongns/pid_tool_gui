#include "serialworker.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>

SerialWorker::SerialWorker(QObject *parent) : QObject(parent) {
    m_serial = new QSerialPort(this);
    connect(m_serial, &QSerialPort::readyRead, this, &SerialWorker::handleReadyRead);
}

SerialWorker::~SerialWorker() {
    if (m_serial->isOpen()) m_serial->close();
}

void SerialWorker::listPorts() {
    QStringList names;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        names << info.portName();
    }
    emit portsFound(names);
}

void SerialWorker::openPort(const QString &name, qint32 baud) {
    if (m_serial->isOpen()) m_serial->close();
    m_serial->setPortName(name);
    m_serial->setBaudRate(baud);
    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit portOpened(false, name);
        emit errorOccurred(m_serial->errorString());
    } else {
        emit portOpened(true, name);
    }
}

void SerialWorker::closePort() {
    if (m_serial->isOpen()) m_serial->close();
    emit portOpened(false, QString());
}

void SerialWorker::sendData(const QByteArray &data) {
    if (!m_serial->isOpen()) {
        emit errorOccurred("Serial port not open");
        return;
    }
    m_serial->write(data);
}

void SerialWorker::handleReadyRead() {
    QByteArray b = m_serial->readAll();
    emit dataReceived(b);
}
