# DeleteDF
-----
1. Will delete duplicate files.(only files <= 100MB)<br>
2. Optional: "Restore" >> create "restore.bat" to put them back.<br>
3. Optional: "Same DIR" >> multiple Scans.<br>
<br>

# How do I use this?<br>
1. Run DeleteDF.exe (32Bit Windows).<br>
2. Drag-Drop a Directory to Scan.<br>
3. CLick [ START ].<br>
<br>
OPTIONS:<br>
Enable "Restore" if you want to restore them later.<br>
Enable "Same DIR" to delete duplicate files (including previous Scans).<br>
<br>

# Build.
_for beginners_ <br>
1.Download [DevCpp_v4.9.9.*](http://www.bloodshed.net/) and install it.<br>
2.Run build.bat (it works only with Compiler:  Dev-C++ 4.9.9.*).<br>
_others_ <br>
You already know how to rebuilt it. ^_^<br>
<br><br><br><br>

# Why RESTORE them???<br>
_example_ <br>
You have a GAME folder and you want to save it as a BACKUP.
Before compressing(or not) the folder, delete duplicate files(Enable RESTORE).
Later you can run "restore.bat" to put them back where they were.
<br><br>

# How Same DIR Works???<br>
_example_ <br>
FOLDERS "A" & "B" & "C" contain the same file "test.txt".<br>
FOLDERS "B" & "C" contain the same file "image.bmp".<br>
<br>
1.Enable "Same DIR".<br>
2.Drag-Drop folder A.<br>
3.Click [ START ].<br>
4.Drag-Drop folder B.<br>
5.Click [ START ].<br>
6.Drag-Drop folder C.<br>
5.Click [ START ].<br>
<br>
RESULT: the file "test.txt" in "B"&"C" will be deleted, because there is a duplicate on folder A.<br>
RESULT: the file "image.bmp" in "C" will be deleted, because there is a duplicate on folder B.<br>
<br>

# NOTE
You can't use "Restore" & "Same DIR" at the same time.
