#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCharts>

#include <QWidget>
#include <QVector>
#include "ui_raindrop_spectrum.h"
#include "qcustomplot.h"


class raindrop_spectrum : public QMainWindow
{
    Q_OBJECT
	QChart* chart_ra = new QChart();
	QChart* chart_sg = new QChart();
	QChartView* mChart;
	QCPColorMap* colorMap;
	QCPColorScale* colorScale;
	QCPMarginGroup* marginGroup;

	QValueAxis* vaX;
	QValueAxis* vaY1;
	QSplineSeries* spY1;

	QBarCategoryAxis* axisX_ra = new QBarCategoryAxis();
	QValueAxis* axisY_ra = new QValueAxis();
	QStringList* time_list = new QStringList();

	HANDLE hDevice; //设备句柄
	LONG m_lChcnt = 1;
	ULONG   samcnt = 1024 * 100 * m_lChcnt;//采集样点数，1024 * 100 * m_lChcnt;
	PUSHORT inBuffer = new USHORT[samcnt];
	QTimer* timer1;
	QTimer* timer2;
	QTimer* timer_ra;
	QTimer* timer_sp;

	QBarSet* drop_set;
	QBarSeries* series_drop;
	int max_drop_amount =0;
	int max_drop_time_axis = 0;
	QVector<QString> x_axis;
	QVector<QString> y_axis;
	QVector<QVector<int>> sp_data;
	int max_amount = 0;
	QCategoryAxis* axis_th;
public:
	
    raindrop_spectrum(QWidget *parent = Q_NULLPTR);
	
	void timer(int ms);
	void show_rain_amount(QBarSeries* series_ra);
	QVector<double>labelPositions(const QVector<QString>& labels, double offset);
public slots:
	void connect_to_card();
	//void read_once();
	void read_consecutive();
	void stop_read();
	
	void timeSlot();
	void timeSlot_signal_ui();
	void timeSlot_ra();
	void show_spectrum();
	void showPointToolTip(QMouseEvent* event);
	void mouseMoved(QMouseEvent* event);
	//void show_rainfall_amount();
	
//protected:
	//virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
private:
    Ui::raindrop_spectrumClass ui;
	int MaxSample = 200;
	QVector<QPointF> vec_sg;
	double PI = 3.1415;
	int background_sg = 2200;
	int rain_counter = 0;
	bool check_drop=true;
	int max_ra_axis = 30;
	
	int lowest_v = 2500;

};
