/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#include "GUI/Control.h"
#include "GUI/Window.h"

#include "win32def.h"

#include "ControlAnimation.h"
#include "Interface.h"
#include "Sprite2D.h"

#ifdef ANDROID
#include "Variables.h"
#endif

#include <cstdio>
#include <cstring>

namespace GemRB {

unsigned int Control::ActionRepeatDelay = 250;

Control::Control(const Region& frame, Window* win)
: View(frame)
{
	InHandler = false;
	VarName[0] = 0;
	Value = 0;

	animation = NULL;
	AnimPicture = NULL;
	ControlType = IE_GUI_INVALID;

	actionTimer = NULL;
	repeatDelay = 0;
    
    SetWindow(win);
}

Control::~Control()
{
	if (actionTimer)
		actionTimer->Invalidate();

	if (InHandler) {
		Log(ERROR, "Control", "Destroying control inside event handler, crash may occur!");
	}
	delete animation;

	Sprite2D::FreeSprite(AnimPicture);
}
    
void Control::SetWindow(Window* win)
{
    if (win == window) {
        return;
    }
    if (window) {
        window->RemoveSubview(this);
		window = NULL;
    }
	
	static Window* recursiveWin = NULL;
    if (win && win != recursiveWin) {
		// avoid infinite recursion
		recursiveWin = win;
        win->AddSubviewInFrontOfView(this);
		recursiveWin = NULL;
    }
	window = win;
}

void Control::SetText(const String* string)
{
	SetText((string) ? *string : L"");
}

bool Control::SetEvent(int eventType, ControlEventHandler handler)
{
	SetAction(handler, eventType);
	return true;
}

void Control::SetAction(ControlEventHandler handler, unsigned int flags)
{
	actions[flags] = handler;
}

void Control::SetActionInterval(unsigned int interval)
{
	repeatDelay = interval;
	if (actionTimer) {
		actionTimer->SetInverval(repeatDelay);
	}
}

void Control::ResetEventHandler(ControlEventHandler &handler)
{
	handler = NULL;
}

//return -1 if there is an error
//return 1 if there is no handler (not an error)
//return 0 if the handler ran as intended
int Control::RunEventHandler(const ControlEventHandler& handler)
{
	if (InHandler) {
		Log(WARNING, "Control", "Nested event handlers are not supported!");
		return -1;
	}
	if (handler) {
		if (!window) {
			return -1;
		}
		InHandler = true;
		//TODO: detect caller errors, trap them???
		handler(this);
		InHandler = false;
		return 0;
	}
	return 1;
}

void Control::UpdateState(const char* varname, unsigned int val)
{
	if (strnicmp(VarName, varname, MAX_VARIABLE_LENGTH-1) == 0) {
		UpdateState(val);
	}
}

void Control::SetFocus()
{
	window->SetFocused(this);
	MarkDirty();
}

bool Control::IsFocused()
{
	return window->FocusedView() == this;
}

void Control::SetAnimPicture(Sprite2D* newpic)
{
	Sprite2D::FreeSprite(AnimPicture);
	AnimPicture = newpic;
	MarkDirty();
}

Timer* Control::StartActionTimer(const ControlEventHandler& action)
{
	class RepeatControlEventHandler : public Callback<void, void> {
		const ControlEventHandler action;
		Control* ctrl;

	public:
		RepeatControlEventHandler(const ControlEventHandler& handler, Control* c)
		: action(handler), ctrl(c) {}

		void operator()() const {
			// update the timer to use the actual repeatDelay
			ctrl->SetActionInterval(ctrl->repeatDelay);
			return action(ctrl);
		}
	};

	EventHandler h = new RepeatControlEventHandler(action, this);
	// always start the timer with ActionRepeatDelay
	// this way we have consistent behavior for the initial delay prior to switching to a faster delay
	return &core->SetTimer(h, ActionRepeatDelay);
}

void Control::OnMouseUp(const MouseEvent& me, unsigned short /*mod*/)
{
	if (actions.count(me.button)) {
		RunEventHandler(actions[me.button]);
		if (actionTimer) {
			actionTimer->Invalidate();
			actionTimer = NULL;
		}
	}
}

void Control::OnMouseDown(const MouseEvent& me, unsigned short /*Mod*/)
{
	if (repeatDelay && actions.count(me.button)) {
		actionTimer = StartActionTimer(actions[me.button]);
	}
}

}
