#character generation, import  (GUICG24)
import GemRB

#import from a game 
ImportWindow = 0

def OnLoad():
	global ImportWindow

	GemRB.LoadWindowPack("GUICG")
        ImportWindow = GemRB.LoadWindow(20)

	TextAreaControl = GemRB.GetControl(ImportWindow, 4)
	GemRB.SetText(ImportWindow, TextAreaControl, 53774)

	TextAreaControl = GemRB.GetControl(ImportWindow,2)
#Fill TextArea Control with character sheets, make textarea a listbox

	DoneButton = GemRB.GetControl(ImportWindow, 0)
	GemRB.SetText(ImportWindow, DoneButton, 2610)
	GemRB.SetButtonState(ImportWindow, DoneButton, IE_GUI_BUTTON_DISABLED)

	CancelButton = GemRB.GetControl(ImportWindow,1)
	GemRB.SetText(ImportWindow, CancelButton, 15416)

        GemRB.SetEvent(ImportWindow, DoneButton, IE_GUI_BUTTON_ON_PRESS, "Done1Press")
        GemRB.SetEvent(ImportWindow, CancelButton, IE_GUI_BUTTON_ON_PRESS, "CancelPress")
	GemRB.ShowModal(ImportWindow)
	return

def Done1Press():
	DoneButton = GemRB.GetControl(ImportWindow, 0)
	GemRB.SetText(ImportWindow, DoneButton, 11973)
	GemRB.SetButtonState(ImportWindow, DoneButton, IE_GUI_BUTTON_DISABLED)
        GemRB.SetEvent(ImportWindow, DoneButton, IE_GUI_BUTTON_ON_PRESS, "Done2Press")
        return
	
def Done2Press():
        GemRB.UnloadWindow(ImportWindow)
        GemRB.SetNextScript("Start")
        return

def CancelPress():
        GemRB.UnloadWindow(ImportWindow)
        GemRB.SetNextScript("GUICG24")
        return
