#include "mainwindow.h"
#include "serialworker.h"
#include "plotviewer.h"

#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>
#include <QShortcut>
#include <QApplication>
#include <QTextCursor>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_serial = new SerialWorker(this);
    m_plot = new PlotViewer(this);

    // Top row: serial controls
    auto *portCombo = new QComboBox();
    auto *findBtn = new QPushButton("Find Port");
    auto *baudCombo = new QComboBox();
    baudCombo->addItems({"9600","19200","38400","57600","115200","230400", "460800", "921600"});
    baudCombo->setCurrentText("115200");
    auto *hexLogChk = new QCheckBox("HEX");
    auto *openBtn = new QPushButton("Open");
    auto *closeBtn = new QPushButton("Close");
    closeBtn->setEnabled(false);

    connect(findBtn, &QPushButton::clicked, this, &MainWindow::onFindPorts);
    connect(openBtn, &QPushButton::clicked, this, &MainWindow::onOpenClosePort);
    connect(closeBtn, &QPushButton::clicked, this, &MainWindow::onOpenClosePort);

    // Second row: send command (three preset command rows)
    auto *cmdLine1 = new QLineEdit();
    cmdLine1->setPlaceholderText("Command input here...");
    auto *sendHexChk1 = new QCheckBox("HEX");
    auto *eolCombo1 = new QComboBox();
    eolCombo1->addItems({"None","LF (\\n)","CRLF (\\r\\n)"});
    eolCombo1->setCurrentIndex(1); // default LF
    auto *sendBtn1 = new QPushButton("Send");
    connect(sendBtn1, &QPushButton::clicked, this, &MainWindow::onSendCommand);

    auto *cmdLine2 = new QLineEdit();
    cmdLine2->setPlaceholderText("Command input here...");
    auto *sendHexChk2 = new QCheckBox("HEX");
    auto *eolCombo2 = new QComboBox();
    eolCombo2->addItems({"None","LF (\\n)","CRLF (\\r\\n)"});
    eolCombo2->setCurrentIndex(1); // default LF
    auto *sendBtn2 = new QPushButton("Send");
    connect(sendBtn2, &QPushButton::clicked, this, &MainWindow::onSendCommand);

    auto *cmdLine3 = new QLineEdit();
    cmdLine3->setPlaceholderText("Command input here...");
    auto *sendHexChk3 = new QCheckBox("HEX");
    auto *eolCombo3 = new QComboBox();
    eolCombo3->addItems({"None","LF (\\n)","CRLF (\\r\\n)"});
    eolCombo3->setCurrentIndex(1); // default LF
    auto *sendBtn3 = new QPushButton("Send");
    connect(sendBtn3, &QPushButton::clicked, this, &MainWindow::onSendCommand);

    // Third row: log view and PID params
    auto *logView = new QTextEdit();
    logView->setReadOnly(true);
    auto *autoScrollChk = new QCheckBox("Auto Scroll");
    autoScrollChk->setChecked(true);
    auto *clearLogsBtn = new QPushButton("Clear Logs");

    auto *pidGroup = new QGroupBox();
    auto *form = new QFormLayout();
    auto *kpEdit = new QLineEdit();
    auto *kiEdit = new QLineEdit();
    auto *kdEdit = new QLineEdit();
    auto *updateBtn = new QPushButton("Update");
    form->addRow(new QLabel("Kp"), kpEdit);
    form->addRow(new QLabel("Ki"), kiEdit);
    form->addRow(new QLabel("Kd"), kdEdit);
    form->addRow(updateBtn);
    pidGroup->setLayout(form);
    connect(updateBtn, &QPushButton::clicked, this, &MainWindow::onUpdatePID);

    // Layout assembly
    auto *topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel("Port:"));
    topRow->addWidget(portCombo);
    topRow->addWidget(findBtn);
    topRow->addWidget(new QLabel("Baud:"));
    topRow->addWidget(baudCombo);
    topRow->addWidget(hexLogChk);
    topRow->addWidget(openBtn);
    topRow->addWidget(closeBtn);

    auto *secondRow = new QHBoxLayout();
    secondRow->addWidget(cmdLine1);
    secondRow->addWidget(sendHexChk1);
    secondRow->addWidget(eolCombo1);
    secondRow->addWidget(sendBtn1);

    auto *secondRow2 = new QHBoxLayout();
    secondRow2->addWidget(cmdLine2);
    secondRow2->addWidget(sendHexChk2);
    secondRow2->addWidget(eolCombo2);
    secondRow2->addWidget(sendBtn2);

    auto *secondRow3 = new QHBoxLayout();
    secondRow3->addWidget(cmdLine3);
    secondRow3->addWidget(sendHexChk3);
    secondRow3->addWidget(eolCombo3);
    secondRow3->addWidget(sendBtn3);

    // left column: log view + controls
    auto *leftCol = new QVBoxLayout();
    leftCol->addWidget(logView, 1);
    auto *logBtns = new QHBoxLayout();
    logBtns->addWidget(autoScrollChk);
    logBtns->addStretch();
    logBtns->addWidget(clearLogsBtn);
    auto *clearPlotBtn = new QPushButton("Clear Plot");
    logBtns->addWidget(clearPlotBtn);
    leftCol->addLayout(logBtns);

    auto *thirdRow = new QHBoxLayout();
    thirdRow->addLayout(leftCol, 3);
    thirdRow->addWidget(pidGroup, 1);

    auto *mainLay = new QVBoxLayout();
    mainLay->addLayout(topRow);
    mainLay->addLayout(secondRow);
    mainLay->addLayout(secondRow2);
    mainLay->addLayout(secondRow3);
    mainLay->addLayout(thirdRow);
    mainLay->addWidget(m_plot, 1);

    m_centerWidget = new QWidget();
    m_centerWidget->setLayout(mainLay);
    setCentralWidget(m_centerWidget);

    // hook up serial signals
    connect(m_serial, &SerialWorker::portsFound, this, &MainWindow::onPortListUpdated);
    connect(m_serial, &SerialWorker::portOpened, this, &MainWindow::onPortOpened);
    connect(m_serial, &SerialWorker::dataReceived, this, &MainWindow::onDataReceived);

    // initial ports
    QTimer::singleShot(100, this, SLOT(onFindPorts()));

    // Save pointers in widget properties for later retrieval in slots
    portCombo->setObjectName("portCombo");
    baudCombo->setObjectName("baudCombo");
    hexLogChk->setObjectName("hexLogChk");
    cmdLine1->setObjectName("cmdLine1");
    sendHexChk1->setObjectName("sendHexChk1");
    eolCombo1->setObjectName("eolCombo1");
    cmdLine2->setObjectName("cmdLine2");
    sendHexChk2->setObjectName("sendHexChk2");
    eolCombo2->setObjectName("eolCombo2");
    cmdLine3->setObjectName("cmdLine3");
    sendHexChk3->setObjectName("sendHexChk3");
    eolCombo3->setObjectName("eolCombo3");
    logView->setObjectName("logView");
    autoScrollChk->setObjectName("autoScrollChk");
    clearLogsBtn->setObjectName("clearLogsBtn");
    kpEdit->setObjectName("kpEdit");
    kiEdit->setObjectName("kiEdit");
    kdEdit->setObjectName("kdEdit");
    openBtn->setObjectName("openBtn");
    closeBtn->setObjectName("closeBtn");
    sendBtn1->setObjectName("sendBtn1");
    sendBtn2->setObjectName("sendBtn2");
    sendBtn3->setObjectName("sendBtn3");
    clearPlotBtn->setObjectName("clearPlotBtn");

    connect(clearLogsBtn, &QPushButton::clicked, this, &MainWindow::onClearLogs);
    connect(clearPlotBtn, &QPushButton::clicked, this, &MainWindow::onClearPlot);

    setWindowTitle("PID Tuning Tool");
    resize(900, 700);

    // Add Ctrl+Q shortcut to quit application
    new QShortcut(Qt::CTRL + Qt::Key_Q, this, SLOT(close()));
}

MainWindow::~MainWindow() {}

void MainWindow::onFindPorts() {
    m_serial->listPorts();
}

void MainWindow::onOpenClosePort() {
    // determine sender
    QObject *s = sender();
    auto *portCombo = m_centerWidget->findChild<QComboBox*>("portCombo");
    auto *baudCombo = m_centerWidget->findChild<QComboBox*>("baudCombo");
    auto *openBtn = m_centerWidget->findChild<QPushButton*>("openBtn");
    auto *closeBtn = m_centerWidget->findChild<QPushButton*>("closeBtn");

    if (!portCombo) return;

    if (s && s->objectName() == "openBtn") {
        QString port = portCombo->currentText();
        qint32 baud = baudCombo ? baudCombo->currentText().toInt() : 115200;
        m_serial->openPort(port, baud);
    } else {
        m_serial->closePort();
    }
}

void MainWindow::onPortListUpdated(const QStringList &ports) {
    auto *portCombo = m_centerWidget->findChild<QComboBox*>("portCombo");
    if (!portCombo) return;
    portCombo->clear();
    portCombo->addItems(ports);
}

void MainWindow::onPortOpened(bool ok, const QString &name) {
    auto *openBtn = m_centerWidget->findChild<QPushButton*>("openBtn");
    auto *closeBtn = m_centerWidget->findChild<QPushButton*>("closeBtn");
    auto *logView = m_centerWidget->findChild<QTextEdit*>("logView");
    if (ok) {
        if (openBtn) openBtn->setEnabled(false);
        if (closeBtn) closeBtn->setEnabled(true);
        if (logView) logView->append(QString("Opened port %1").arg(name));
    } else {
        if (openBtn) openBtn->setEnabled(true);
        if (closeBtn) closeBtn->setEnabled(false);
        if (logView) logView->append("Closed port");
    }
}

void MainWindow::onDataReceived(const QByteArray &data) {
    auto *hexLogChk = m_centerWidget->findChild<QCheckBox*>("hexLogChk");
    auto *logView = m_centerWidget->findChild<QTextEdit*>("logView");
    if (!logView) return;

    if (hexLogChk && hexLogChk->isChecked()) {
        logView->append(QString(data.toHex(' ').toUpper()));
        auto *autoScrollChk = m_centerWidget->findChild<QCheckBox*>("autoScrollChk");
        if (autoScrollChk && autoScrollChk->isChecked()) {
            logView->moveCursor(QTextCursor::End);
            logView->ensureCursorVisible();
        }
        return;
    }

    QString s = QString::fromUtf8(data);

    // Ghép với phần dòng chưa hoàn chỉnh từ lần trước
    QString fullPayload = m_incompleteLine + s;
    QStringList lines = fullPayload.split('\n');
    m_incompleteLine.clear();

    bool endsWithLinebreak = fullPayload.endsWith('\n');
    if (!endsWithLinebreak) {
        m_incompleteLine = lines.takeLast();
    }

    for (const QString &rawLine : lines) {
        QString line = rawLine;

        // Loại bỏ CR nếu có (nguồn serial từ Windows CRLF)
        line.remove('\r');
        if (line.trimmed().isEmpty()) continue;

        // Thêm vào logView chỉ khi có line hoàn chỉnh
        logView->append(line);
        auto *autoScrollChk = m_centerWidget->findChild<QCheckBox*>("autoScrollChk");
        if (autoScrollChk && autoScrollChk->isChecked()) {
            logView->moveCursor(QTextCursor::End);
            logView->ensureCursorVisible();
        }

        // Parse Arduino Serial Plotter format: "Variable_1:value1,Variable_2:value2"
        QStringList pairs = line.split(',', Qt::SkipEmptyParts);

        bool hasValidData = false;
        for (const QString &pair : pairs) {
            QString pairTrimmed = pair.trimmed();
            int colonIdx = pairTrimmed.indexOf(':');
            if (colonIdx > 0 && colonIdx < pairTrimmed.length() - 1) {
                QString varName = pairTrimmed.left(colonIdx).trimmed();
                QString valueStr = pairTrimmed.mid(colonIdx + 1).trimmed();

                bool ok = false;
                double value = valueStr.toDouble(&ok);
                if (ok) {
                    m_plot->appendSeriesPointByName(varName, value);
                    hasValidData = true;
                }
            }
        }

        // Increment counter after all values for this line are added
        if (hasValidData) {
            m_plot->incrementSampleCounter();
        }
    }
}

void MainWindow::onSendCommand() {
    // Determine which Send button invoked this slot and pick matching widgets
    QObject *s = sender();
    QString idx = "1"; // default to first row
    if (s) {
        QString sname = s->objectName();
        if (sname.startsWith("sendBtn")) {
            idx = sname.mid(QString("sendBtn").length());
            if (idx.isEmpty()) idx = "1";
        }
    }

    auto *cmdLine = m_centerWidget->findChild<QLineEdit*>(QString("cmdLine%1").arg(idx));
    auto *sendHexChk = m_centerWidget->findChild<QCheckBox*>(QString("sendHexChk%1").arg(idx));
    auto *logView = m_centerWidget->findChild<QTextEdit*>("logView");
    if (!cmdLine) return;
    QString txt = cmdLine->text();
    if (txt.isEmpty()) return;
    QByteArray out;
    if (sendHexChk && sendHexChk->isChecked()) {
        // remove spaces then convert hex
        QString h = txt;
        h.remove(' ');
        out = QByteArray::fromHex(h.toUtf8());
    } else {
        out = txt.toUtf8();
    }
    // Append configured End-Of-Line bytes for this command row (None / LF / CRLF)
    auto *eolCombo = m_centerWidget->findChild<QComboBox*>(QString("eolCombo%1").arg(idx));
    if (eolCombo) {
        int ei = eolCombo->currentIndex();
        if (ei == 1) { // LF
            out.append('\n');
        } else if (ei == 2) { // CRLF
            out.append('\r');
            out.append('\n');
        }
    }
    m_serial->sendData(out);
    if (logView) logView->append(QString("TX: %1").arg(QString::fromUtf8(out)));
    if (logView) {
        auto *autoScrollChk = m_centerWidget->findChild<QCheckBox*>("autoScrollChk");
        if (autoScrollChk && autoScrollChk->isChecked()) {
            logView->moveCursor(QTextCursor::End);
            logView->ensureCursorVisible();
        }
    }
}

void MainWindow::onUpdatePID() {
    auto *kpEdit = m_centerWidget->findChild<QLineEdit*>("kpEdit");
    auto *kiEdit = m_centerWidget->findChild<QLineEdit*>("kiEdit");
    auto *kdEdit = m_centerWidget->findChild<QLineEdit*>("kdEdit");
    if (!kpEdit || !kiEdit || !kdEdit) return;
    QString kp = kpEdit->text();
    QString ki = kiEdit->text();
    QString kd = kdEdit->text();

    // simple protocol: send `PID Kp Ki Kd\n`
    QString cmd = QString("PID %1 %2 %3\n").arg(kp, ki, kd);
    m_serial->sendData(cmd.toUtf8());
    auto *logView = m_centerWidget->findChild<QTextEdit*>("logView");
    if (logView) logView->append(QString("Sent PID: %1").arg(cmd.trimmed()));
}

void MainWindow::onClearLogs() {
    auto *logView = m_centerWidget->findChild<QTextEdit*>("logView");
    if (logView) logView->clear();
}

void MainWindow::onClearPlot() {
    if (m_plot) {
        m_plot->clear();
    }
}
