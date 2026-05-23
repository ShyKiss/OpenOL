class OLUIFrontEnd_LoadGame extends OLUIFrontEnd_Screen;

var localized string TitleText;
var localized string LoadText;
var localized string DeleteText;

var int SelectedIndex;

var transient GFxClikWidget SaveList;

var transient GFxClikWidget BackButton;
var transient GFxClikWidget LoadButton;
var transient GFxClikWidget DeleteButton;

var config bool bShowCheckpointNames;

function OnTopMostView(optional bool bPlayOpenAnimation /* = false */)
{
	PopulateSaveList();
}

function Press_Back(GFxClikWidget.EventData ev)
{
	Back();
}

function Press_Load(GFxClikWidget.EventData ev)
{
	local OLEngine Engine;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (Engine != None && SelectedIndex >= 0 && SelectedIndex < Engine.SaveFiles.length)
	{
		Engine.LoadSaveFile(Engine.SaveFiles[SelectedIndex].FileName);

		GetOLPC().StopAllSounds();

		Close(true);
	}
}

function Press_Delete(GFxClikWidget.EventData ev)
{
	local OLEngine Engine;
	local int PreviousSelectedIndex;
	local int PreviousScrollPosition;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (Engine != None && SelectedIndex >= 0 && SelectedIndex < Engine.SaveFiles.length)
	{
		PreviousSelectedIndex = SelectedIndex;
		PreviousScrollPosition = SaveList.GetFloat("scrollPosition");

		Engine.DeleteSaveFile(Engine.SaveFiles[SelectedIndex].FileName);
		PopulateSaveList();

		if (PreviousSelectedIndex >= Engine.SaveFiles.Length && Engine.SaveFiles.Length > 0) {
			PreviousSelectedIndex = Engine.SaveFiles.Length-1;
		}
		SaveList.SetFloat("selectedIndex", PreviousSelectedIndex);
		SaveList.SetFloat("scrollPosition", PreviousScrollPosition);
		SetSelectedIndex(PreviousSelectedIndex);
	}
}

function PopulateSaveList()
{
	local OLEngine Engine;
	local int i;
	local GFxObject Obj, DataProvider;

	Engine = OLEngine(class'Engine'.static.GetEngine());
	if (Engine != None)
	{
		Engine.RefreshSaveFiles();

		DataProvider = CreateArray();
		
		for (i = 0; i < Engine.SaveFiles.length; ++i)
		{
			Obj = CreateObject("Object");
			Obj.SetString("label", GetSaveFileDisplayName(Engine.SaveFiles[i]));
			DataProvider.SetElementObject(i, Obj);
		}
	}

	SaveList.SetObject("dataProvider", DataProvider);
	SaveList.SetFloat("selectedIndex", 0);

	SetSelectedIndex(0);
}

function SaveListChanged(GFxClikWidget.EventData ev)
{
	local GFxObject Group;
	local GFxObject Button;
	local int Index;

	Group = SaveList.GetObject("group");

	if (Group == None)
	{
		return;
	}

	Button = Group.GetObject("selectedButton");

	if (Button == None)
	{
		return;
	}

	Index = Button.GetInt("index");

	if (SelectedIndex != Index)
	{
		SetSelectedIndex(Index);	
	}
}

function SetSelectedIndex(int Index)
{
	SelectedIndex = Index;
}

function string GetSaveFileDisplayName(SaveFileInfo SaveFile)
{
	local CheckpointTime SaveFileTime;
	local string DateString, TimeString;
	local int Minutes;
	local name CheckpointTag;

	CheckpointTag = GetCheckpointTag(SaveFile.CheckpointName);

	SaveFileTime = SaveFile.Time;
	DateString = SaveFileTime.Day $ "/" $ SaveFileTime.Month $ "/" $ SaveFileTime.Year;
	Minutes = FFloor(SaveFileTime.SecondsSinceMidnight/60) % 60;
	TimeString = FFloor(SaveFileTime.SecondsSinceMidnight/3600) $ ":" $ (Minutes < 10 ? "0" : "") $ Minutes;

	if (bShowCheckpointNames)
	{
		return SaveFile.CheckpointName $ " - " $ DateString $ " - " $ TimeString;
	}
	else if (CheckpointTag != '')
	{
		return Localize("Locations", string(CheckpointTag), "OLGame") $ " - " $ DateString $ " - " $ TimeString;
	}
	else
	{
		return DateString $ " - " $ TimeString;
	}
}

function name GetCheckpointTag(name CheckpointName)
{
	local name Tag;
	local OLCheckpoint CP;

	foreach GetOLPC().AllActors(class'OLCheckpoint', CP)
	{
		if (Caps(CP.CheckpointName) == Caps(CheckpointName))
		{
			Tag = CP.Tag;
			break;
		}
	}
	return Tag;
}

/** Callback when a CLIK widget with enableInitCallback set to TRUE is initialized.  Returns TRUE if the widget was handled, FALSE if not. */
event bool WidgetInitialized(name WidgetName, name WidgetPath, GFxObject Widget)
{
	local bool bWasHandled;
	bWasHandled = TRUE;

	switch(WidgetName)
	{
		case ('loadTitle'):
			Widget.SetString("text", TitleText);
			break;
		case ('backBtn'):
			BackButton = GFxClikWidget(Widget);
			BackButton.AddEventListener('CLIK_press', Press_Back);
			BackButton.SetString("label", BackText);
			break;
		case ('loadBtn'):
			LoadButton = GFxClikWidget(Widget);
			LoadButton.AddEventListener('CLIK_press', Press_Load);
			LoadButton.SetString("label", LoadText);
			break;
		case ('deleteBtn'):
			DeleteButton = GFxClikWidget(Widget);
			DeleteButton.AddEventListener('CLIK_press', Press_Delete);
			DeleteButton.SetString("label", DeleteText);
			break;
		case ('saveList'):
			SaveList = GFxClikWidget(Widget);
			PopulateSaveList();
			SaveList.AddEventListener('CLIK_change', SaveListChanged);
			SaveList.AddEventListener('CLIK_itemDoubleClick', Press_Load);
			break;
		default:
			bWasHandled = FALSE;
	}

	if (!bWasHandled)
	{
		bWasHandled = Super.WidgetInitialized(WidgetName, WidgetPath, Widget);  
	}
	return bWasHandled;
}

defaultproperties
{
	SubWidgetBindings.Add((WidgetName="backBtn",WidgetClass=class'GFxClikWidget'))
	SubWidgetBindings.Add((WidgetName="loadBtn",WidgetClass=class'GFxClikWidget'))
	SubWidgetBindings.Add((WidgetName="deleteBtn",WidgetClass=class'GFxClikWidget'))
	SubWidgetBindings.Add((WidgetName="saveList",WidgetClass=class'GFxClikWidget'))
}