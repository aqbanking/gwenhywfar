


#ifndef GWENHYWFAR_GUI_CPROGRESS_H
#define GWENHYWFAR_GUI_CPROGRESS_H

#include <gwenhywfar/list.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/logger.h>



typedef struct GWEN_GUI_CPROGRESS GWEN_GUI_CPROGRESS;
GWEN_LIST_FUNCTION_DEFS(GWEN_GUI_CPROGRESS, GWEN_Gui_CProgress)


GWEN_GUI_CPROGRESS *GWEN_Gui_CProgress_new(GWEN_GUI *gui,
					   uint32_t id,
					   uint32_t progressFlags,
					   const char *title,
					   const char *text,
					   uint64_t total);
void GWEN_Gui_CProgress_free(GWEN_GUI_CPROGRESS *cp);


GWEN_GUI *GWEN_Gui_CProgress_GetGui(const GWEN_GUI_CPROGRESS *cp);
uint32_t GWEN_Gui_CProgress_GetId(const GWEN_GUI_CPROGRESS *cp);
const char *GWEN_Gui_CProgress_GetTitle(const GWEN_GUI_CPROGRESS *cp);
const char *GWEN_Gui_CProgress_GetText(const GWEN_GUI_CPROGRESS *cp);
uint64_t GWEN_Gui_CProgress_GetTotal(const GWEN_GUI_CPROGRESS *cp);
void GWEN_Gui_CProgress_SetTotal(GWEN_GUI_CPROGRESS *cp, uint64_t i);

uint64_t GWEN_Gui_CProgress_GetCurrent(const GWEN_GUI_CPROGRESS *cp);
const char *GWEN_Gui_CProgress_GetLogBuf(const GWEN_GUI_CPROGRESS *cp);
int GWEN_Gui_CProgress_GetAborted(const GWEN_GUI_CPROGRESS *cp);


int GWEN_Gui_CProgress_Advance(GWEN_GUI_CPROGRESS *cp, uint64_t progress);

int GWEN_Gui_CProgress_Log(GWEN_GUI_CPROGRESS *cp,
			   GWEN_LOGGER_LEVEL level,
			   const char *text);

int GWEN_Gui_CProgress_End(GWEN_GUI_CPROGRESS *cp);



#endif

