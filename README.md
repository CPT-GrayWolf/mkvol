## Make Volatile Utility
The 'mkvol' utility allows a user to create a temporary file that will exist for the lifetime of the calling process.

The idea being that even if a fault in the calling process or script causes it to exit before freeing the file, 'mkvol' is simple enough that (barring extreme cases or meddling users) it will **always** remove the file. 

This allows simple, reliable creation of temporary "life of program" files, especially when writing shell scripts, or running tests.

## Installation
The 'mkvol' utility is incredibly simple to build, install, and use.

### Build
The program can be built using make:

	# make
However, if you encounter any errors building or using the program due to UPIDs (see UPIDs below), you can omit them from the build using:

	# make CFLAGS="-D NO_UPID"
This shouldn't be a problem on most systems running Linux, but may be needed for other *nix systems.
Keep in mind, while other UNIX-like OSs should work, they have now been tested. (Testing welcome!)

### Install
Installing 'mkvol' is, again, done with make:

	# make install
This will install both the 'mkvol' program, and its man page, which can be read using:

	# man mkvol
just like any other man page, assuming you have 'man' on your system (most do).

Cleanup:

For good measure, you can run:
	
	# make clean
after installation.

This isn't needed, but it's a good habit.

### Usage
I wont give a list of command options here. That's what the documentation is for.

Instead, I'll give a short example.

If you wanted, for example, to make a file with a known name and location, but make sure it was cleaned up when you closed your terminal, you could use 'mkvol'.

The created file is a regular file and can be used like any other file.
Lets look at 'mkvol' in use:
	
	$ bash
	$ ls
	$ echo $$
	22356
	$ mkvol test
	$ ls
	test.file
	$ cat test.file
	$ echo $$ > test.file
	$ cat test.file
	22356
	$ exit
	$ ls
	$ cat test.file
	cat: test.file: No such file or directory
If you didn't get that, we opened a new shell (with a PID of 22356), called 'mkvol' to create a temporary file, and wrote our shell's PID to the temporary file, where it was visible until the shell exited.
The file, and its contents were removed as soon as the shell that called 'mkvol' terminated.

Unlike file descriptors or tmpfs files, 'mkvol' files can be created anywhere and are visible to any program for as long as they exist.

## UPID
A quick note about PIDs and 'mkvol'.

PIDs are unique numbers assigned to each process at its start.
Until your system runs out of them...

This rarely happens in modern user systems under normal conditions, but when it does happen, the system will begin to reap old PIDs, assigning them to new processes.

Because 'mkvol' uses signaling to its caller's PID in order to determine if it's time to exit, there's e very minor chance of a race condition where the caller dies and is replaced by a new process of the same PID before 'mkvol' wakes up to run its check.

This would cause 'mkvol' to continue to exist, thinking that it's caller is still alive, even though it should have died.

To combat this, 'mkvol' can use Unique PIDs on systems that provide the correct information using procfs.

This isn't a problem under Linux where procsf is almost always mounted and provides the correct files, but on some systems, like FreeBSD, procfs may not be mounted at all, or may not give the correct information needed to generate the UPIDs that 'mkvol' expects.

If this is the case on your system you can compile 'mkval' using the NO_UPID option, as shown above. This omits the code for UPIDs completely, opening up the chance for the race condition to occur, but seeing as it's not a likely problem to begin with, it should be fine.

I hope to fix this in the future, so that UPIDs will work under most systems, but for now, this is fine.
