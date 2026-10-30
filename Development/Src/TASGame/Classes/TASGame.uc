// Separate GameInfo for Tool-Assisted-Speedrun tooling (frame advance + scripted input
// playback), kept out of OLGame proper the same way MultiplayerGame is kept in its own
// package. Select it at launch via ?game=TASGame.TASGame.
class TASGame extends OLGame;

static event class<GameInfo> SetGameType(string MapName, string Options, string Portal)
{
    return Default.class;
}

DefaultProperties
{
    PlayerControllerClass=Class'TASGame.TASPlayerController'
    DefaultPawnClass=Class'OLGame.OLHero'
}
