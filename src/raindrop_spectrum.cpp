
#include <windows.h>
#include <thread>


#include <sys/timeb.h> 
#include "USB9825.h"
#include "raindrop_spectrum.h"
#include "qcustomplot.h"

#include <math.h>
#include <thread>
#include <wtypes.h>
#include <direct.h>
#include <iostream>

#include<qvector.h>

#include<ctime>
#include <time.h>
using namespace std;
#pragma execution_character_set("utf-8")

#define  MAX_SEGMENT 3
UCHAR  devNum;  //当前打开设备号
USHORT* dataBuff[MAX_SEGMENT];//采集信息缓冲，采用Block环形缓冲方式
BOOL    ADRun=true;//正在采集标志 
static string folderPath = "C:\\rain_data";
static char* folderPath_c = "C:\\rain_data";
/*
void raindrop_spectrum::DoDataExchange(CDataExchange* pDX)
{
	raindrop_spectrum::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFCADDlg)
	DDX_Control(pDX, IDC_CMBDIS, m_cmbDisch);
	DDX_Control(pDX, IDC_CMBCHACNT, m_cmbChCnt);
	DDX_Control(pDX, IDC_CMBSRC, m_cmbSrc);
	DDX_Control(pDX, IDC_CMBMODE, m_cmbMode);
	DDX_Text(pDX, IDC_EDTDIS, m_strOutput);
	DDX_Text(pDX, IDC_EDTTRIGLEN, m_nTrigLen);
	DDX_Text(pDX, IDC_EDTSDI, m_strDI);
	DDX_Text(pDX, IDC_EDTSDO, m_strDO);
	DDX_Text(pDX, IDC_EDTFREQ, m_nClkdeci);
	DDX_Text(pDX, IDC_EDTTRIGDELAY, m_nTrigDelay);
	DDX_Text(pDX, IDC_EDTADOFFSET, m_strADoffset);
	DDX_Text(pDX, IDC_EDTADOFFSET1, m_strADoffset1);
	DDX_Text(pDX, IDC_EDTADOFFSET2, m_strADoffset2);
	DDX_Text(pDX, IDC_EDTADOFFSET3, m_strADoffset3);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CMBCLK, m_cmbADCLK);
	DDX_Text(pDX, IDC_EDTPWMFREQ, m_pwmFreq);
	DDX_Check(pDX, IDC_CHKPWM, m_bEnPWM);
	DDX_Text(pDX, IDC_EDTTRIGLEVEL, m_nTrigLevel);
}
*/
   
//连接采集卡
void raindrop_spectrum::connect_to_card() {
	int i = 0;
	//打开设备
	for (i = 0; i<MAX_SEGMENT; i++)
	{
		dataBuff[i] = NULL;
	}
	hDevice = INVALID_HANDLE_VALUE;
	//依次打开设备号0、1、2...31，如果只有一个卡连接，则设备号0，连接上第二个设备，
	//则设备号是1
	BOOL bOpen = FALSE;
	devNum = 0;
	for (i = 0; i<32; i++)
	{
		hDevice = USB9825_Link(i);
		qApp->processEvents();
		if (hDevice != INVALID_HANDLE_VALUE) {
			devNum = i;
			//连接成功
			ui.pushButton_cot->setText("连接成功");
			QString devNum_str = QString::number(devNum);
			
			qApp->processEvents();
			break;
		}
	}
	if (i == 32) {
		//没找到设备
		ui.pushButton_cot->setText("连接失败");
		qApp->processEvents();
	}


}
//单次采集测试
/*
void raindrop_spectrum::read_once() {
	samcnt = 1024 * 1024 * m_lChcnt;
	//分配内存
	PUSHORT inBuffer = NULL;
	inBuffer = new USHORT[samcnt];


	//一次读完samcnt长度数据，例如一个触发长度，如果samcnt大于READ_MAX_LEN，则分为多次读取
	ULONG DataOver = 0; //缓存区溢出指示
	ULONG rlen = 0;//每次读取长度
	ULONG alen = 0;//已经读取长度
	int rcnt = 0;//总共需要读取的次数
	if ((samcnt%READ_MAX_LEN) == 0)//如果读取的长度，刚好是最大允许读取长度的整数倍
		rcnt = (samcnt / READ_MAX_LEN);
	else
		rcnt = (samcnt / READ_MAX_LEN) + 1;
	//分多次读取，每次读最大长度是READ_MAX_LEN
	for (int i = 0; i<rcnt; i++)
	{
		if (i == (rcnt - 1))//如果是最后一次
		{
			if ((samcnt%READ_MAX_LEN) == 0)//如果刚好是整数倍
				rlen = READ_MAX_LEN;
			else
				rlen = samcnt%READ_MAX_LEN;
		}
		else
		{
			rlen = READ_MAX_LEN;
		}
		//等待缓存数据量达到要求
		//读数 
		if (!USB9825_ReadAD(hDevice, inBuffer + alen, rlen))
		{
			//ui.label_3->setText("读取数据失败");
			
			
			break;
		}
		//判断数据是否溢出
		alen += rlen;
	}


	//保存数据
	PUSHORT buf0 = new  USHORT[samcnt / m_lChcnt];//分配内存
	for (int k = 0; k<m_lChcnt; k++)//各通道，先取数据，再写盘
	{
		int OneCnt = samcnt / m_lChcnt;

		//先取数据
		for (int i = 0; i<OneCnt; i++)
		{
			buf0[i] = inBuffer[m_lChcnt*i + k];
		}

		char filename[100];
		sprintf(filename, "ad%d.bin", k);
		FILE *fp0 = fopen(filename, "wb");
		fwrite(buf0, sizeof(USHORT), OneCnt, fp0);
		fclose(fp0);
	}

	delete[] buf0;

}


//持续采集测试
void raindrop_spectrum::read_consecutive() {
	//ULONG status = FALSE;
	memset(inBuffer, 0, samcnt * sizeof(USHORT));
	ui.label_5->setText("正在采集");
	qApp->processEvents();
	//QBarSet* set0 = new QBarSet("Jane");
	//QBarSeries* series_ra = new QBarSeries();
	//int max_amount = 0;
	QLineSeries* series_sg = new QLineSeries();

	while (ADRun) {
		USB9825_ReadAD(hDevice, inBuffer, samcnt);
		//series_sg->append(index, inBuffer[index]);

		//if (index > 200) {
		//	series_sg->remove(0);
		//}
		//index++;
		timer(1);
		qApp->processEvents();
	}
	ui.label_5->setText("采集结束");
	qApp->processEvents();
	delete[] inBuffer;
	inBuffer = NULL;
}
*/


//停止采集
void raindrop_spectrum::stop_read() {
	ADRun = false;
	USB9825_StopAD(hDevice, devNum);
	timer1->stop();
	delete[] inBuffer;
	inBuffer = NULL;
	
}
//开始采集
void raindrop_spectrum::read_consecutive() {

	USB9825_PARA_INIT para_init;
	para_init.TriggerMode = TRIG_MODE_POST;//触发模式 TRIG_MODE_CONTINUE连续采集,TRIG_MODE_POST后触发
	para_init.TriggerSource = TRIG_SRC_EXT_RISING;//触发源  TRIG_SRC_EXT_RISING外正沿触发
	para_init.lSelADClk = ADCLK_INT;//选择采样钟 ADCLK_IN板上时钟 内时钟
	para_init.lChCnt = 1;//通道数
	para_init.ClkDeci = 0;//频率因子 //分频因子，0表示1分频 1表示2分频 2表示3分频
	para_init.TriggerLength = 1024;//触发长度,默认1024
	para_init.TriggerDelay = 100; //触发延时，仅延时触发有效，以基准时钟100M为单位
	para_init.TriggerLevel = 2048;//触发门限
	para_init.lADFmt = ADFMT_STBIN;//直接二进制输出

	if (USB9825_InitAD(hDevice, &para_init))
	{
		//ui.label_2->setText("初始化成功");
		if (USB9825_ExeSoftTrig(hDevice))
		{
			ui.label_3->setText("软件触发成功");
			qApp->processEvents();
			ADRun = true;
		}
	}
	else {
		ui.label_3->setText("初始化失败");
		qApp->processEvents();
	}

	mChart = new QChartView();//采集实时卡信号图
	mChart->setEnabled(true);
	mChart->chart()->setTitle("雨滴信号");
	mChart->chart()->legend()->hide();
	mChart->chart()->setTheme(QtCharts::QChart::ChartTheme::ChartThemeDark);

	//设置信号图信息
	vaX = new QValueAxis();
	vaY1 = new QValueAxis();
	spY1 = new QSplineSeries();
	vaX->setRange(0, MaxSample);
	vaX->setTickCount(7);
	vaX->setLabelsColor(QColor(0, 255, 0));
	vaX->setTitleText("采样点");
	vaY1->setRange(2000, 2400);
	vaY1->setTickCount(6);
	vaY1->setLabelsColor(QColor(255, 0, 0));
	vaY1->setTitleText("采集电压幅值（V）");
	mChart->chart()->addAxis(vaX, Qt::AlignBottom);
	mChart->chart()->addAxis(vaY1, Qt::AlignLeft);
	mChart->chart()->addSeries(spY1);
	axis_th = new QCategoryAxis;
	axis_th->append("阈值", background_sg);
	axis_th->setGridLinePen(QPen(QColor(0, 170, 140), 1.2, Qt::DotLine));
	mChart->chart()->addAxis(axis_th, Qt::AlignRight);
	spY1->attachAxis(axis_th);
	spY1->attachAxis(vaX);
	spY1->attachAxis(vaY1);
	spY1->setColor(QColor(0, 255, 0));
	spY1->setUseOpenGL();// 开启openGL加速

	//贴信号图到界面
	QHBoxLayout* hlay = new QHBoxLayout(ui.widget_signal);
	hlay->addWidget(mChart);
	this->setLayout(hlay);
	memset(inBuffer, 0, samcnt * sizeof(USHORT));
	qApp->processEvents();
	
	
	drop_set = new QBarSet("rain_count");
	series_drop = new QBarSeries();
	timer1 =new QTimer(this);
	timer1->setInterval(0.1);
	connect(timer1, SIGNAL(timeout()), this, SLOT(timeSlot()));//雨滴信号的实时识别和统计
	timer1->start();
	drop_set->setColor(QColor(44, 173, 243));
	timer_ra = new QTimer(this);
	timer_ra->setInterval(1000);
	connect(timer_ra, SIGNAL(timeout()), this, SLOT(timeSlot_ra()));//雨量统计图的时间变化
	timer_ra->start();
	colorMap = new QCPColorMap(ui.widget_spectrum->xAxis, ui.widget_spectrum->yAxis);
	colorScale = new QCPColorScale(ui.widget_spectrum);  // 构造一个色条
	marginGroup = new QCPMarginGroup(ui.widget_spectrum);
	for (int i = 2000; i < 2400; i=i+2) {
		x_axis.append(QString::number(i));
	}
	for (int i = 0; i < 200; i=i+2) {
		y_axis.append(QString::number(i));
	}
	show_spectrum();
	ui.widget_spectrum->replot();//显示雨滴速速度和尺寸谱
	qApp->processEvents();
}

//计算数据在页面的二位坐标
QVector<double> raindrop_spectrum::labelPositions(const QVector<QString>& labels, double offset)
{
	QVector<double> result(labels.size());
	for (int i = 0; i < labels.size(); ++i)
		result[i] = i + offset;
	return result; 
}

//雨滴速速度和尺寸谱
void raindrop_spectrum::show_spectrum() {
	//绘图设置
	ui.widget_spectrum->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
	ui.widget_spectrum->axisRect()->setupFullAxesBox(true);
	ui.widget_spectrum->xAxis->setLabel("雨滴直径");
	ui.widget_spectrum->yAxis->setLabel("雨滴速度");
	ui.widget_spectrum->xAxis->setLabelColor(Qt::white);
	ui.widget_spectrum->yAxis->setLabelColor(Qt::white);
	ui.widget_spectrum->setBackground(QBrush(QColor(67, 61, 60)));
	colorMap->data()->setSize(x_axis.size(), y_axis.size());
	colorMap->data()->setRange(QCPRange(2000, 2400), QCPRange(0, 200));  // 颜色图在x、y轴上的范围
	colorMap->setInterpolate(false);
	//QSharedPointer<QCPAxisTickerText> xTicker(new QCPAxisTickerText);
	//QSharedPointer<QCPAxisTickerText> yTicker(new QCPAxisTickerText);
	//xTicker->setTicks(labelPositions(x_axis, 0.5), x_axis);
	//xTicker->setTickStepStrategy(QCPAxisTicker::tssReadability);
	//yTicker->setTicks(labelPositions(y_axis, 0.5), y_axis);
	//xTicker->setSubTickCount(1);
	//yTicker->setSubTickCount(1);
	//ui.widget_spectrum->xAxis->setTicker(xTicker);
	//ui.widget_spectrum->yAxis->setTicker(yTicker);

	//设置文字和网格尺寸和颜色
	ui.widget_spectrum->xAxis->setBasePen(QPen(Qt::white, 1));
	ui.widget_spectrum->yAxis->setBasePen(QPen(Qt::white, 1));
	ui.widget_spectrum->xAxis->setTickPen(QPen(Qt::white, 1));
	ui.widget_spectrum->yAxis->setTickPen(QPen(Qt::white, 1));
	ui.widget_spectrum->xAxis->setSubTickPen(QPen(Qt::white, 1));
	ui.widget_spectrum->yAxis->setSubTickPen(QPen(Qt::white, 1));
	ui.widget_spectrum->xAxis->setTickLabelColor(Qt::white);
	ui.widget_spectrum->yAxis->setTickLabelColor(Qt::white);
	ui.widget_spectrum->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
	ui.widget_spectrum->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
	ui.widget_spectrum->xAxis->ticker()->setTickCount(10);

	//设置数据范围
	ui.widget_spectrum->xAxis->setRange(2000, 2400);
	ui.widget_spectrum->yAxis->setRange(0, 200);

	//设置色彩地图
	colorMap->data()->fillAlpha(0);
	colorScale->setType(QCPAxis::atBottom);   
	colorScale->axis()->setBasePen(QPen(Qt::black, 1));
	colorScale->axis()->setTickPen(QPen(Qt::black, 1));
	colorScale->axis()->setSubTickPen(QPen(Qt::black, 1));
	colorScale->axis()->setTickLabelColor(Qt::white);
	ui.widget_spectrum->plotLayout()->addElement(1, 0, colorScale); // 在颜色图下面显示
	colorMap->setColorScale(colorScale);
	QCPColorGradient gradient;  // 色条使用的颜色渐变
	gradient.setColorStopAt(0.0, QColor("#f6efa6"));   // 设置色条开始时的颜色
	gradient.setColorStopAt(1.0, QColor("#bf444c"));  // 设置色条结束时的颜色
	colorMap->setGradient(gradient);
	ui.widget_spectrum->axisRect()->setMarginGroup(QCP::msLeft | QCP::msRight, marginGroup);
	colorScale->setMarginGroup(QCP::msLeft | QCP::msRight, marginGroup);
	qApp->processEvents();
	/*
	//色彩热力图测试
	ui.widget_spectrum->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
	ui.widget_spectrum->axisRect()->setupFullAxesBox(true);
	ui.widget_spectrum->xAxis->setLabel("雨滴直径");
	ui.widget_spectrum->yAxis->setLabel("雨滴速度");
	// set up the QCPColorMap:
	QCPColorMap* colorMap = new QCPColorMap(ui.widget_spectrum->xAxis, ui.widget_spectrum->yAxis);
	int nx = 4;
	int ny = 4;
	colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
	colorMap->data()->setRange(QCPRange(2010, 2014), QCPRange(30, 34)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
	// now we assign some data, by accessing the QCPColorMapData instance of the color map:
	
	
	for (int xIndex = 0; xIndex < nx; ++xIndex)
	{
		for (int yIndex = 0; yIndex < ny; ++yIndex)
		{
			colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
			double r = 3 * qSqrt(x * x + y * y) + 1e-2;
			z = 2 * x * (qCos(r + 2) / r - qSin(r + 2) / r); // the B field strength of dipole radiation (modulo physical constants)
			colorMap->data()->setCell(xIndex, yIndex, z);
		}
	}
	
	colorMap->data()->setCell(2011, 31,2);
	colorMap->data()->setCell(2012, 32, 10);
	// add a color scale:
	QCPColorScale* colorScale = new QCPColorScale(ui.widget_spectrum);
	ui.widget_spectrum->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
	colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
	colorMap->setColorScale(colorScale); // associate the color map with the color scale
	colorScale->axis()->setLabel("Magnetic Field Strength");

	// set the color gradient of the color map to one of the presets:
	colorMap->setGradient(QCPColorGradient::gpPolar);
	// we could have also created a QCPColorGradient instance and added own colors to
	// the gradient, see the documentation of QCPColorGradient for what's possible.

	// rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
	colorMap->rescaleDataRange();

	// make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
	QCPMarginGroup* marginGroup = new QCPMarginGroup(ui.widget_spectrum);
	ui.widget_spectrum->axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
	colorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);

	// rescale the key (x) and value (y) axes so the whole color map is visible:
	ui.widget_spectrum->rescaleAxes();
	qApp->processEvents();

	*/
}

//雨量统计图
void raindrop_spectrum::show_rain_amount(QBarSeries* series_ra) {

	chart_ra->setTheme(QtCharts::QChart::ChartTheme::ChartThemeDark);
	chart_ra->setTitle("降雨量统计");
	QString timeStamp = QString("");
	QString datestr = QString("");
	QString timestr = QString("");
	SYSTEMTIME sys;//获取系统时间
	GetLocalTime(&sys);
	QString yearStr = QString("%1").arg(sys.wYear, 4, 10, QLatin1Char('0'));
	QString monthStr = QString("%1").arg(sys.wMonth, 2, 10, QLatin1Char('0'));
	QString dayStr = QString("%1").arg(sys.wDay, 2, 10, QLatin1Char('0'));
	QString hourStr = QString("%1").arg(sys.wHour, 2, 10, QLatin1Char('0'));
	QString minuteStr = QString("%1").arg(sys.wMinute, 2, 10, QLatin1Char('0'));
	//时间坐标轴
	timeStamp = minuteStr;
	datestr = yearStr + "年" + monthStr + "月" + dayStr + "日";
	timestr = hourStr + ":" + minuteStr;
	ui.label_13->setText(datestr);
	ui.label_22->setText(timestr);
	axisX_ra->append(timeStamp);
	time_list->append(timeStamp);
	if (time_list->size() > max_ra_axis) {
		time_list->removeFirst();
		axisX_ra->setCategories(*time_list);
	}
	//绘图设置
	QFont font;
	font.setPixelSize(18);
	chart_ra->setTitleFont(font);
	axisX_ra->setTitleFont(font);
	axisY_ra->setTitleFont(font);
	chart_ra->setTitleBrush(QBrush(Qt::white));
	QFont labelsFont;
	labelsFont.setPixelSize(12);
	axisX_ra->setLabelsFont(labelsFont);
	axisY_ra->setLabelsFont(labelsFont);
	QPen axisPen(QRgb(0xd18952));
	axisPen.setWidth(2);
	axisX_ra->setLinePen(axisPen);
	axisY_ra->setLinePen(axisPen);
	QBrush axisBrush(Qt::white);
	axisX_ra->setLabelsBrush(axisBrush);
	axisX_ra->setTitleBrush(axisBrush);
	axisY_ra->setTitleBrush(axisBrush);
	axisY_ra->setLabelsBrush(axisBrush);
	axisY_ra->setShadesPen(Qt::NoPen);
	axisY_ra->setShadesBrush(QBrush(QColor(0x99, 0xcc, 0xcc, 0x55)));
	axisY_ra->setShadesVisible(true);
	axisX_ra->setTitleText("时间(分钟) 更新时间：" + hourStr+"点"+ minuteStr +"分");
	chart_ra->addAxis(axisX_ra, Qt::AlignBottom);
	series_ra->attachAxis(axisX_ra);
	axisY_ra->setTitleText("雨滴数量");
	axisY_ra->setRange(0, max_amount);
	chart_ra->addAxis(axisY_ra, Qt::AlignLeft);
	series_ra->attachAxis(axisY_ra);
	chart_ra->legend()->hide();
	series_ra->setLabelsPosition(QAbstractBarSeries::LabelsInsideEnd); // 设置数据系列标签的位置于数据柱内测上方
	series_ra->setLabelsVisible(true);
	chart_ra->addSeries(series_ra);

	ui.widget_3->setChart(chart_ra);
	//雨量大小标识
	if (rain_counter == 0) {
		ui.label_12->setText("晴");
		ui.label_12->setStyleSheet("background-color: rgb(233, 196, 150);");
	}
	else if (rain_counter < 5) {
		ui.label_12->setText("小雨");
		ui.label_12->setStyleSheet("background-color: rgb(0, 170, 255);");
	}
	else if (rain_counter < 15) {
		ui.label_12->setText("中雨");
		ui.label_12->setStyleSheet("background-color: rgb(0, 147, 221);");
	}
	else if (rain_counter < 30) {
		ui.label_12->setText("大雨");
		ui.label_12->setStyleSheet("background-color: rgb(0, 111, 166);");
	}
	else{
		ui.label_12->setText("暴雨");
		ui.label_12->setStyleSheet("background-color: rgb(255, 85, 0);");
	}
	
	qApp->processEvents();
}

//等待计时器
void raindrop_spectrum::timer(int ms) {

	for (int i=1; i < ms; i++) {
		Sleep(1);
		qApp->processEvents();
	}
}

//雨量统计的横坐标的分钟变化
void raindrop_spectrum::timeSlot_ra() {
	static SYSTEMTIME sys;//获取系统时间
	static QString old_min = "";
	GetLocalTime(&sys);
	QString minuteStr = QString("%1").arg(sys.wMinute, 2, 10, QLatin1Char('0'));
	//QString secondStr = QString("%1").arg(sys.wSecond, 2, 10, QLatin1Char('0'));
	if (old_min != minuteStr) {
		old_min = minuteStr;
		if (max_amount<rain_counter) {
			max_amount = rain_counter;
		}
		drop_set->append(rain_counter);
		if (max_drop_time_axis > max_ra_axis) {
			drop_set->remove(0);
			series_drop->remove(0);
		}
		series_drop->append(drop_set);
		show_rain_amount(series_drop);
		rain_counter = 0;
		max_drop_time_axis++;
	}
}

//雨滴信号识别和统计
void raindrop_spectrum::timeSlot() {	
	/*
	static int cnt = 0;
	if (cnt <= MaxSample)
	{
		float tmp = 85.0 + 85 * sin(cnt * 4 * PI / MaxSample);
		vecKv.push_back(QPointF(cnt, tmp));
	}
	else
	{
		for (int i = 1; i < MaxSample; ++i)
		{
			vecKv[i - 1] = QPointF(vecKv[i - 1].x(), vecKv[i].y());
			//vecVacc[i - 1] = QPointF(vecVacc[i - 1].x(), vecVacc[i].y());
		}
		float tmp = 85.0 + 85 * sin(cnt * 4 * PI / MaxSample);
		vecKv[MaxSample - 1] = QPointF(vecKv[MaxSample - 1].x(), tmp);
		//tmp = 5 + 5 * cos(cnt * 4 * PI / MaxSample);
		//vecVacc[MaxSample - 1] = QPointF(vecVacc[MaxSample - 1].x(), tmp);

		if (cnt >= 3 * MaxSample)// 计数器复位
		{
			cnt = 0;
			vecKv.clear();
			//vecVacc.clear();
		}
	}
	cnt++;
	*/
	static int index = 0;
	static clock_t start, finish;
	double sp_x, sp_y;
	USB9825_ReadAD(hDevice, inBuffer, samcnt);//读取采集卡的信号到inbuffer
	if (index <= MaxSample) {
		vec_sg.push_back(QPointF(index, inBuffer[index]));//信号没填满图
	}
	else {
		//信号图平移
		for (int i = 1; i < MaxSample; ++i)
		{
			vec_sg[i - 1] = QPointF(vec_sg[i - 1].x(), vec_sg[i].y());
		}
		vec_sg[MaxSample - 1] = QPointF(vec_sg[MaxSample - 1].x(), inBuffer[index]);
	}

	if (lowest_v>inBuffer[index] ) {
		lowest_v = inBuffer[index];
	}
	//标记识别到的雨滴，下落过程中，信号下降
	if ((inBuffer[index] < background_sg)&& check_drop ) {
		start = clock();
		lowest_v = 2500;
		check_drop = false;
		rain_counter++;
		QString bg_cont_str = QString::number(rain_counter);
		ui.label_8->setText(bg_cont_str);
		qApp->processEvents();
	}
	//标记完毕，下落过程结束，信号回弹到背景信号
	if ((inBuffer[index] >= background_sg)&& (!check_drop)) {
		int duration=0;
		finish = clock();
		check_drop = true;
		QString bg_v_str = QString::number(lowest_v);
		ui.label_9->setText(bg_v_str);
		duration = finish - start;
		QString bg_time_str = QString::number(duration);
		bg_time_str = bg_time_str + "ms";
		ui.label_11->setText(bg_time_str);
		colorMap->data()->setAlpha(lowest_v - 2000, duration, 255);
		colorMap->data()->setCell(lowest_v - 2000, duration, 1);
		ui.widget_spectrum->replot();
		qApp->processEvents();

		//记录雨滴数据
		time_t calendar_time = time(NULL);
		struct tm* tm_local = localtime(&calendar_time);
		char str_f_t[20];
		char str_f_name[50];
		strftime(str_f_t, sizeof(str_f_t), "%G-%m-%d %H:%M:%S", tm_local);
		strftime(str_f_name, sizeof(str_f_name), "C:\\rain_data\\Rain_record%G-%m-%d.csv", tm_local);
		char drop_data[100];
		sprintf(drop_data, "%s,%d,%d\n", str_f_t, lowest_v, duration);
		//char write_name[100];
		//std::string const& cc = std::string(folderPath_c) + std::string(str_f_name);
		//char const* name_w = cc.c_str();
		//strcat(folderPath_c, str_f_name);
		
		FILE* fp_drop = fopen(str_f_name, "a");
		fprintf(fp_drop, drop_data);
		fclose(fp_drop);

		 
	}
	if (inBuffer[index + 1] != NULL) {
		index++;
		ui.label_5->setText(QString::number(index));
	}
	
	spY1->replace(vec_sg);
	qApp->processEvents();
}
//背景阈值信号调整
void raindrop_spectrum::timeSlot_signal_ui() {
	background_sg = ui.spinBox_3->value();
	axis_th->remove("阈值");
	axis_th->append("阈值", background_sg);
}

//鼠标位置测试
void raindrop_spectrum::showPointToolTip(QMouseEvent* event)
{
	int x = ui.widget_spectrum->xAxis->pixelToCoord(event->pos().x());
	int y = ui.widget_spectrum->yAxis->pixelToCoord(event->pos().y());
	setToolTip(QString("%1 , %2").arg(x).arg(y));
}

//鼠标移动指示
void raindrop_spectrum::mouseMoved(QMouseEvent* event)
{
	int x, y;
	x = this->ui.widget_spectrum->xAxis->pixelToCoord(event->pos().x());
	y = this->ui.widget_spectrum->yAxis->pixelToCoord(event->pos().y());
	ui.label->setText("直径:"+QString::number(x));
	ui.label_2->setText("速度:" + QString::number(y));
	
}

//界面布局
raindrop_spectrum::raindrop_spectrum(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	
	mkdir(folderPath.c_str());   // 返回 0 表示创建成功，-1 表示失败

	time_t calendar_time = time(NULL);
	struct tm* tm_local = localtime(&calendar_time);
	char str_f_t[100];
	char str_f_name[100];
	strftime(str_f_t, sizeof(str_f_t), "%G-%m-%d %H:%M:%S", tm_local);
	strftime(str_f_name, sizeof(str_f_name), "C:\\rain_data\\Rain_record%G-%m-%d.csv", tm_local);
	FILE* fp_drop = fopen(str_f_name, "a");
	fprintf(fp_drop, "开启时间：,%s\n 时间,雨滴最低幅度(V)(雨滴大小),雨滴速度\n", str_f_t);
	fclose(fp_drop);
	
	ui.widget_spectrum->setMouseTracking(true);
	connect_to_card();
	read_consecutive();
	connect(ui.pushButton_cot, SIGNAL(clicked()), this, SLOT(connect_to_card()));
	//connect(ui.pushButton_one, SIGNAL(clicked()), this, SLOT(read_once()));
	connect(ui.pushButton_stop, SIGNAL(clicked()), this, SLOT(stop_read()));

	connect(ui.pushButton_consecutive, SIGNAL(clicked()), this, SLOT(read_consecutive()));
	connect(ui.pushButton_consecutive, SIGNAL(clicked()), this, SLOT(show_spectrum()));
	connect(ui.spinBox_3, SIGNAL(valueChanged(int)), this, SLOT(timeSlot_signal_ui()));
	//connect(this, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));
	connect(ui.widget_spectrum, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent *)));

}


/*
	QCustomPlot* customPlot = new QCustomPlot(ui.widget_signal);
	customPlot->resize(541, 331);

	//可变数组存放绘图的坐标的数据，分别存放x和y坐标的数据，101为数据长度
	QVector<double> x(101), y(101);

	//添加数据，这里演示y=x^3，为了正负对称，x从-10到+10
	for (int i = 0; i < 101; i++)
	{
		x[i] = i / 5 - 10;
		y[i] = qPow(x[i], 3);//x的y次方;
	}

	//向绘图区域QCustomPlot添加一条曲线


	customPlot->addGraph();

	//添加数据
	customPlot->graph(0)->setData(x, y);

	//设置坐标轴名称
	customPlot->xAxis->setLabel("x");
	customPlot->yAxis->setLabel("y");

	//设置坐标轴显示范围，否则只能看到默认范围
	customPlot->xAxis->setRange(-11, 11);
	customPlot->yAxis->setRange(-1100, 1100);
	qApp->processEvents();*/