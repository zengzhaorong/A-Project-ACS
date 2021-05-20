#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include "config.h"
#include "mainwindow.h"
#include "image_convert.h"


/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "protocol.h"
#include "capture.h"
#include "public.h"
#ifdef __cplusplus
}
#endif


static MainWindow *mainwindow;
extern struct main_mngr_info main_mngr;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	int y_pix = 0;
	int funcArea_width;
	int funcArea_height;
	int widget_height;
	QImage image;
	QFont font;
	QPalette pa;
	QTextCodec *codec = QTextCodec::codecForName("GBK");

	funcArea_width = CONFIG_WINDOW_WIDTH(main_mngr.config_ini) -CONFIG_CAPTURE_WIDTH(main_mngr.config_ini);
	funcArea_height = CONFIG_WINDOW_HEIGHT(main_mngr.config_ini);
	
	/* can show Chinese word */
	setWindowTitle(codec->toUnicode(CONFIG_WINDOW_TITLE(main_mngr.config_ini)));
	
	resize(CONFIG_WINDOW_WIDTH(main_mngr.config_ini), CONFIG_WINDOW_HEIGHT(main_mngr.config_ini));

	mainWindow = new QWidget;
	setCentralWidget(mainWindow);

	backgroundImg.load(WIN_BACKGRD_IMG);

	/* show video area */
	videoArea = new QLabel(mainWindow);
	videoArea->setPixmap(QPixmap::fromImage(backgroundImg));
	videoArea->setGeometry(0, 0, CONFIG_CAPTURE_WIDTH(main_mngr.config_ini), CONFIG_CAPTURE_HEIGH(main_mngr.config_ini));
	videoArea->show();
	
	font.setPointSize(CONFIG_WINDOW_FONT_SIZE(main_mngr.config_ini));
	pa.setColor(QPalette::WindowText,Qt::yellow);
	textOnVideo = new QLabel(mainWindow);
	textOnVideo->setFont(font);
	textOnVideo->setPalette(pa);

	/* clock */
	y_pix += Y_INTERV_PIXEL_EX;
	widget_height = 90;
	clockLabel = new QLabel(mainWindow);
	clockLabel->setWordWrap(true);	// adapt to text, can show multi row
	clockLabel->setGeometry(FUNC_AREA_PIXEL_X +5, y_pix, funcArea_width, widget_height);	// height: set more bigger to adapt to arm
	clockLabel->show();
	y_pix += widget_height;

#ifdef MANAGER_CLIENT_ENABLE
	image.load(EXTRAINFO_MNGR_IMG);
	widget_height = 130;
	(void)funcArea_height;
#else
	image.load(EXTRAINFO_USER_IMG);
	widget_height = funcArea_height - y_pix;
#endif
	extraInfo = new QLabel(mainWindow);
	extraInfo->setPixmap(QPixmap::fromImage(image));
	extraInfo->setGeometry(FUNC_AREA_PIXEL_X, y_pix, funcArea_width, widget_height);
	extraInfo->show();
	y_pix += widget_height;

#ifdef MANAGER_CLIENT_ENABLE

	/* user id - name edit */
	y_pix += Y_INTERV_PIXEL_EX;
	userIdEdit = new QLineEdit(mainWindow);
	userIdEdit->setPlaceholderText(codec->toUnicode(TEXT_USER_ID));
	userIdEdit->setGeometry(FUNC_AREA_PIXEL_X +5, y_pix, 50, WIDGET_HEIGHT_PIXEL);
	userNameEdit = new QLineEdit(mainWindow);
	userNameEdit->setPlaceholderText(codec->toUnicode(TEXT_USER_NAME));
	userNameEdit->setGeometry(FUNC_AREA_PIXEL_X +5 +50 +2, y_pix, 100, WIDGET_HEIGHT_PIXEL);
	y_pix += WIDGET_HEIGHT_PIXEL;
	/* add user button */
	y_pix += Y_INTERV_PIXEL_IN;
	addUserBtn = new QPushButton(mainWindow);
	addUserBtn->setText(codec->toUnicode(TEXT_ADD_USER));
    connect(addUserBtn, SIGNAL(clicked()), this, SLOT(addUser()));
	addUserBtn->setGeometry(FUNC_AREA_PIXEL_X +30, y_pix, 100, WIDGET_HEIGHT_PIXEL);
	y_pix += WIDGET_HEIGHT_PIXEL;

	/* user list box */
	y_pix += Y_INTERV_PIXEL_EX;
	userListBox = new QComboBox(mainWindow);
	userListBox->setGeometry(FUNC_AREA_PIXEL_X +5, y_pix, 150, WIDGET_HEIGHT_PIXEL);
	userListBox->setEditable(true);
	y_pix += WIDGET_HEIGHT_PIXEL;
	/* delete user button */
	y_pix += Y_INTERV_PIXEL_IN;
	delUserBtn = new QPushButton(mainWindow);
	delUserBtn->setText(codec->toUnicode(TEXT_DEL_USER));
    connect(delUserBtn, SIGNAL(clicked()), this, SLOT(deleteUser()));
	delUserBtn->setGeometry(FUNC_AREA_PIXEL_X +30, y_pix, 100, WIDGET_HEIGHT_PIXEL);
	y_pix += WIDGET_HEIGHT_PIXEL;

	tableView = new QTableView(mainWindow);
	userModel = new QStandardItemModel();
	userModel->setHorizontalHeaderLabels({"ID", "Name", "Time", "Status"});
	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);		// set select the whole row 
	//tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);	// adapt to table veiw
	tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents );	// adapt to text
	tableView->setGeometry(0, 0, CONFIG_CAPTURE_WIDTH(main_mngr.config_ini), CONFIG_CAPTURE_HEIGH(main_mngr.config_ini));
	tableView->hide();
#endif

#if !defined(USER_CLIENT_ENABLE) && defined(MANAGER_CLIENT_ENABLE)
	switchCaptureBtn = new QPushButton(mainWindow);
	switchCaptureBtn->setText(codec->toUnicode(TEXT_SWIT_CAPTURE));
    connect(switchCaptureBtn, SIGNAL(clicked()), this, SLOT(switchCapture()));
	switchCaptureBtn->setGeometry(FUNC_AREA_PIXEL_X -80, CONFIG_CAPTURE_HEIGH(main_mngr.config_ini) -WIDGET_HEIGHT_PIXEL, 80, WIDGET_HEIGHT_PIXEL);
#endif

	buf_size = CONFIG_CAPTURE_WIDTH(main_mngr.config_ini) *CONFIG_CAPTURE_HEIGH(main_mngr.config_ini) *3;
	video_buf = (unsigned char *)malloc(buf_size);
	if(video_buf == NULL)
	{
		buf_size = 0;
		printf("ERROR: malloc for video_buf failed!");
	}

	/* set timer to show image */
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(showMainwindow()));
	timer->start(TIMER_INTERV_MS);

	tmpShowTimer = new QTimer(this);

	sys_state = &main_mngr.work_state;

}

MainWindow::~MainWindow(void)
{
	
}

void MainWindow::showMainwindow(void)
{
	static int old_state = WORK_STA_NORMAL;
	mainwin_mode_e mode;
	int len;
	int ret;

	timer->stop();

	QDateTime time = QDateTime::currentDateTime();
	QString str = time.toString("yyyy-MM-dd hh:mm:ss dddd");
	clockLabel->setText(str);

	/* show capture image */
	ret = capture_get_newframe(video_buf, buf_size, &len);
	if(ret == 0)
	{
		QImage videoQImage;

		videoQImage = jpeg_to_QImage(video_buf, len);

		/* draw face rectangles */
		drawFaceRectangle(videoQImage);

		videoArea->setPixmap(QPixmap::fromImage(videoQImage));
		videoArea->show();
	}

	if(*sys_state != old_state)
	{
		if(*sys_state == WORK_STA_DISCONNECT)
		{
			mode = MAINWIN_MODE_DISCONECT;
		}
		else if(*sys_state == WORK_STA_ADDUSER)
		{
			mode = MAINWIN_MODE_ADDUSER;
		}
		else if(old_state==WORK_STA_ADDUSER && *sys_state==WORK_STA_NORMAL)
		{
			mode = MAINWIN_MODE_ADDUSER_OK;
		}
		else if(*sys_state == WORK_STA_RECOGN)
		{
			mode = MAINWIN_MODE_RECOGN;
		}
		else
		{
			mode = MAINWIN_MODE_NORAML;
		}
		
		switch_mainwin_mode(mode);
		old_state = *sys_state;
	}

	timer->start(TIMER_INTERV_MS);
	
}

void MainWindow::drawFaceRectangle(QImage &img)
{
	static QRect old_rect;
	static int old_rect_cnt = 0;
	QRect rects;
	QPainter painter(&img);

	if(face_rects.width() > 0)
	{
		old_rect = face_rects;
		face_rects.setWidth(0);
		old_rect_cnt = 0;
	}
	if(old_rect.width() > 0)
	{
		painter.setPen(QPen(Qt::green, 3, Qt::SolidLine, Qt::RoundCap));
		painter.drawRect(old_rect.x(), old_rect.y(), old_rect.width(), old_rect.height());
		old_rect_cnt ++;
		if(old_rect_cnt *TIMER_INTERV_MS > FACE_RECT_LASTING_MS)
		{
			old_rect.setWidth(0);
			old_rect_cnt = 0;
		}
	}

}

void MainWindow::addUser(void)
{
	char id_name[64] = {0};
	QString idStr;
	QString NameStr;
	QByteArray ba;
	char name[USER_NAME_LEN] = {0};
	int id;

	/* get input text */
	idStr = userIdEdit->text();
	NameStr = userNameEdit->text();
	if(idStr.length() <= 0 || NameStr.length() <= 0)
	{
		printf("%s: QLineEdit is empty !\n", __FUNCTION__);
		return ;
	}

	ba = idStr.toLatin1();
	id = atoi(ba.data());
	if(id <= 0)
	{
		printf("%s: ID is illegal !\n", __FUNCTION__);
		return ;
	}

	/* close remote capture */
	if(main_mngr.capture_flag == 1)
	{
		proto_0x20_switchCapture(main_mngr.mngr_handle, 0);
		main_mngr.capture_flag = 0;
		v4l2cap_clear_newframe();
	}

	ba = NameStr.toLatin1();
	memset(name, 0, sizeof(name));
	strncpy((char *)name, ba.data(), strlen(ba.data()));

	printf("user Id: %d, name: %s\n", id, name);
	memcpy(id_name, &id, 4);
	memcpy(id_name +4, name, USER_NAME_LEN);

	/* opencv camera */
	#if defined(MANAGER_CLIENT_ENABLE) && !defined(USER_CLIENT_ENABLE)
	start_capture_task();
	#endif

	main_mngr.work_state = WORK_STA_ADDUSER;

	proto_0x04_switchWorkSta(main_mngr.mngr_handle, main_mngr.work_state, (uint8_t *)id_name);
}

void MainWindow::deleteUser(void)
{
	QString qstrUsrName;
	char user_name[USER_NAME_LEN] = {0};
	QByteArray ba;

	qstrUsrName = userListBox->currentText();
	ba = qstrUsrName.toLatin1();
	strncpy(user_name, ba.data(), strlen(ba.data()));
	if(strlen(user_name) <= 0)
		return ;

	if(QMessageBox::warning(this,"Warning", "Delete "+userListBox->currentText()+" ?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::No)
	{
		return ;
	}

	proto_0x06_deleteUser(main_mngr.mngr_handle, 1, user_name);
	
	mainwin_set_userList(0, 1, user_name);
		
}

void MainWindow::switchCapture(void)
{

	if(main_mngr.capture_flag == 0)
	{
		main_mngr.capture_flag = 1;
		v4l2cap_clear_newframe();
	}
	else
	{
		main_mngr.capture_flag = 0;
		videoArea->setPixmap(QPixmap::fromImage(backgroundImg));
	}
	
	proto_0x20_switchCapture(main_mngr.mngr_handle, main_mngr.capture_flag);
}

void MainWindow::textOnVideo_show_over(void)
{
	tmpShowTimer->stop();
	textOnVideo->hide();
	
	mainwin_mode = MAINWIN_MODE_NORAML;

	*sys_state = WORK_STA_NORMAL;
}

/* switch mainwindow display mode: normal, add user, add user ok, recognzie ... */
int MainWindow::switch_mainwin_mode(mainwin_mode_e mode)
{
	QTextCodec *codec = QTextCodec::codecForName("GBK");
	char showText[128] = {0};
	int addface_x;

	mainwin_mode = mode;

	if(mode == MAINWIN_MODE_DISCONECT)
	{
		tmpShowTimer->stop();
		addface_x = 240;
		textOnVideo->setGeometry(addface_x, 0, CONFIG_CAPTURE_WIDTH(main_mngr.config_ini) -addface_x, 50);
		textOnVideo->setText(codec->toUnicode(NOT_CONNECT_SERVER));
		textOnVideo->show();
	}
	else if(mode == MAINWIN_MODE_NORAML)
	{
		textOnVideo->hide();
	}
	else if(mode == MAINWIN_MODE_ADDUSER)
	{
		tmpShowTimer->stop();
		addface_x = 200;
		textOnVideo->setGeometry(addface_x, 0, CONFIG_CAPTURE_WIDTH(main_mngr.config_ini) -addface_x, 50);
		textOnVideo->setText(codec->toUnicode(BEGIN_ADD_FACE_TEXT));
		textOnVideo->show();
	}
	else if(mode == MAINWIN_MODE_ADDUSER_OK)
	{
		addface_x = 230;
		textOnVideo->setGeometry(addface_x, 0, CONFIG_CAPTURE_WIDTH(main_mngr.config_ini) -addface_x, 50);
		textOnVideo->setText(codec->toUnicode(SUCCESS_ADD_FACE_TEXT));
		textOnVideo->show();
		QObject::connect(tmpShowTimer, SIGNAL(timeout()), this, SLOT(textOnVideo_show_over()));
		tmpShowTimer->start(TIMER_ADDUSER_OK_MS);
		/* close capture, show background image */
		#if defined(MANAGER_CLIENT_ENABLE) && !defined(USER_CLIENT_ENABLE)
		capture_task_stop();
		videoArea->setPixmap(QPixmap::fromImage(backgroundImg));
		#endif
	}
	else if(mode == MAINWIN_MODE_RECOGN)
	{
		addface_x = 200;
		textOnVideo->setGeometry(addface_x, 0, CONFIG_CAPTURE_WIDTH(main_mngr.config_ini) -addface_x, 50);
		
		sprintf(showText, "%s: %s - %d%c", TEXT_RECOGN_SUCCESS, userRecogn, confidence, '%');
		textOnVideo->setText(codec->toUnicode(showText));
		textOnVideo->show();
		QObject::connect(tmpShowTimer, SIGNAL(timeout()), this, SLOT(textOnVideo_show_over()));
		tmpShowTimer->start(CONFIG_FACE_RECOINTERVAL(main_mngr.config_ini));
	}
	else
	{
	}

	return 0;
}

int mainwin_set_rects(int x, int y, int w, int h)
{

	mainwindow->face_rects.setX(x);
	mainwindow->face_rects.setY(y);
	mainwindow->face_rects.setWidth(w);
	mainwindow->face_rects.setHeight(h);

	return 0;
}

/* flag: 0-delete, 1-add */
int mainwin_set_userList(int flag, int userCnt, char *usr_name)
{
	int index;
	int i;

	if(usr_name == NULL)
		return -1;

	for(i=0; i<userCnt; i++)
	{
		if(flag == 0)
		{
			/* delete user */
			index = mainwindow->userListBox->currentIndex();
			mainwindow->userListBox->removeItem(index);
		}
		else
		{
			/* add user */
			mainwindow->userListBox->addItem(usr_name +i*USER_NAME_LEN);
		}
	}

	return 0;
}

/* id: -1, only show usr_name */
int mainwin_set_recognInfo(int id, uint8_t confid, char *usr_name, int status)
{

	if(usr_name == NULL)
		return -1;

	memset(mainwindow->userRecogn, 0, sizeof(mainwindow->userRecogn));
	
	if(id == -1)
	{
		sprintf(mainwindow->userRecogn, "%s", usr_name);
	}
	else
	{
		mainwindow->face_id = id;
		strncpy(mainwindow->userRecogn, usr_name, strlen(usr_name));
	}

	mainwindow->confidence = confid;
	mainwindow->face_status = status;

	return 0;
}


int mainwindow_init(void)
{
	mainwindow = new MainWindow;

	mainwindow->show();
	
	return 0;
}

void mainwindow_deinit(void)
{

}

/* notice:
 * use timer to display,
 * if use thread, it will occur some error.
 */
int start_mainwindow_task(void)
{
	mainwindow_init();

	return 0;
}


