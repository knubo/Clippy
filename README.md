# Clippy
Clippy is a compile helper, original created for Viking Mud. It drastically shorten down your round trip while working with code.

It works that after a file/files has been freshed (loaded) it periodically scans the timestamps. If it detect a timestamp change, it will attemt to load it. It also scans log files for errors and present them to the owner of this Clippy. So after uploading a file you don't need to continously reload it and tail error messages.

It also has some basic understanding of intheritance, if you use full path to whatever is inherited, it will refresh files based on the inheritance tree. If you are carrying a copy of the item that is updated, it will also refresh that copy of the item. Like

inherit "/path/to/file";

It does not understand includes in this verison.


