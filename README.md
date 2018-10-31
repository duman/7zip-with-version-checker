# 7zip-with-version-checker
This program will allow you to check if there's a newer version of 7-Zip before launching it.

<p align="center"><img align="center" src="https://image.ibb.co/kQ9eoL/version1.png" alt="Update box"></p>

* Installation is pretty straightforward, go to [releases](https://github.com/tkduman/7zip-with-version-checker/releases).
* Download the latest version.
* Unpack content to 7-Zip installation folder.
* Create a shortcut for the _7-Zip with Version Check.exe_
* Launch 7-Zip from this exe.

With the current version program will check if there's an update per week, and if there's none it will just launch it. Check happens one time in a week, thus after first launch it won't ask or try to check anymore and will just launch it.

* Clicking yes will open 7-Zip's downloads page, you can download the version that suits you.
* Clicking no will just launch the application and ignore checking updates for a week.

## To-do

* System-tray/background application, so that this program won't have to be the default launcher.
* Allow users to edit next check date in a friendlier way. (It's already possible, but requires settings to be modified with UNIX time.)
* Get x64/x86 version and auto download the installer upon clicking yes.

## Contribution

Feel free to improve the code and open issues. Have fun.
