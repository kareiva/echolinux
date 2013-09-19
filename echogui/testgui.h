/** Header file generated with fdesign on Wed Dec 24 12:31:32 2003.**/

#ifndef FD_testgui_h_
#define FD_testgui_h_

/** Callbacks, globals and object handlers **/
extern void mainWindow(FL_OBJECT *, long);
extern void getConnection(FL_OBJECT *, long);
extern void sayGoodbye(FL_OBJECT *, long);
extern void PTT(FL_OBJECT *, long);
extern void typedData(FL_OBJECT *, long);
extern void displayData(FL_OBJECT *, long);
extern void nodeList(FL_OBJECT *, long);
extern void timer1Call(FL_OBJECT *, long);
extern void getList(FL_OBJECT *, long);
extern void voxThreshold(FL_OBJECT *, long);
extern void voxToggle(FL_OBJECT *, long);
extern void listTimerCB(FL_OBJECT *, long);

extern void searchCall(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *testgui;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *mainWindow;
	FL_OBJECT *connectTo;
	FL_OBJECT *infoData;
	FL_OBJECT *ConnectedNode;
	FL_OBJECT *disconnectButton;
	FL_OBJECT *PT;
	FL_OBJECT *typedData;
	FL_OBJECT *displayData;
	FL_OBJECT *txIndicator;
	FL_OBJECT *nodeList;
	FL_OBJECT *timer1;
	FL_OBJECT *update;
	FL_OBJECT *strength;
	FL_OBJECT *serverInfo;
	FL_OBJECT *voxThreshold;
	FL_OBJECT *voxButton;
	FL_OBJECT *listTimer;
} FD_testgui;

extern FD_testgui * create_form_testgui(void);
typedef struct {
	FL_FORM *searchForm;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *findButton;
	FL_OBJECT *CancelButton;
	FL_OBJECT *tagetString;
} FD_searchForm;

extern FD_searchForm * create_form_searchForm(void);

#endif /* FD_testgui_h_ */
