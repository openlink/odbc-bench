/*
 *  Dialog.cpp
 *  odbc-bench
 *
 *  Created by Farmer Joe on 10/11/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "odbcbench_macosx.h"
#include "Dialog.h"

OPL_Dialog::OPL_Dialog(CFStringRef resname):
	m_window(0), m_status(true)
{
	OSStatus err;
	static EventTypeSpec dialog_events[] = {
		{ kEventClassCommand, kEventCommandProcess }
	};

	// create and show preferences dialog
	err = CreateWindowFromNib(g_main_nib, resname, &m_window);
	require_noerr(err, error);

	static EventHandlerUPP g_eventHandlerUPP = NULL;
	if (g_eventHandlerUPP == NULL)
		g_eventHandlerUPP = NewEventHandlerUPP(eventHandler);

	// install control event handler
	err = InstallWindowEventHandler(getWindow(), g_eventHandlerUPP,
		GetEventTypeCount(dialog_events), dialog_events, this, NULL);
	require_noerr(err, error);

error:
	/* do nothing */;
}

pascal OSStatus
OPL_Dialog::eventHandler(EventHandlerCallRef handlerRef, EventRef eventRef, void *userData)
{
	OSStatus err = eventNotHandledErr;
	UInt32 eventClass = GetEventClass(eventRef);
	UInt32 eventKind = GetEventKind(eventRef);
	HICommand cmd;
	OPL_Dialog *self = (OPL_Dialog *) userData;

	// Sanity checks
	if (!self->getWindow())
		return err;
	if (eventClass != kEventClassCommand && eventKind != kEventCommandProcess)
		return err;
	
	// Obtain HICommand
	err = GetEventParameter(eventRef, kEventParamDirectObject,  
		typeHICommand, NULL, sizeof(cmd), NULL, &cmd);
	require_noerr(err, error);

	// process command
	err = noErr;
	switch (cmd.commandID) {
	case kHICommandOK:
		self->m_status = true;
		break;
		
	case kHICommandCancel:
		self->m_status = false;
		break;
		
	default:
		return self->handleCommandEvent(cmd.commandID);
	}

	// we're done
	self->endModal();
	
error:
	return err;
}
