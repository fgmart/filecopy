# filecopy
copying a file byte-by-byte

I wrote this little utility to copy video files from directory A to directory B, where "directory A" was inside my Chromebook's linux world and "directory B" is a folder shared with the underlying OS.

For some reason, "cp" was failing and I discovered if I copy files one byte at a time, it works.

The utility also preserves the file's creation and modified timestamps.

ChatGPT helped me out here, particularly on the timestamp stuff - thank you, ChatGPT.
