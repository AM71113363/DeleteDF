# DeleteDF
-----
1. Will delete duplicate files.(only files <= 100MB)
2. Optional: "Restore" >> create "restore.bat" to put them back.
3. Optional: "Same DIR" >> multiple Scans.
<br>
### How do I use this?
1. Run DeleteDF.exe (32Bit Windows).
2. Drag-Drop a Directory to Scan.
3. CLick [ START ].
<br>
OPTIONS:
Enable "Restore" if you want to restore them later.
Enable "Same DIR" to delete duplicate files (including previous Scans).
<br>
### Build.
_for beginners_
1.Download [DevCpp_v4.9.9.*](http://www.bloodshed.net/) and install it.
2.Run build.bat (it works only with Compiler:  Dev-C++ 4.9.9.*).
_others_
You already know how to rebuilt it. ^_^
<br><br><br><br>
#### Why RESTORE them???
_example_
You have a GAME folder and you want to save it as a BACKUP.
Before compressing(or not) the folder, delete duplicate files(Enable RESTORE).
Later you can run "restore.bat" to put them back where they were.
<br><br>
#### How "Same DIR" Works???
_example_
FOLDERS "A" & "B" & "C" contain the same file "test.txt".
FOLDERS "B" & "C" contain the same file "image.bmp".

1.Enable "Same DIR".
2.Drag-Drop folder A.
3.Click [ START ].
4.Drag-Drop folder B.
5.Click [ START ].
6.Drag-Drop folder C.
5.Click [ START ].
<br>
RESULT: the file "test.txt" in "B"&"C" will be deleted, because there is a duplicate on folder A.
RESULT: the file "image.bmp" in "C" will be deleted, because there is a duplicate on folder B.
<br>
## NOTE
You can't use "Restore" & "Same DIR" at the same time.