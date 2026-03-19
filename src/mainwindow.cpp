#include "mainwindow.h"
#include "serialworker.h"
#include "plotviewer.h"
#include "process_data_packet.h"
#include "velocity_control.h"

#include <cstring>
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
#include <QMenuBar>
#include <QAction>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_serial = new SerialWorker(this);
    m_plot = new PlotViewer(this);

    // Top row: serial controls
    auto *portCombo = new QComboBox();
    portCombo->setMaximumWidth(160);
    auto *findBtn = new QPushButton("Find Port");
    findBtn->setMaximumWidth(120);
    auto *baudCombo = new QComboBox();
    baudCombo->addItems({"9600","19200","38400","57600","115200","230400", "460800", "921600"});
    baudCombo->setCurrentText("115200");
    baudCombo->setMaximumWidth(140);
    auto *hexLogChk = new QCheckBox("HEX");
    auto *openBtn = new QPushButton("Open");
    openBtn->setMaximumWidth(110);
    auto *closeBtn = new QPushButton("Close");
    closeBtn->setMaximumWidth(110);
    auto *toggleLedBuzzerBtn = new QPushButton("PID/LED/Buzzer");
    toggleLedBuzzerBtn->setCheckable(true);
    toggleLedBuzzerBtn->setMaximumWidth(150);
    closeBtn->setEnabled(false);

    connect(findBtn, &QPushButton::clicked, this, &MainWindow::onFindPorts);
    connect(openBtn, &QPushButton::clicked, this, &MainWindow::onOpenClosePort);
    connect(closeBtn, &QPushButton::clicked, this, &MainWindow::onOpenClosePort);
    connect(hexLogChk, &QCheckBox::stateChanged, this, &MainWindow::onHexModeToggled);

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
    auto *logFilterCombo = new QComboBox();
    logFilterCombo->addItems({"Show All", "Only RX", "Only TX", "Hide All"});
    logFilterCombo->setCurrentIndex(0);
    auto *clearLogsBtn = new QPushButton("Clear Logs");

    // Encoder data section (enabled in hex mode only)
    m_encGroup = new QGroupBox("Encoder Data");
    m_encGroup->setEnabled(false);
    auto *encForm = new QFormLayout();
    m_leftEncLabel  = new QLabel("--");
    m_rightEncLabel = new QLabel("--");
    m_leftEncLabel->setMinimumWidth(80);
    m_rightEncLabel->setMinimumWidth(80);
    encForm->addRow(new QLabel("Left:"),  m_leftEncLabel);
    encForm->addRow(new QLabel("Right:"), m_rightEncLabel);
    m_encGroup->setLayout(encForm);

    // Motor RPM feedback section (enabled in hex mode only)
    m_rpmGroup = new QGroupBox("Motor RPM Feedback");
    m_rpmGroup->setEnabled(false);
    auto *rpmForm = new QFormLayout();
    m_leftRpmLabel = new QLabel("--");
    m_rightRpmLabel = new QLabel("--");
    m_leftRpmLabel->setMinimumWidth(80);
    m_rightRpmLabel->setMinimumWidth(80);
    rpmForm->addRow(new QLabel("Left RPM:"), m_leftRpmLabel);
    rpmForm->addRow(new QLabel("Right RPM:"), m_rightRpmLabel);
    m_rpmGroup->setLayout(rpmForm);

    // Velocity command section (enabled in hex mode only)
    m_velGroup = new QGroupBox("Velocity Command");
    m_velGroup->setEnabled(false);
    auto *velForm = new QFormLayout();
    auto *leftRpmEdit  = new QLineEdit();
    auto *rightRpmEdit = new QLineEdit();
    leftRpmEdit->setPlaceholderText("0");
    rightRpmEdit->setPlaceholderText("0");
    auto *velContinuousChk = new QCheckBox("Continuous");
    auto *velIntervalEdit = new QLineEdit();
    velIntervalEdit->setText("100");
    velIntervalEdit->setMaximumWidth(80);
    auto *sendVelBtn = new QPushButton("Send Velocity");
    auto *velPeriodicRow = new QWidget();
    auto *velPeriodicLay = new QHBoxLayout(velPeriodicRow);
    velPeriodicLay->setContentsMargins(0, 0, 0, 0);
    velPeriodicLay->addWidget(velContinuousChk);
    velPeriodicLay->addWidget(new QLabel("Interval(ms):"));
    velPeriodicLay->addWidget(velIntervalEdit);
    velPeriodicLay->addStretch();
    velForm->addRow(new QLabel("Left RPM:"),  leftRpmEdit);
    velForm->addRow(new QLabel("Right RPM:"), rightRpmEdit);
    velForm->addRow(velPeriodicRow);
    velForm->addRow(sendVelBtn);
    m_velGroup->setLayout(velForm);
    connect(sendVelBtn, &QPushButton::clicked, this, &MainWindow::onSendVelCommand);

    m_velSendTimer = new QTimer(this);
    m_velSendTimer->setSingleShot(false);
    connect(m_velSendTimer, &QTimer::timeout, this, &MainWindow::onSendVelCommand);
    connect(velContinuousChk, &QCheckBox::stateChanged, this, [this, velIntervalEdit](int state) {
        bool enableContinuous = (state == Qt::Checked);
        if (!enableContinuous) {
            if (m_velSendTimer) m_velSendTimer->stop();
            appendLog("Velocity continuous send stopped");
            return;
        }

        bool ok = false;
        int intervalMs = velIntervalEdit->text().toInt(&ok);
        if (!ok || intervalMs <= 0) {
            appendLog("[ERR] Invalid velocity interval (ms)");
            auto *chk = m_centerWidget ? m_centerWidget->findChild<QCheckBox*>("velContinuousChk") : nullptr;
            if (chk) chk->setChecked(false);
            return;
        }

        if (m_velSendTimer) {
            m_velSendTimer->start(intervalMs);
        }
        appendLog(QString("Velocity continuous send started (%1 ms)").arg(intervalMs));
    });

    connect(velIntervalEdit, &QLineEdit::editingFinished, this, [this, velIntervalEdit]() {
        if (!m_velSendTimer || !m_velSendTimer->isActive()) return;
        bool ok = false;
        int intervalMs = velIntervalEdit->text().toInt(&ok);
        if (!ok || intervalMs <= 0) {
            appendLog("[ERR] Invalid velocity interval (ms)");
            return;
        }
        m_velSendTimer->setInterval(intervalMs);
        appendLog(QString("Velocity interval updated: %1 ms").arg(intervalMs));
    });

    // PID config section
    auto *pidGroup = new QGroupBox("PID Config");
    auto *form = new QFormLayout();
    auto *kpEdit = new QLineEdit();
    auto *kiEdit = new QLineEdit();
    auto *kdEdit = new QLineEdit();
    auto *updateBtn = new QPushButton("Send PID");
    form->addRow(new QLabel("Kp"), kpEdit);
    form->addRow(new QLabel("Ki"), kiEdit);
    form->addRow(new QLabel("Kd"), kdEdit);
    form->addRow(updateBtn);
    pidGroup->setLayout(form);
    connect(updateBtn, &QPushButton::clicked, this, &MainWindow::onSendPIDConfig);

    // Communication control section (enabled in hex mode only)
    m_commGroup = new QGroupBox("Feedback Control");
    m_commGroup->setEnabled(false);
    auto *commForm = new QFormLayout();
    auto *feedbackEncChk = new QCheckBox("Encoder");
    auto *feedbackRpmChk = new QCheckBox("Motor RPM");
    feedbackEncChk->setChecked(true);
    auto *sendCommBtn = new QPushButton("Apply Feedback");
    commForm->addRow(feedbackEncChk);
    commForm->addRow(feedbackRpmChk);
    commForm->addRow(sendCommBtn);
    m_commGroup->setLayout(commForm);
    connect(sendCommBtn, &QPushButton::clicked, this, &MainWindow::onSendCommControl);

    // LED control section (enabled in hex mode only)
    m_ledGroup = new QGroupBox("LED Control");
    m_ledGroup->setEnabled(false);
    auto *ledForm = new QFormLayout();
    auto *ledTypeEdit = new QLineEdit();
    auto *ledREdit = new QLineEdit();
    auto *ledGEdit = new QLineEdit();
    auto *ledBEdit = new QLineEdit();
    auto *ledP1Edit = new QLineEdit();
    auto *ledP2Edit = new QLineEdit();
    ledTypeEdit->setPlaceholderText("type");
    ledREdit->setPlaceholderText("0..255");
    ledGEdit->setPlaceholderText("0..255");
    ledBEdit->setPlaceholderText("0..255");
    ledP1Edit->setPlaceholderText("param1");
    ledP2Edit->setPlaceholderText("param2");
    auto *sendLedBtn = new QPushButton("Send LED");
    ledForm->addRow(new QLabel("Type:"), ledTypeEdit);
    ledForm->addRow(new QLabel("R:"), ledREdit);
    ledForm->addRow(new QLabel("G:"), ledGEdit);
    ledForm->addRow(new QLabel("B:"), ledBEdit);
    ledForm->addRow(new QLabel("Param1:"), ledP1Edit);
    ledForm->addRow(new QLabel("Param2:"), ledP2Edit);
    ledForm->addRow(sendLedBtn);
    m_ledGroup->setLayout(ledForm);
    connect(sendLedBtn, &QPushButton::clicked, this, &MainWindow::onSendLEDControl);

    // Buzzer control section (enabled in hex mode only)
    m_buzzerGroup = new QGroupBox("Buzzer Control");
    m_buzzerGroup->setEnabled(false);
    auto *buzzerForm = new QFormLayout();
    auto *buzzerTypeEdit = new QLineEdit();
    auto *buzzerP1Edit = new QLineEdit();
    auto *buzzerP2Edit = new QLineEdit();
    buzzerTypeEdit->setPlaceholderText("type");
    buzzerP1Edit->setPlaceholderText("param1");
    buzzerP2Edit->setPlaceholderText("param2");
    auto *sendBuzzerBtn = new QPushButton("Send Buzzer");
    buzzerForm->addRow(new QLabel("Type:"), buzzerTypeEdit);
    buzzerForm->addRow(new QLabel("Param1:"), buzzerP1Edit);
    buzzerForm->addRow(new QLabel("Param2:"), buzzerP2Edit);
    buzzerForm->addRow(sendBuzzerBtn);
    m_buzzerGroup->setLayout(buzzerForm);
    connect(sendBuzzerBtn, &QPushButton::clicked, this, &MainWindow::onSendBuzzerControl);

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
    topRow->addWidget(toggleLedBuzzerBtn);

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

    // left column: top controls + command rows + log view + controls
    auto *leftCol = new QVBoxLayout();
    leftCol->addLayout(topRow);
    leftCol->addLayout(secondRow);
    leftCol->addLayout(secondRow2);
    leftCol->addLayout(secondRow3);
    leftCol->addWidget(logView, 1);
    auto *spaceLineBtn = new QPushButton("Space Line");
    auto *logBtns = new QHBoxLayout();
    logBtns->addWidget(autoScrollChk);
    logBtns->addWidget(logFilterCombo);
    logBtns->addWidget(spaceLineBtn);
    logBtns->addStretch();
    logBtns->addWidget(clearLogsBtn);
    auto *clearPlotBtn = new QPushButton("Clear Plot");
    logBtns->addWidget(clearPlotBtn);
    leftCol->addLayout(logBtns);
    connect(spaceLineBtn, &QPushButton::clicked, this, [this]() {
        auto *logView = m_centerWidget->findChild<QTextEdit*>("logView");
        if (!logView) return;
        logView->append("");
        logView->append("-----------------------------");
        logView->append("");
    });

    auto *rightColLeft = new QVBoxLayout();
    rightColLeft->addWidget(m_encGroup);
    rightColLeft->addWidget(m_rpmGroup);
    rightColLeft->addWidget(m_velGroup);
    rightColLeft->addWidget(m_commGroup);
    rightColLeft->addStretch();

    auto *rightColLedBuzzer = new QVBoxLayout();
    rightColLedBuzzer->addWidget(pidGroup);
    rightColLedBuzzer->addWidget(m_ledGroup);
    rightColLedBuzzer->addWidget(m_buzzerGroup);
    rightColLedBuzzer->addStretch();

    auto *ledBuzzerWidget = new QWidget();
    ledBuzzerWidget->setLayout(rightColLedBuzzer);
    ledBuzzerWidget->setVisible(false);

    auto *rightCols = new QHBoxLayout();
    rightCols->addLayout(rightColLeft, 1);
    rightCols->addWidget(ledBuzzerWidget, 1);

    auto *rightWidget = new QWidget();
    rightWidget->setLayout(rightCols);

    auto *thirdRow = new QHBoxLayout();
    thirdRow->addLayout(leftCol, 3);
    thirdRow->addWidget(rightWidget, 1);
    thirdRow->setAlignment(rightWidget, Qt::AlignTop);

    auto *mainLay = new QVBoxLayout();
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
    logFilterCombo->setObjectName("logFilterCombo");
    clearLogsBtn->setObjectName("clearLogsBtn");
    kpEdit->setObjectName("kpEdit");
    kiEdit->setObjectName("kiEdit");
    kdEdit->setObjectName("kdEdit");
    leftRpmEdit->setObjectName("leftRpmEdit");
    rightRpmEdit->setObjectName("rightRpmEdit");
    velContinuousChk->setObjectName("velContinuousChk");
    velIntervalEdit->setObjectName("velIntervalEdit");
    feedbackEncChk->setObjectName("feedbackEncChk");
    feedbackRpmChk->setObjectName("feedbackRpmChk");
    ledTypeEdit->setObjectName("ledTypeEdit");
    ledREdit->setObjectName("ledREdit");
    ledGEdit->setObjectName("ledGEdit");
    ledBEdit->setObjectName("ledBEdit");
    ledP1Edit->setObjectName("ledP1Edit");
    ledP2Edit->setObjectName("ledP2Edit");
    buzzerTypeEdit->setObjectName("buzzerTypeEdit");
    buzzerP1Edit->setObjectName("buzzerP1Edit");
    buzzerP2Edit->setObjectName("buzzerP2Edit");
    openBtn->setObjectName("openBtn");
    closeBtn->setObjectName("closeBtn");
    sendBtn1->setObjectName("sendBtn1");
    sendBtn2->setObjectName("sendBtn2");
    sendBtn3->setObjectName("sendBtn3");
    sendVelBtn->setObjectName("sendVelBtn");
    sendCommBtn->setObjectName("sendCommBtn");
    sendLedBtn->setObjectName("sendLedBtn");
    sendBuzzerBtn->setObjectName("sendBuzzerBtn");
    toggleLedBuzzerBtn->setObjectName("toggleLedBuzzerBtn");
    clearPlotBtn->setObjectName("clearPlotBtn");

    connect(clearLogsBtn, &QPushButton::clicked, this, &MainWindow::onClearLogs);
    connect(clearPlotBtn, &QPushButton::clicked, this, &MainWindow::onClearPlot);

    auto *viewMenu = menuBar()->addMenu("View");
    auto *showLedBuzzerAct = new QAction("Show PID/LED/Buzzer Panel", this);
    showLedBuzzerAct->setCheckable(true);
    showLedBuzzerAct->setChecked(false);
    viewMenu->addAction(showLedBuzzerAct);

    connect(toggleLedBuzzerBtn, &QPushButton::toggled, this,
            [ledBuzzerWidget, showLedBuzzerAct](bool checked) {
                ledBuzzerWidget->setVisible(checked);
                showLedBuzzerAct->setChecked(checked);
            });

    connect(showLedBuzzerAct, &QAction::toggled, this,
            [ledBuzzerWidget, toggleLedBuzzerBtn](bool checked) {
                ledBuzzerWidget->setVisible(checked);
                toggleLedBuzzerBtn->setChecked(checked);
            });

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
    if (ok) {
        if (openBtn) openBtn->setEnabled(false);
        if (closeBtn) closeBtn->setEnabled(true);
        appendLog(QString("Opened port %1").arg(name));
    } else {
        if (openBtn) openBtn->setEnabled(true);
        if (closeBtn) closeBtn->setEnabled(false);
        appendLog("Closed port");
    }
}

void MainWindow::onDataReceived(const QByteArray &data) {
    auto *hexLogChk = m_centerWidget->findChild<QCheckBox*>("hexLogChk");
    bool hexMode = hexLogChk && hexLogChk->isChecked();

    if (hexMode) {
        // Frame-based parsing: STX(0xAA) ... ETX(0xDD)
        for (int i = 0; i < data.size(); ++i) {
            uint8_t byte = static_cast<uint8_t>(data[i]);
            if (byte == STX) {
                m_rxFrameBuffer.clear();
                m_inFrame = true;
            } else if (byte == ETX && m_inFrame) {
                m_inFrame = false;
                // Display full frame as hex
                QByteArray fullFrame;
                fullFrame.append(static_cast<char>(STX));
                fullFrame.append(m_rxFrameBuffer);
                fullFrame.append(static_cast<char>(ETX));
                appendLog(QString("RX: ") + QString(fullFrame.toHex(' ').toUpper()), false, true);
                // Decode and dispatch
                processFrame(m_rxFrameBuffer);
                m_rxFrameBuffer.clear();
            } else if (m_inFrame) {
                m_rxFrameBuffer.append(static_cast<char>(byte));
            }
        }
        return;
    }

    // Text mode: line-based parsing (existing behaviour)
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
        appendLog(line, false, true);

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
    appendLog(QString("TX: %1").arg(QString::fromUtf8(out)), true, false);
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

// ---------- Helper: log text + optional auto-scroll ----------

void MainWindow::appendLog(const QString &text, bool isTx, bool isRx) {
    auto *logView = m_centerWidget->findChild<QTextEdit*>("logView");
    if (!logView) return;
    auto *logFilterCombo = m_centerWidget->findChild<QComboBox*>("logFilterCombo");
    int filterIdx = logFilterCombo ? logFilterCombo->currentIndex() : 0;

    bool shouldShow = true;
    switch (filterIdx) {
        case 0: // Show All
            shouldShow = true;
            break;
        case 1: // Only RX
            shouldShow = isRx;
            break;
        case 2: // Only TX
            shouldShow = isTx;
            break;
        case 3: // Hide All
            shouldShow = false;
            break;
        default:
            shouldShow = true;
            break;
    }

    if (!shouldShow) return;

    logView->append(text);
    auto *autoScrollChk = m_centerWidget->findChild<QCheckBox*>("autoScrollChk");
    if (autoScrollChk && autoScrollChk->isChecked()) {
        logView->moveCursor(QTextCursor::End);
        logView->ensureCursorVisible();
    }
}

// ---------- Helper: build framed packet (STX | escaped data+CRC | ETX) ----------

QByteArray MainWindow::buildPacket(const uint8_t *data, int len) {
    uint8_t buf[512];
    uint8_t pktLen = encoderAllPackage(data, static_cast<uint8_t>(len), buf);
    return QByteArray(reinterpret_cast<const char*>(buf), pktLen);
}

// ---------- Helper: decode and dispatch a complete received frame ----------

void MainWindow::processFrame(const QByteArray &rawBetweenSTXETX) {
    // rawBetweenSTXETX contains escaped payload + escaped CRC (no STX/ETX)
    uint8_t decoded[256];
    uint8_t len = decoderAllPackage(
        reinterpret_cast<const uint8_t*>(rawBetweenSTXETX.constData()),
        static_cast<uint8_t>(rawBetweenSTXETX.size()),
        decoded);

    if (len == 0) {
        appendLog("[ERR] Frame discarded: CRC mismatch or invalid", false, true);
        return;
    }
    if (len < 1) return;

    uint8_t type = decoded[0];
    switch (type) {
        case DEBUG_STRING: {
            QString dbgStr = QString::fromUtf8(
                reinterpret_cast<const char*>(decoded + 1), len - 1);
            appendLog(QString("DBG: ") + dbgStr, false, true);
            break;
        }
        case WHEEL_ENC_COMMAND: {
            if (len < static_cast<uint8_t>(sizeof(WheelEncType))) {
                appendLog("[ERR] WHEEL_ENC frame too short", false, true);
                break;
            }
            WheelEncType enc;
            std::memcpy(&enc, decoded, sizeof(WheelEncType));
            if (m_leftEncLabel)
                m_leftEncLabel->setText(QString::number(enc.left_enc));
            if (m_rightEncLabel)
                m_rightEncLabel->setText(QString::number(enc.right_enc));
            break;
        }
        case MOTOR_RPM_COMMAND:
        case CMD_VEL_COMMAND: {
            if (len < static_cast<uint8_t>(sizeof(CmdVelType))) {
                appendLog("[ERR] RPM frame too short", false, true);
                break;
            }
            CmdVelType rpm;
            std::memcpy(&rpm, decoded, sizeof(CmdVelType));
            if (m_leftRpmLabel) {
                m_leftRpmLabel->setText(QString::number(rpm.left_rpm));
            }
            if (m_rightRpmLabel) {
                m_rightRpmLabel->setText(QString::number(rpm.right_rpm));
            }
            if (m_hasSetRpm) {
                m_plot->appendSeriesPointByName("Set Left RPM", static_cast<qreal>(m_lastSetLeftRpm));
                m_plot->appendSeriesPointByName("Set Right RPM", static_cast<qreal>(m_lastSetRightRpm));
            }
            m_plot->appendSeriesPointByName("Left RPM", static_cast<qreal>(rpm.left_rpm));
            m_plot->appendSeriesPointByName("Right RPM", static_cast<qreal>(rpm.right_rpm));
            m_plot->incrementSampleCounter();
            break;
        }
        case PID_CONFIG_COMMAND:
        case COMM_CTRL_COMMAND:
        case LED_CONTROL_COMMAND:
        case BUZZER_CONTROL_COMMAND:
            appendLog(QString("[WARN] Received outgoing command type: %1").arg(type), false, true);
            break;
        default:
            appendLog(QString("[WARN] Unknown frame type: %1").arg(type), false, true);
            break;
    }
}

// ---------- Slot: HEX mode toggled ----------

void MainWindow::onHexModeToggled(int state) {
    bool hexMode = (state == Qt::Checked);
    if (m_encGroup) m_encGroup->setEnabled(hexMode);
    if (m_rpmGroup) m_rpmGroup->setEnabled(hexMode);
    if (m_velGroup) m_velGroup->setEnabled(hexMode);
    if (m_commGroup) m_commGroup->setEnabled(hexMode);
    if (m_ledGroup) m_ledGroup->setEnabled(hexMode);
    if (m_buzzerGroup) m_buzzerGroup->setEnabled(hexMode);
    // Reset parsing state when switching modes
    m_inFrame = false;
    m_rxFrameBuffer.clear();
    m_incompleteLine.clear();

    if (!hexMode && m_velSendTimer && m_velSendTimer->isActive()) {
        m_velSendTimer->stop();
        auto *velContinuousChk = m_centerWidget->findChild<QCheckBox*>("velContinuousChk");
        if (velContinuousChk) velContinuousChk->setChecked(false);
    }
}

// ---------- Slot: Send velocity command (framed) ----------

void MainWindow::onSendVelCommand() {
    int16_t leftRpm = 0;
    int16_t rightRpm = 0;

    if (sender() == m_velSendTimer) {
        if (!m_hasSetRpm) {
            return;
        }
        leftRpm = m_lastSetLeftRpm;
        rightRpm = m_lastSetRightRpm;
    } else {
        auto *leftRpmEdit  = m_centerWidget->findChild<QLineEdit*>("leftRpmEdit");
        auto *rightRpmEdit = m_centerWidget->findChild<QLineEdit*>("rightRpmEdit");
        if (!leftRpmEdit || !rightRpmEdit) return;

        bool ok1, ok2;
        leftRpm  = static_cast<int16_t>(leftRpmEdit->text().toInt(&ok1));
        rightRpm = static_cast<int16_t>(rightRpmEdit->text().toInt(&ok2));
        if (!ok1 || !ok2) {
            appendLog("[ERR] Invalid RPM values");
            return;
        }

        m_lastSetLeftRpm = leftRpm;
        m_lastSetRightRpm = rightRpm;
        m_hasSetRpm = true;
    }

    CmdVelType cmd;
    cmd.type      = CMD_VEL_COMMAND;
    cmd.left_rpm  = leftRpm;
    cmd.right_rpm = rightRpm;

    QByteArray pkt = buildPacket(reinterpret_cast<const uint8_t*>(&cmd), sizeof(cmd));
    m_serial->sendData(pkt);
    appendLog(QString("TX Vel: L=%1 R=%2 | %3")
        .arg(leftRpm).arg(rightRpm)
        .arg(QString(pkt.toHex(' ').toUpper())), true, false);
}

// ---------- Slot: Send PID config (framed in hex mode, text otherwise) ----------

void MainWindow::onSendPIDConfig() {
    auto *kpEdit    = m_centerWidget->findChild<QLineEdit*>("kpEdit");
    auto *kiEdit    = m_centerWidget->findChild<QLineEdit*>("kiEdit");
    auto *kdEdit    = m_centerWidget->findChild<QLineEdit*>("kdEdit");
    auto *hexLogChk = m_centerWidget->findChild<QCheckBox*>("hexLogChk");
    if (!kpEdit || !kiEdit || !kdEdit) return;

    bool hexMode = hexLogChk && hexLogChk->isChecked();
    if (hexMode) {
        bool ok1, ok2, ok3;
        float kp = kpEdit->text().toFloat(&ok1);
        float ki = kiEdit->text().toFloat(&ok2);
        float kd = kdEdit->text().toFloat(&ok3);
        if (!ok1 || !ok2 || !ok3) {
            appendLog("[ERR] Invalid PID values");
            return;
        }

        PIDConfigType cfg;
        cfg.type = PID_CONFIG_COMMAND;
        cfg.Kp = kp;
        cfg.Ki = ki;
        cfg.Kd = kd;

        QByteArray pkt = buildPacket(reinterpret_cast<const uint8_t*>(&cfg), sizeof(cfg));
        m_serial->sendData(pkt);
        appendLog(QString("TX PID: Kp=%1 Ki=%2 Kd=%3 | %4")
            .arg(kp, 0, 'f', 4).arg(ki, 0, 'f', 4).arg(kd, 0, 'f', 4)
            .arg(QString(pkt.toHex(' ').toUpper())), true, false);
    } else {
        // Text mode: "PID kp ki kd\n"
        QString cmd = QString("PID %1 %2 %3\n")
            .arg(kpEdit->text(), kiEdit->text(), kdEdit->text());
        m_serial->sendData(cmd.toUtf8());
        appendLog(QString("TX PID: %1").arg(cmd.trimmed()), true, false);
    }
}

void MainWindow::onSendCommControl() {
    auto *feedbackEncChk = m_centerWidget->findChild<QCheckBox*>("feedbackEncChk");
    auto *feedbackRpmChk = m_centerWidget->findChild<QCheckBox*>("feedbackRpmChk");
    if (!feedbackEncChk || !feedbackRpmChk) return;

    uint8_t feedback = FEEDBACK_DEFAULT;
    if (feedbackEncChk->isChecked()) {
        feedback |= FEEDBACK_ENCODER;
    }
    if (feedbackRpmChk->isChecked()) {
        feedback |= FEEDBACK_MOTOR_RPM;
    }

    CommCtrlType comm;
    comm.feedback = feedback;

    uint8_t payload[1 + sizeof(CommCtrlType)] = {0};
    payload[0] = COMM_CTRL_COMMAND;
    std::memcpy(payload + 1, &comm, sizeof(comm));

    QByteArray pkt = buildPacket(payload, sizeof(payload));
    m_serial->sendData(pkt);
    appendLog(QString("TX COMM_CTRL: feedback=0x%1 | %2")
        .arg(QString::number(feedback, 16).toUpper())
        .arg(QString(pkt.toHex(' ').toUpper())), true, false);
}

void MainWindow::onSendLEDControl() {
    auto *ledTypeEdit = m_centerWidget->findChild<QLineEdit*>("ledTypeEdit");
    auto *ledREdit = m_centerWidget->findChild<QLineEdit*>("ledREdit");
    auto *ledGEdit = m_centerWidget->findChild<QLineEdit*>("ledGEdit");
    auto *ledBEdit = m_centerWidget->findChild<QLineEdit*>("ledBEdit");
    auto *ledP1Edit = m_centerWidget->findChild<QLineEdit*>("ledP1Edit");
    auto *ledP2Edit = m_centerWidget->findChild<QLineEdit*>("ledP2Edit");
    if (!ledTypeEdit || !ledREdit || !ledGEdit || !ledBEdit || !ledP1Edit || !ledP2Edit) return;

    bool okType, okR, okG, okB, okP1, okP2;
    int type = ledTypeEdit->text().toInt(&okType);
    int r = ledREdit->text().toInt(&okR);
    int g = ledGEdit->text().toInt(&okG);
    int b = ledBEdit->text().toInt(&okB);
    int p1 = ledP1Edit->text().toInt(&okP1);
    int p2 = ledP2Edit->text().toInt(&okP2);
    if (!okType || !okR || !okG || !okB || !okP1 || !okP2) {
        appendLog("[ERR] Invalid LED values");
        return;
    }

    auto clampU8 = [](int v) -> uint8_t {
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        return static_cast<uint8_t>(v);
    };
    auto clampU16 = [](int v) -> uint16_t {
        if (v < 0) v = 0;
        if (v > 65535) v = 65535;
        return static_cast<uint16_t>(v);
    };

    LEDControlType led;
    led.type = clampU8(type);
    led.r = clampU8(r);
    led.g = clampU8(g);
    led.b = clampU8(b);
    led.param1 = clampU16(p1);
    led.param2 = clampU16(p2);

    uint8_t payload[1 + sizeof(LEDControlType)] = {0};
    payload[0] = LED_CONTROL_COMMAND;
    std::memcpy(payload + 1, &led, sizeof(led));

    QByteArray pkt = buildPacket(payload, sizeof(payload));
    m_serial->sendData(pkt);
    appendLog(QString("TX LED: type=%1 rgb=(%2,%3,%4) p1=%5 p2=%6 | %7")
        .arg(led.type)
        .arg(led.r)
        .arg(led.g)
        .arg(led.b)
        .arg(led.param1)
        .arg(led.param2)
        .arg(QString(pkt.toHex(' ').toUpper())), true, false);
}

void MainWindow::onSendBuzzerControl() {
    auto *buzzerTypeEdit = m_centerWidget->findChild<QLineEdit*>("buzzerTypeEdit");
    auto *buzzerP1Edit = m_centerWidget->findChild<QLineEdit*>("buzzerP1Edit");
    auto *buzzerP2Edit = m_centerWidget->findChild<QLineEdit*>("buzzerP2Edit");
    if (!buzzerTypeEdit || !buzzerP1Edit || !buzzerP2Edit) return;

    bool okType, okP1, okP2;
    int type = buzzerTypeEdit->text().toInt(&okType);
    int p1 = buzzerP1Edit->text().toInt(&okP1);
    int p2 = buzzerP2Edit->text().toInt(&okP2);
    if (!okType || !okP1 || !okP2) {
        appendLog("[ERR] Invalid buzzer values");
        return;
    }

    auto clampU8 = [](int v) -> uint8_t {
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        return static_cast<uint8_t>(v);
    };
    auto clampU16 = [](int v) -> uint16_t {
        if (v < 0) v = 0;
        if (v > 65535) v = 65535;
        return static_cast<uint16_t>(v);
    };

    BuzzerControlType buzzer;
    buzzer.type = clampU8(type);
    buzzer.param1 = clampU16(p1);
    buzzer.param2 = clampU16(p2);

    uint8_t payload[1 + sizeof(BuzzerControlType)] = {0};
    payload[0] = BUZZER_CONTROL_COMMAND;
    std::memcpy(payload + 1, &buzzer, sizeof(buzzer));

    QByteArray pkt = buildPacket(payload, sizeof(payload));
    m_serial->sendData(pkt);
    appendLog(QString("TX BUZZER: type=%1 p1=%2 p2=%3 | %4")
        .arg(buzzer.type)
        .arg(buzzer.param1)
        .arg(buzzer.param2)
        .arg(QString(pkt.toHex(' ').toUpper())), true, false);
}