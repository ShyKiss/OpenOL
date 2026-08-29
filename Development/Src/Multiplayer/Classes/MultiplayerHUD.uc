class MultiplayerHUD extends OLHUD
    config(Multiplayer)
    native;

const NOTIF_DURATION = 5.0;
const NOTIF_FADE_IN  = 0.25;
const NOTIF_FADE_OUT = 0.6;

// Max remote player nicks shown in the HUD nick list (0 = unlimited).
var int MaxNickDisplay;

struct NotificationEntry
{
    var string Text;
    var float  ExpireTime;
};
var array<NotificationEntry> Notifications;

native function AddNotification(string Msg);

Event OnLostFocusPause(Bool bEnable) {
    return;
}

exec function ShowMenu()
{
    ShowMenuType(EMT_PauseMenu);
}

event ShowMenuType(EMenuType MenuType)
{
    local OLGame TheGame;

    if (MenuType != EMT_MainMenu && !CanShowSubMenu())
        return;

    TheGame = OLGame(WorldInfo.Game);
    if (MenuManager == None || !MenuManager.bMovieIsOpen)
    {
        if (PlayerOwner != None)
            PlayerOwner.PlayerInput.ResetInput();

        if (MenuType != EMT_MainMenu)
        {
            if (MenuType == EMT_Credits)
                TheGame.bSoundOnPause = FALSE;
            else
                TheGame.bSoundOnPause = TRUE;
            // Skip SetPause — game keeps running in multiplayer.
        }

        if (MenuManager == None)
        {
            if (class'OLUtils'.static.IsConsole())
            {
                if (class'OLUtils'.static.IsDingo())
                    MenuManager = new(self) class'OLUIFrontEnd_ConsoleXbox';
                else
                    MenuManager = new(self) class'OLUIFrontEnd_Console';
            }
            else
            {
                MenuManager = new(self) class'OLUIFrontEnd';
            }
            MenuManager.MenuType = MenuType;
        }

        if (MenuManager != None)
        {
            if (MenuType == EMT_MainMenu)
                MuteSelectSound(2.5);
            else
                MuteSelectSound();

            MenuManager.Start(false);

            if (CamcorderHUD != None)
                CamcorderHUD.SetVisible(false);
            HideHUDMessages();
        }
    }
}


event PostRender()
{
    // All HUD rendering is handled by the ImGui overlay (MpHud_DrawOverlay in OLImGui.cpp).
    // Snapshot is filled each tick by MpHud_UpdateSnapshot via FOLImGuiTicker.
    super.PostRender();
}


DefaultProperties
{
    MaxNickDisplay=6
}
