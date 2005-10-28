/*
 *  Dialog.cpp
 *
 *  $Id$
 *
 *  odbc-bench - a TPC-A and TPC-C like benchmark program for databases
 *  Copyright (C) 2000-2005 OpenLink Software <odbc-bench@openlinksw.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
