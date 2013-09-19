/* Form definition file generated with fdesign. */

#include <forms.h>
#include <stdlib.h>
#include "testgui.h"

FD_testgui *create_form_testgui(void)
{
  FL_OBJECT *obj;
  FD_testgui *fdui = (FD_testgui *) fl_calloc(1, sizeof(*fdui));

  fdui->testgui = fl_bgn_form(FL_NO_BOX, 670, 460);
  fdui->mainWindow = obj = fl_add_box(FL_UP_BOX,0,0,670,460,"");
    fl_set_object_color(obj,FL_DODGERBLUE,FL_MAGENTA);
    fl_set_object_callback(obj,mainWindow,0);
  fdui->connectTo = obj = fl_add_input(FL_NORMAL_INPUT,120,20,230,30,"Remote Node");
    fl_set_object_color(obj,FL_WHITE,FL_WHITE);
    fl_set_object_lcolor(obj,FL_SPRINGGREEN);
    fl_set_object_callback(obj,getConnection,0);
	//prevent resizing of main window
	fl_winsize(670, 460);
  fdui->infoData = obj = fl_add_text(FL_NORMAL_TEXT,360,10,300,290,"");
    fl_set_object_color(obj,FL_BLACK,FL_WHITE);
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->ConnectedNode = obj = fl_add_text(FL_NORMAL_TEXT,120,60,230,30,"");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->disconnectButton = obj = fl_add_button(FL_NORMAL_BUTTON,120,410,90,40,"Disconnect");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sayGoodbye,0);
  fdui->PT = obj = fl_add_button(FL_NORMAL_BUTTON,520,410,130,30,"PTT");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,PTT,0);
  fdui->typedData = obj = fl_add_input(FL_NORMAL_INPUT,20,120,330,30,"");
    fl_set_object_color(obj,FL_WHITE,FL_WHITE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,typedData,0);
  fdui->displayData = obj = fl_add_browser(FL_NORMAL_BROWSER,360,300,300,80,"");
    fl_set_object_color(obj,FL_CYAN,FL_YELLOW);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,displayData,0);
  obj = fl_add_text(FL_NORMAL_TEXT,10,70,110,20,"Connected To");
    fl_set_object_color(obj,FL_DODGERBLUE,FL_WHITE);
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,100,120,20,"Type Data Here");
    fl_set_object_color(obj,FL_DODGERBLUE,FL_MCOL);
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->txIndicator = obj = fl_add_text(FL_NORMAL_TEXT,470,410,40,30,"TX");
    fl_set_object_color(obj,FL_RED,FL_CHARTREUSE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->nodeList = obj = fl_add_browser(FL_HOLD_BROWSER,20,175,330,200,"");
    fl_set_object_color(obj,FL_WHITE,FL_GREEN);
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,nodeList,0);
  fdui->timer1 = obj = fl_add_timer(FL_HIDDEN_TIMER,120,430,20,20,"");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,timer1Call,0);
  fdui->update = obj = fl_add_button(FL_NORMAL_BUTTON,20,410,90,40,"Update");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,getList,0);
  fdui->strength = obj = fl_add_slider(FL_HOR_FILL_SLIDER,320,395,140,20,"");
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_color(obj,FL_GREEN,FL_BLUE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_slider_bounds(obj, 0, 100);
    fl_set_slider_value(obj, 0);
    fl_set_slider_size(obj, 0.15);
  fdui->serverInfo = obj = fl_add_text(FL_NORMAL_TEXT,20,155,330,20,"");
    fl_set_object_color(obj,FL_WHITE,FL_MCOL);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->voxThreshold = obj = fl_add_slider(FL_HOR_FILL_SLIDER,320,425,140,20,"");
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_color(obj,FL_GREEN,FL_BLUE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,voxThreshold,0);
    fl_set_slider_bounds(obj, 0, 100);
    fl_set_slider_value(obj, 0);
    fl_set_slider_size(obj, 0.15);
  fdui->voxButton = obj = fl_add_lightbutton(FL_PUSH_BUTTON,240,425,70,20,"VOX");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,voxToggle,0);
  fdui->listTimer = obj = fl_add_timer(FL_HIDDEN_TIMER,530,390,10,10,"");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_callback(obj,listTimerCB,0);
  fl_end_form();

  fdui->testgui->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_searchForm *create_form_searchForm(void)
{
  FL_OBJECT *obj;
  FD_searchForm *fdui = (FD_searchForm *) fl_calloc(1, sizeof(*fdui));

  fdui->searchForm = fl_bgn_form(FL_NO_BOX, 330, 110);
  obj = fl_add_box(FL_UP_BOX,0,0,330,110,"");
  fdui->findButton = obj = fl_add_button(FL_NORMAL_BUTTON,40,60,120,30,"Find");
    fl_set_object_callback(obj,searchCall,1);
  fdui->CancelButton = obj = fl_add_button(FL_NORMAL_BUTTON,180,60,120,30,"Cancel");
    fl_set_object_callback(obj,searchCall,2);
  fdui->tagetString = obj = fl_add_input(FL_NORMAL_INPUT,20,20,300,30,"");
    fl_set_object_callback(obj,searchCall,1);
  fl_end_form();

  fdui->searchForm->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

