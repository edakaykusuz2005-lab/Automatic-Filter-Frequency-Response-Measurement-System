#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QTableWidgetItem>
#include <QtGlobal>
#include <QClipboard>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_sweep(&m_serial, this),
      dyno_plot(0)
//      m_plot(0)
{
    ui->setupUi(this);

    setupPlot();
    setupConnections();
    ui->baudComboBox->addItem("921600", 921600);
    ui->baudComboBox->addItem("460800", 460800);
    ui->baudComboBox->addItem("230400", 230400);
    ui->baudComboBox->addItem("115200", 115200);
    ui->baudComboBox->addItem("9600", 9600);

    ui->baudComboBox->setCurrentIndex(0);

    ui->timerClockSpinBox->setValue(144000000);
    ui->samplesSpinBox->setValue(20);

    ui->startFreqSpinBox->setValue(100);
    ui->stopFreqSpinBox->setValue(10000);
    ui->stepFreqSpinBox->setValue(100);

    ui->resultTable->setColumnWidth(0, 110);
    ui->resultTable->setColumnWidth(1, 90);
    ui->resultTable->setColumnWidth(2, 90);
    ui->resultTable->setColumnWidth(3, 90);

    refreshPorts();

    logMessage("Application ready.");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupPlot()
{
//    m_plot = new BodePlotWidget(this);
//    ui->plotLayout->addWidget(m_plot);
    dyno_plot = new DynamicBodePlotWidget(this);
    ui->plotLayout->addWidget(dyno_plot);
}

void MainWindow::setupConnections()
{
    connect(ui->refreshButton, SIGNAL(clicked()),
            this, SLOT(refreshPorts()));

    connect(ui->connectButton, SIGNAL(clicked()),
            this, SLOT(connectSerial()));

    connect(ui->disconnectButton, SIGNAL(clicked()),
            this, SLOT(disconnectSerial()));

    connect(ui->startButton, SIGNAL(clicked()),
            this, SLOT(startSweep()));

    connect(ui->stopButton, SIGNAL(clicked()),
            this, SLOT(stopSweep()));

    connect(&m_serial, SIGNAL(rawLineReceived(QString)),
            this, SLOT(logMessage(QString)));

    connect(&m_serial, SIGNAL(errorOccurred(QString)),
            this, SLOT(showError(QString)));

    connect(&m_sweep, SIGNAL(measurementReady(MeasurementPoint)),
            this, SLOT(onMeasurementReady(MeasurementPoint)));

    connect(&m_sweep, SIGNAL(message(QString)),
            this, SLOT(logMessage(QString)));

    connect(&m_sweep, SIGNAL(errorOccurred(QString)),
            this, SLOT(showError(QString)));

    connect(&m_sweep, SIGNAL(progressChanged(int,int)),
            ui->progressBar, SLOT(setValue(int)));
}

void MainWindow::refreshPorts()
{
    ui->portComboBox->clear();
    ui->portComboBox->addItems(m_serial.availablePorts());
}

void MainWindow::connectSerial()
{
    QString port = ui->portComboBox->currentText();
    qint32 baud = ui->baudComboBox->itemData(ui->baudComboBox->currentIndex()).toInt();

    if (port.isEmpty())
    {
        showError("No serial port selected.");
        return;
    }

    if (m_serial.openPort(port, baud))
        logMessage("Serial port opened: " + port);
}

void MainWindow::disconnectSerial()
{
    m_serial.closePort();
    logMessage("Serial port closed.");
}

void MainWindow::startSweep()
{
    ui->resultTable->setRowCount(0);
//    m_plot->clear();
    dyno_plot->clear();

    double fStart = ui->startFreqSpinBox->value();
    double fStop = ui->stopFreqSpinBox->value();
    double fStep = ui->stepFreqSpinBox->value();
    double timerClock = ui->timerClockSpinBox->value();
    int samples = ui->samplesSpinBox->value();

//    m_plot->setFrequencyRange(qMax(1.0, fStart), qMax(fStart * 10.0, fStop));

    int total = static_cast<int>((fStop - fStart) / fStep) + 1;
    if (total < 0)
        total = 0;

    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(0);

    m_sweep.start(fStart, fStop, fStep, timerClock, samples);
}

void MainWindow::stopSweep()
{
    m_sweep.stop();
}

void MainWindow::onMeasurementReady(const MeasurementPoint &m)
{
//    m_plot->addPoint(m.frequencyHz, m.gainDb);

    dyno_plot->addPoint(m.frequencyHz, m.gainDb);

    int row = ui->resultTable->rowCount();
    ui->resultTable->insertRow(row);

    ui->resultTable->setItem(row, 0, new QTableWidgetItem(QString::number(m.frequencyHz, 'f', 3)));
    ui->resultTable->setItem(row, 1, new QTableWidgetItem(QString::number(m.vinRms, 'f', 6)));
    ui->resultTable->setItem(row, 2, new QTableWidgetItem(QString::number(m.voutRms, 'f', 6)));
    ui->resultTable->setItem(row, 3, new QTableWidgetItem(QString::number(m.gainDb, 'f', 3)));
}

void MainWindow::logMessage(const QString &text)
{
    QString t = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    ui->logTextEdit->append(QString("[%1] %2").arg(t).arg(text));
}

void MainWindow::showError(const QString &text)
{
    logMessage("ERROR: " + text);
}

void MainWindow::copyTableViewToClipboard(QTableView *tableView)
{
    qDebug() << "Copy" << endl;
    QAbstractItemModel *model = tableView->model();

    QString text;

    // Optional: Copy headers
    for (int col = 0; col < model->columnCount(); ++col)
    {
        text += model->headerData(col, Qt::Horizontal).toString();

        if (col < model->columnCount() - 1)
            text += "\t";
    }
    text += "\n";

    // Copy data
    for (int row = 0; row < model->rowCount(); ++row)
    {
        for (int col = 0; col < model->columnCount(); ++col)
        {
            QModelIndex index = model->index(row, col);
            text += model->data(index).toString();

            if (col < model->columnCount() - 1)
                text += "\t";
        }

        if (row < model->rowCount() - 1)
            text += "\n";
    }

    QApplication::clipboard()->setText(text);
}

void MainWindow::on_resultTable_cellDoubleClicked(int row, int column)
{
    ui->resultTable->selectAll();
    copyTableViewToClipboard(ui->resultTable);
}
