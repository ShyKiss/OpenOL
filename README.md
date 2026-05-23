# OpenOL — Modded "Client" for Outlast Game
***NOTE (mainly for Red Barrels):** **In addition** to the files from this repository, you should still have the **original source files and a licensed copy of the game** (for content files, etc.). The files from this repository are provided only for **modding convenience***<p>
If you happen to have the original Outlast source files, you can add/replace them with these

The resulting `OLGame.exe`, along with `Core.upk`, `Engine.upk`, `OLGame.upk`, or any other (post-cook) `.upk` files to which you add UnrealScript, can be used to replace the original files in **your licensed Steam version of the game**.

### Currently, the modified source code allows you to:

- Load original maps (including DLC)
- Load custom maps via a separate tab in the main menu (maps displayed in this tab must be placed in the `CookedPCConsole/Mods/Persistent` folder)
- Cook `.upk` files, saving textures and lighting in separate `.tfc` files (which are read from any subfolder of `CookedPCConsole`)
- Cook `.upk` files, saving `Textures`, `CharTextures `and `Lighting` within each .upk (without `.tfc` files). This is useful when you don’t want to drag three separate `.tfc` files just for one `.upk`
- Adding a custom suffix to `RefShaderCache` when cooking `.upk` files (unfortunately, the original `RefShaderCache`, like `GlobalShaderCache`, is not supported)

### There are also some fixes in the source code:

- Changing the player's mesh at specific checkpoints
- Changing the gamemode at specific checkpoints
