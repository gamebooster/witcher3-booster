# Script Compilation Guide for Witcher 3

### only a pre-release (mostly untested and most stuff most likely doesn't work)

requirements: newest witcher3-booster (to load scriptplugins and get script-compilation messages)

 1. Create the directory `The Witcher 3 Wild Hunt\content\content0\scripts`
 * Copy `example_commands.ws` into it
 * Start `witcher3.exe -dumpscripts`
 * Open `The Witcher 3 Wild Hunt\bin\script_compilation.log` to see if something went wrong
 * Copy and rename `The Witcher 3 Wild Hunt\content\content0\x64.final.redscripts` to `The Witcher 3 Wild Hunt\bin\x64\scriptplugins\examplecommands.redscriptsplugin`
 * Start `witcher3.exe` and enjoy your added commands `showhud`, `hidehud` etc.

You can search for new functions in `functions_log_for_script.txt` or enums in `enums_log_for_script.txt`

### Troubleshoot

If `witcher3.exe` crashes directly after startup you forgot to include all properties for a class with an exposed function with arguments or the class or function is simply unsupported (wrongs flags etc)

If gerald doesn't move anymore or something else doesn't work anymore you overwrited a class with `import class` that handles events etc. for example `import class CR4Player`

### RedScript

Witcher 2 scripts as language reference: https://github.com/haggen/Witcher2/tree/master/base_scripts/game
Decompiled Witcher 3 Code without Control-Flow (Sir_Kane): https://raw.githubusercontent.com/gamebooster/witcher3-booster/master/script_compilation/decompiled_w3_code_without_cfg_sir_kane.txt

#### Tips

* name (CName) `'pig'`
* Put `start /W witcher3.exe -dumpscripts && COPY /Y ..\..\content\content0\x64.final.redscripts scriptplugins\myplugin.redscriptsplugin` into  `compile.bat` in `The Witcher 3 Wild Hunt\bin\x64`
