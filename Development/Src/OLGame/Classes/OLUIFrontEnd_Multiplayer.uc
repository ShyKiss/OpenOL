class OLUIFrontEnd_Multiplayer extends OLUIFrontEnd_Screen;

var transient GFxClikWidget ApplyButton;
var transient GFxClikWidget BackButton;
var transient GFxClikWidget CopyLinkButton;
var transient GFxClikWidget InviteSteamButton;
var transient GFxObject     SettingsList;

var localized string ApplyText;
var localized string CopyLinkText;
var localized string InviteSteamText;
var localized string InviteLinkLabelText;

var localized string UsernameText;
var localized string IPText;
var localized string PortText;
var localized string RoomCodeText;
var localized string PasswordText;
var localized string SyncInteractableText;
var localized string SyncEnemiesText;
var localized string SyncMatineesText;
var localized string SyncPickupsText;
var localized string SpeedrunModeText;
var localized string CreateServerText;
var localized string StopServerText;

// OST_ type constants matching OptionsList.as
const OST_CHECKBOX       = 0;
const OST_BUTTON         = 4; // OST_ControllerConfigButton — single button row
const OST_TEXTINPUT      = 6;

// Fake ProfileSettingIDs used to identify our custom buttons in Press_OptionItemButton
const BTN_ID_COPY_LINK      = 9001;
const BTN_ID_INVITE_STEAM   = 9002;
const BTN_ID_CREATE_SERVER  = 9003;

// Indices into SettingsOptions array
var int UsernameIdx;
var int IPIdx;
var int PortIdx;
var int RoomCodeIdx;
var int PasswordIdx;
var int SyncInteractableIdx;
var int SyncEnemiesIdx;
var int SyncMatineesIdx;
var int SyncPickupsIdx;
var int SpeedrunModeIdx;
var int InviteLinkIdx;
var int CreateServerIdx;

struct SettingEntry
{
    var string Label;
    var int    Type;
    var string StringValue;
    var int    IntValue;
    var bool   bReadOnly;
};
var array<SettingEntry> SettingsOptions;


function OnViewLoaded()
{
    Super.OnViewLoaded();
    SetFunction("Press_OptionItemButton", self, nameof(Press_OptionItemButton));
    SetFunction("OnTextInputChanged", self, nameof(OnTextInputChanged));
    BuildOptions();
}

// Called by Flash whenever any text input field changes.
function OnTextInputChanged(int idx, string NewValue)
{
    if (idx >= 0 && idx < SettingsOptions.Length)
        SettingsOptions[idx].StringValue = NewValue;
}

function Press_OptionItemButton(int PSID)
{
    local GFxClikWidget.EventData Dummy;

    switch (PSID)
    {
        //case BTN_ID_COPY_LINK:
        //    Press_CopyLink(Dummy);
        //    break;
        case BTN_ID_INVITE_STEAM:
            Press_InviteSteam(Dummy);
            break;
        case BTN_ID_CREATE_SERVER:
            Press_CreateServer(Dummy);
            break;
    }
}


function BuildOptions()
{
    local SettingEntry E;

    SettingsOptions.Length = 0;

    E.Label = UsernameText;  E.Type = OST_TEXTINPUT; E.StringValue = GetOLPC().GetNetUsername();   E.IntValue = 0; E.bReadOnly = false;
    UsernameIdx = SettingsOptions.Length;
    SettingsOptions.AddItem(E);

    E.Label = IPText;        E.Type = OST_TEXTINPUT; E.StringValue = GetOLPC().GetNetIP();         E.IntValue = 0; E.bReadOnly = false;
    IPIdx = SettingsOptions.Length;
    SettingsOptions.AddItem(E);

    E.Label = PortText;      E.Type = OST_TEXTINPUT; E.StringValue = GetOLPC().GetNetPort();       E.IntValue = 0; E.bReadOnly = false;
    PortIdx = SettingsOptions.Length;
    SettingsOptions.AddItem(E);

    E.Label = RoomCodeText;  E.Type = OST_TEXTINPUT; E.StringValue = GetOLPC().GetNetRoomCode();   E.IntValue = 0; E.bReadOnly = false;
    RoomCodeIdx = SettingsOptions.Length;
    SettingsOptions.AddItem(E);

    E.Label = PasswordText;  E.Type = OST_TEXTINPUT; E.StringValue = GetOLPC().GetNetPassword();   E.IntValue = 1; E.bReadOnly = false;
    PasswordIdx = SettingsOptions.Length;
    SettingsOptions.AddItem(E);

    E.Label = SyncInteractableText; E.Type = OST_CHECKBOX; E.StringValue = ""; E.IntValue = GetOLPC().GetNetSyncInteractable() ? 1 : 0; E.bReadOnly = false;
    SyncInteractableIdx = SettingsOptions.Length;
    SettingsOptions.AddItem(E);

    E.Label = SyncEnemiesText; E.Type = OST_CHECKBOX; E.StringValue = ""; E.IntValue = GetOLPC().GetNetSyncEnemies() ? 1 : 0; E.bReadOnly = false;
    SyncEnemiesIdx = SettingsOptions.Length;
    SettingsOptions.AddItem(E);

    E.Label = SyncMatineesText; E.Type = OST_CHECKBOX; E.StringValue = ""; E.IntValue = GetOLPC().GetNetSyncMatinees() ? 1 : 0; E.bReadOnly = false;
    SyncMatineesIdx = SettingsOptions.Length;
    SettingsOptions.AddItem(E);

    E.Label = SyncPickupsText; E.Type = OST_CHECKBOX; E.StringValue = ""; E.IntValue = GetOLPC().GetNetSyncPickups() ? 1 : 0; E.bReadOnly = false;
    SyncPickupsIdx = SettingsOptions.Length;
    SettingsOptions.AddItem(E);

    E.Label = SpeedrunModeText; E.Type = OST_CHECKBOX; E.StringValue = ""; E.IntValue = GetOLPC().GetNetSpeedrunMode() ? 1 : 0; E.bReadOnly = false;
    SpeedrunModeIdx = SettingsOptions.Length;
    SettingsOptions.AddItem(E);

    // TODO: re-enable invite link row and Copy Link button when openol:// deep-link is ready
    //E.Label = InviteLinkLabelText; E.Type = OST_TEXTINPUT; E.IntValue = 0; E.bReadOnly = true;
    //E.StringValue = GetOLPC().NativeBuildInviteLink(
    //    SettingsOptions[IPIdx].StringValue,
    //    SettingsOptions[PortIdx].StringValue,
    //    SettingsOptions[RoomCodeIdx].StringValue,
    //    SettingsOptions[PasswordIdx].StringValue);
    //InviteLinkIdx = SettingsOptions.Length;
    //SettingsOptions.AddItem(E);

    //E.bReadOnly = false;
    //E.Label = ""; E.Type = OST_BUTTON; E.StringValue = ""; E.IntValue = BTN_ID_COPY_LINK;
    //SettingsOptions.AddItem(E);

    E.Label = ""; E.Type = OST_BUTTON; E.StringValue = ""; E.IntValue = BTN_ID_INVITE_STEAM;
    SettingsOptions.AddItem(E);

    // Create/Stop Server button
    E.Label = ""; E.Type = OST_BUTTON; E.StringValue = ""; E.IntValue = BTN_ID_CREATE_SERVER;
    CreateServerIdx = SettingsOptions.Length;
    SettingsOptions.AddItem(E);
}

function PopulateList()
{
    local int i;
    local GFxObject DataProvider, Obj, LabelArr;
    local bool bRelayRunning;

    if (SettingsList == None)
        return;

    bRelayRunning = GetOLPC().NativeIsRelayRunning();

    DataProvider = CreateArray();
    for (i = 0; i < SettingsOptions.Length; i++)
    {
        Obj = CreateObject("Object");
        Obj.SetString("label",      SettingsOptions[i].Label);
        Obj.SetFloat ("OptionType", SettingsOptions[i].Type);
        if (SettingsOptions[i].Type == OST_TEXTINPUT)
        {
            Obj.SetFloat("ProfileSettingID", i);
            Obj.SetString("TextInputValue", SettingsOptions[i].StringValue);
            Obj.SetBool("IsPassword", SettingsOptions[i].IntValue != 0);
            Obj.SetBool("IsReadOnly", SettingsOptions[i].bReadOnly);
        }
        else if (SettingsOptions[i].Type == OST_BUTTON)
        {
            Obj.SetFloat("ProfileSettingID", SettingsOptions[i].IntValue);
            Obj.SetFloat("ButtonLabelIndex", 0);
            LabelArr = CreateArray();
            //if (SettingsOptions[i].IntValue == BTN_ID_COPY_LINK)
            //    LabelArr.SetElementString(0, CopyLinkText);
            //else
            if (SettingsOptions[i].IntValue == BTN_ID_INVITE_STEAM)
                LabelArr.SetElementString(0, InviteSteamText);
            else if (SettingsOptions[i].IntValue == BTN_ID_CREATE_SERVER)
                LabelArr.SetElementString(0, bRelayRunning ? StopServerText : CreateServerText);
            Obj.SetObject("ButtonLabelsList", LabelArr);
        }
        else
            Obj.SetBool("CheckboxSelected", SettingsOptions[i].IntValue != 0);
        DataProvider.SetElementObject(i, Obj);
    }

    SettingsList.SetObject("dataProvider", DataProvider);
    SettingsList.SetFloat ("selectedIndex", 0);
}

function StoreListValues()
{
    local int i;
    local array<ASValue> Args;
    local ASValue RetVal;

    if (SettingsList == None)
        return;

    Args.Length = 1;
    Args[0].Type = AS_Number;

    for (i = 0; i < SettingsOptions.Length; i++)
    {
        if (SettingsOptions[i].bReadOnly)
            continue;
        Args[0].n = i;
        if (SettingsOptions[i].Type == OST_TEXTINPUT)
        {
            RetVal = SettingsList.Invoke("GetSelectionValueStringAt", Args);
            SettingsOptions[i].StringValue = RetVal.s;
        }
        else if (SettingsOptions[i].Type != OST_BUTTON)
        {
            RetVal = SettingsList.Invoke("GetSelectionValueAt", Args);
            SettingsOptions[i].IntValue = int(RetVal.n);
        }
    }

    // TODO: re-enable when invite link row is restored
    //Link = GetOLPC().NativeBuildInviteLink(
    //    SettingsOptions[IPIdx].StringValue,
    //    SettingsOptions[PortIdx].StringValue,
    //    SettingsOptions[RoomCodeIdx].StringValue,
    //    SettingsOptions[PasswordIdx].StringValue);
    //SettingsOptions[InviteLinkIdx].StringValue = Link;
    //if (SettingsList != None)
    //{
    //    Args.Length = 2;
    //    Args[0].Type = AS_Number; Args[0].n = InviteLinkIdx;
    //    Args[1].Type = AS_String; Args[1].s = Link;
    //    SettingsList.Invoke("SetTextInputValueAt", Args);
    //}
}

function SaveSettings()
{
    StoreListValues();

    GetOLPC().SaveNetworkSettings(
        SettingsOptions[IPIdx].StringValue,
        SettingsOptions[PortIdx].StringValue,
        SettingsOptions[UsernameIdx].StringValue,
        SettingsOptions[SyncInteractableIdx].IntValue != 0,
        SettingsOptions[SyncEnemiesIdx].IntValue != 0,
        SettingsOptions[SyncMatineesIdx].IntValue != 0,
        SettingsOptions[SyncPickupsIdx].IntValue != 0,
        SettingsOptions[SpeedrunModeIdx].IntValue != 0,
        SettingsOptions[RoomCodeIdx].StringValue,
        SettingsOptions[PasswordIdx].StringValue);
}

function string BuildInviteLink()
{
    local string IP, Port, Room, Pass;

    StoreListValues();
    IP   = SettingsOptions[IPIdx].StringValue;
    Port = SettingsOptions[PortIdx].StringValue;
    Room = SettingsOptions[RoomCodeIdx].StringValue;
    Pass = SettingsOptions[PasswordIdx].StringValue;

    return GetOLPC().NativeBuildInviteLink(IP, Port, Room, Pass);
}

function bool ParseInviteLink(string Link)
{
    local string OutIP, OutPort, OutRoom, OutPass;

    if (!GetOLPC().NativeParseInviteLink(Link, OutIP, OutPort, OutRoom, OutPass))
        return false;

    if (Len(OutIP)   > 0) SettingsOptions[IPIdx].StringValue       = OutIP;
    if (Len(OutPort) > 0) SettingsOptions[PortIdx].StringValue      = OutPort;
    if (Len(OutRoom) > 0) SettingsOptions[RoomCodeIdx].StringValue  = OutRoom;
    SettingsOptions[PasswordIdx].StringValue = OutPass;
    return true;
}

function DoConnect()
{
    SaveSettings();
    ConsoleCommand("open DLC_Intro_Persistent?game=Multiplayer.MultiplayerGame");
}

function Press_Apply(GFxClikWidget.EventData ev)
{
    DoConnect();
}

function CopyWidgetTextByName(string WidgetLabel)
{
    local int i;
    for (i = 0; i < SettingsOptions.Length; i++)
    {
        if (SettingsOptions[i].Label == WidgetLabel)
        {
            GetOLPC().CopyToClipboard(SettingsOptions[i].StringValue);
            return;
        }
    }
}

function Press_CopyLink(GFxClikWidget.EventData ev)
{
    CopyWidgetTextByName(InviteLinkLabelText);
}

function Press_InviteSteam(GFxClikWidget.EventData ev)
{
    GetOLPC().NativeOpenSteamFriendsOverlay();
}

function Press_CreateServer(GFxClikWidget.EventData ev)
{
    local int Port;

    if (GetOLPC().NativeIsRelayRunning())
    {
        // Stop relay, then refresh list so button shows "Create Server".
        GetOLPC().NativeStopRelay();
        PopulateList();
    }
    else
    {
        // Save settings first so Port field is up to date.
        StoreListValues();
        Port = int(SettingsOptions[PortIdx].StringValue);
        if (Port <= 0) Port = 7777;

        // Start relay and connect to it locally (no P2P — we are the host).
        GetOLPC().NativeStartRelay(Port);
        GetOLPC().NativeConnectLocal(Port);

        // Open the map.
        SaveSettings();
        ConsoleCommand("open DLC_Intro_Persistent?game=Multiplayer.MultiplayerGame");
    }
}

function Press_Back(GFxClikWidget.EventData ev)
{
    MenuManager.PopView();
}

function bool Back()
{
    MenuManager.PopView();
    return true;
}

event bool WidgetInitialized(name WidgetName, name WidgetPath, GFxObject Widget)
{
    local bool bWasHandled;
    bWasHandled = false;

    switch (WidgetName)
    {
        case ('applyBtn'):
            ApplyButton = GFxClikWidget(Widget);
            ApplyButton.AddEventListener('CLIK_press', Press_Apply);
            ApplyButton.SetString("label", ApplyText);
            bWasHandled = true;
            break;
        case ('backBtn'):
            BackButton = GFxClikWidget(Widget);
            BackButton.AddEventListener('CLIK_press', Press_Back);
            BackButton.SetString("label", BackText);
            bWasHandled = true;
            break;
        //case ('copyLinkBtn'):
        //    CopyLinkButton = GFxClikWidget(Widget);
        //    CopyLinkButton.AddEventListener('CLIK_press', Press_CopyLink);
        //    CopyLinkButton.SetString("label", CopyLinkText);
        //    bWasHandled = true;
        //    break;
        case ('inviteSteamBtn'):
            InviteSteamButton = GFxClikWidget(Widget);
            InviteSteamButton.AddEventListener('CLIK_press', Press_InviteSteam);
            InviteSteamButton.SetString("label", InviteSteamText);
            bWasHandled = true;
            break;
        case ('gameplayList'):
            SettingsList = Widget;
            PopulateList();
            bWasHandled = true;
            break;
        default:
            bWasHandled = false;
    }

    if (!bWasHandled)
        bWasHandled = Super.WidgetInitialized(WidgetName, WidgetPath, Widget);

    return bWasHandled;
}

defaultproperties
{
    SubWidgetBindings.Add((WidgetName="applyBtn",WidgetClass=class'GFxClikWidget'))
    SubWidgetBindings.Add((WidgetName="backBtn",WidgetClass=class'GFxClikWidget'))
    //SubWidgetBindings.Add((WidgetName="copyLinkBtn",WidgetClass=class'GFxClikWidget'))
    SubWidgetBindings.Add((WidgetName="inviteSteamBtn",WidgetClass=class'GFxClikWidget'))
}
