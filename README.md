# OpenOL — Modded "Client" for Outlast Game
***NOTE (mainly for Red Barrels):** **In addition** to the files from this repository, you should still have the **original source files and a licensed copy of the game** (for content files, etc.). The files from this repository are provided only for **modding convenience***<p>
If you happen to have the original Outlast source files, you can add/replace them with these

The resulting `OLGame.exe`, along with `Core.upk`, `Engine.upk`, `OLGame.upk`, or any other (post-cook) `.upk` files to which you add UnrealScript, can be used to replace the original files in **your licensed Steam version of the game**.

## Currently, the modified source code allows you to:

### Content/Maps/Shaders

- Load original content/maps/shaders (including DLC)
- Load custom maps via a separate tab in the main menu (maps displayed in this tab must be placed in the `CookedPCConsole/Mods/Persistent` folder)
- Cook `.upk` files, saving textures and lighting in separate `.tfc` files (which are read from any subfolder of `CookedPCConsole`)
- Cook `.upk` files, saving `Textures`, `CharTextures `and `Lighting` within each .upk (without `.tfc` files). This is useful when you don’t want to drag three separate `.tfc` files just for one `.upk`
- Adding a custom suffix to `RefShaderCache` when cooking `.upk` files
- Asset redirection for only-script packages (`.u`). For example, updated scripts from `OLGame.u` now take priority over original `OLGame.upk`, and loading order is: `.u` script -> group-referenced `.upk` (e.g. `menuassets.upk`) -> original `OLGame.upk`

### Multiplayer (beta)

- Connect to [OpenOL relays](https://github.com/ShyKiss/OpenOL-Relay) and play in multiplayer mode
- Influence other clients worlds (Doors/Triggers/Enemies/Cutscenes)

### New Console Commands

- `Reload` reload current checkpoint
- `SpawnEnemy <Enemy> <Weapon> <ShouldAttack>` spawns enemy with specified weapon and attack control
- `ToggleSmoothCamera` toggles smooth camera setting
- `ToggleUnlockDoors` toggles whether player ignores locked/blocked doors
- `ToggleAllowGhostDoors` toggles ability to do “ghost door” glitch (e.g. for speedruns)
- `MaxFPS <Number>` alias for `Set Engine MaxSmoothedFramerate <Number>` command
- `Unlit` / `Lit` / `ToggleUnlit` sets/toggles viewmode to Unlit or Lit

### Fixes and New Features:

- Player mesh matches checkpoints and game type
- Player footstep sounds are traced to the first texture, not to a collision + trace from each foot, not from center of the mesh
