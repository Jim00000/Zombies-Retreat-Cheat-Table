# Zombies-Retreat-Cheat-Table

Visit https://sirensdomain.itch.io/zombiesretreat for more game information <br/>
Notice that this cheat table only supports Cheat Engine 7.1 or above

## About Zombie's Retreat 2: Gridlocked

[Zombie's Retreat 2: Gridlocked](https://sirensdomain.itch.io/zombies-retreat-2) is out now. In fact, a few cheats still take effect on Zombie's Retreat 2 but the others are broken. You may try my another [repository](https://github.com/Jim00000/Zombies-Retreat-2-Cheat-Script) for Zombie's Retreat 2 in a different implementation of cheat which use javascript instead of Cheat Engine.

# Cheat Table Content

- All kinds of ammo x99 & medicinal herb x99 (Fire your guns or use herbs to make value change)
- Get all of items x90 (See Note 1)
- Set held items x90 (See Note 1)

## Note 1

I found out that this game sometimes crashes while the cheat is activated. To avoid game crash, activate the cheat while the game menu is open and deactivate the cheat before leaving the game menu. Be sure to click "Item" entry in the game menu to make cheat take effect.

## Caveat

"Get all of items x90" cheat will make game crash or freeze. It is not recommended to use.

# How to select correct process

## Use SpeedHack
Run Zombie's Retreat executable at first. Open Cheat Engine, "File" -> "Open Process" to open "Process List" windows. Click "Processes" tab, You will see list like "000004AC - svchost.exe". There are four processes likes "00001CF0-Zombie's Retreat - Beta x.xx.x.exe". Only one process is actually that our cheat takes effect. But which one ? You can use speedhack (there is a "Enable speedhack" checkbox) to check which process autually got influenced and that's what we want.

![Imgur](https://i.imgur.com/oJEbPll.png)
![Imgur](https://i.imgur.com/fsYC94G.gif)

## Use task manager in Windows (Simple but not 100% accurate, maybe)

Run Zombie's Retreat executable at first. Open task manager (shortcut: Ctrl+Shift+Esc), open "Details" tab, you can find 4 "Zombie's Retreat - Beta x.xx.x.exe" processes. Find the process which uses the most memory, and use calculator application with "Programmer" mode to convert its PID from decimal format to heximal format. Match that value in the Process List window of Cheat Engine program.

![Imgur](https://i.imgur.com/lr7L1qZ.gif)

# Demo

## All kinds of ammo x99 & medicinal herb x99

![Imgur](https://i.imgur.com/0MaD6vb.gif)

## Set held items x90

![Imgur](https://i.imgur.com/2HPr8Zf.gif)

# Change Log

## 05.29.2020 update
- Support for beta 0.14.4 version

## 05.08.2020 update
- Support for beta 0.13.2 version
- Translate from Mandarin (zh-tw) to English (en)
- Upgrade "Make current equipment ..." cheat, fix bugs of "Make current equipment ..." cheat and rename "Make current equipment ..." cheat as "All kinds of ammo x99 & medicinal herb x99"
## 01.28.2020 update
- Initial release for Beta 0.12.2

# License

MIT License
