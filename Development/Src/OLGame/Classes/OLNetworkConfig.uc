// Stores multiplayer connection settings in OLMultiplayer.ini.
// Lives in OLGame so it's accessible before the Multiplayer package loads.
// FMpConnection::LoadConfig reads from this class's config section.
class OLNetworkConfig extends Object
    native
    config(Multiplayer);

var config string IP;
var config string UdpPort;
var config string UserName;
var config string RoomCode;
var config string Password;
var config bool   SyncInteractable;
var config bool   SyncEnemies;
var config bool   SyncMatinees;
var config bool   SyncPickups;
var config bool   SpeedrunMode;
var config string HostSteamID;  // SteamID of the host for P2P join

static function Save(string NewIP, string NewUdpPort, string NewUserName,
    bool bSyncInteractable, bool bSyncEnemies, bool bSyncMatinees, bool bSyncPickups,
    bool bSpeedrunMode,
    string NewRoomCode, string NewPassword)
{
    default.IP               = NewIP;
    default.UdpPort          = NewUdpPort;
    default.UserName         = NewUserName;
    default.RoomCode         = NewRoomCode;
    default.Password         = NewPassword;
    default.SyncInteractable = bSyncInteractable;
    default.SyncEnemies      = bSyncEnemies;
    default.SyncMatinees     = bSyncMatinees;
    default.SyncPickups      = bSyncPickups;
    default.SpeedrunMode     = bSpeedrunMode;
    StaticSaveConfig();
}

static function SaveHostSteamID(string NewHostSteamID)
{
    default.HostSteamID = NewHostSteamID;
    StaticSaveConfig();
}

DefaultProperties
{
}
