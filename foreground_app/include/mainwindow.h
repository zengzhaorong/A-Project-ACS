#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QTextCodec>
#include <QComboBox>
#include <QMessageBox>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QPainter>
#include <QRect>

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "public.h"
#ifdef __cplusplus
}
#endif

/* widget locate pixel */
#define Y_INTERV_PIXEL_IN		3
#define Y_INTERV_PIXEL_EX		8
#define WIDGET_HEIGHT_PIXEL		30
#define FUNC_AREA_PIXEL_X		CONFIG_CAPTURE_WIDTH(main_mngr.config_ini)

#define TIMER_INTERV_MS			1
#define TIMER_ADDUSER_OK_MS		(3*1000)
#define FACE_RECT_LASTING_MS	30	// face rectangle lasting time(ms)

#define WIN_BACKGRD_IMG			"resource/background.jpg"		// 界面背景图
#define EXTRAINFO_MNGR_IMG		"resource/extraInfo_mngr.png"
#define EXTRAINFO_USER_IMG		"resource/extraInfo_user.png"

/* tips text, support Chinese */
#define NOT_CONNECT_SERVER		"未连接服务器"
#define BEGIN_ADD_FACE_TEXT		"录入人脸：请正对摄像头"
#define SUCCESS_ADD_FACE_TEXT	"录入人脸成功"
#define TEXT_RECOGN_SUCCESS		"识别成功"			// "识别成功"、"签到成功"、"欢迎回家"
#define TEXT_ADD_USER			"添加用户"
#define TEXT_DEL_USER			"删除用户"
#define TEXT_HIST_REC			"历史记录"
#define TEXT_SAVE				"保存"
#define TEXT_RESET				"重置"
#define TEXT_TAKE_PHOTO			"拍照"
#define TEXT_SWIT_CAPTURE		"切换画面"


typedef enum {
	MAINWIN_MODE_DISCONECT,
	MAINWIN_MODE_NORAML,
	MAINWIN_MODE_ADDUSER,
	MAINWIN_MODE_ADDUSER_OK,
	MAINWIN_MODE_RECOGN,
}mainwin_mode_e;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void showMainwindow(void);
	void addUser(void);
	void deleteUser(void);
	void showHistRecord(void);
	void saveRecord(void);
	void resetRecord(void);
	void takePhoto(void);
	void switchCapture(void);
	void textOnVideo_show_over(void);
	
public:
	void show_func_area(void);
	void drawFaceRectangle(QImage &img);
	int switch_mainwin_mode(mainwin_mode_e mode);

private:
	QWidget			*mainWindow;		// main window
	QLabel 			*videoArea;			// video area
	QImage			backgroundImg;		// background image
	QTimer 			*timer;				// display timer
	QLabel			*clockLabel;		// display clock
	QLabel 			*extraInfo;			// show as image
	QLineEdit		*userIdEdit;		// edit add user id
	QLineEdit		*userNameEdit;		// edit add user name
	QPushButton 	*addUserBtn;		// add user button
	QPushButton 	*delUserBtn;		// delete user button
	QPushButton 	*showHistRecordBtn;	// show history record button
	QPushButton 	*saveRecordBtn;		// save record button
	QPushButton 	*resetRecordBtn;	// delete record button
	QPushButton 	*takePhotoBtn;		// take photo button
	QPushButton 	*switchCaptureBtn;	// switch capture button
	unsigned char 	*video_buf;
	unsigned int 	buf_size;
	
public:
	QComboBox		*userListBox;		// user list box
	QLabel 			*textOnVideo;		// text show on video
	QTimer 			*tmpShowTimer;		// control temple show, few second
	QTableView		*tableView;
	QStandardItemModel *listModel;
	QRect 			face_rects;			// face rectangles
	int 			face_id;
	char 			userRecogn[USER_NAME_LEN];
	uint8_t			confidence;
	int 			face_status;
	workstate_e		*sys_state;			// is system work state (main_mngr.work_state)
	int 			mainwin_mode;
	int 			stateTick;			// 
	int				takePhotoFlag;
};


int mainwin_set_userList(int flag, int userCnt, char *usr_name);
int mainwin_set_recordList(uint32_t time, int id, char *usr_name, int confid);
void mainwin_clear_recordList(void);

int mainwin_set_rects(int x, int y, int w, int h);
int mainwin_set_recognInfo(int id, uint8_t confid, char *usr_name, int status);

int start_mainwindow_task(void);


#endif	// _MAINWINDOW_H_