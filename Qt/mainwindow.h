#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>

#include "serialmanager.h"
#include "sweepcontroller.h"
#include "bodeplotwidget.h"
#include "dynamicbodeplotwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void refreshPorts();
    void connectSerial();
    void disconnectSerial();
    void startSweep();
    void stopSweep();

    void onMeasurementReady(const MeasurementPoint &m);
    void logMessage(const QString &text);
    void showError(const QString &text);

    void copyTableViewToClipboard(QTableView *tableView);

    void on_resultTable_cellDoubleClicked(int row, int column);

private:
    Ui::MainWindow *ui;

    SerialManager m_serial;
    SweepController m_sweep;

//    BodePlotWidget *m_plot;
    DynamicBodePlotWidget *dyno_plot;

    void setupPlot();
    void setupConnections();
};

#endif // MAINWINDOW_H
