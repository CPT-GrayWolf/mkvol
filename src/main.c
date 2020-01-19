/*
 *  Make "Volatile" File
 *  
 *  Copyright (c) 2020 Ian "Luna" Ericson
 *  Finity Software Group
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

/* 
 * If NO_UPID is defined, don't worry about using
 * upid checking which requires a procfs stat file
 * to exist for the calling proces.
 * Should be more compatible, but re-introduces the
 * chance for a new process to be assigned the same
 * PID as the caller while mkvol is idle causing a
 * race condition.
 * This is unlikely, but we're prepared to handle it
 * anyway when using upids.
 */
#ifdef NO_UPID
    #define PID_TYPE pid_t
    #define GET_PID_TYPE getppid()
    #define CHECK_PID(PID) kill(PID, 0)
#else
    // Header for unique PID handeling.
    #include "upid.h"
    #define PID_TYPE struct upid_t
    #define GET_PID_TYPE get_upid(getppid())
    #define CHECK_PID(UPID) check_upid(UPID)
#endif
#define DEFAULT_DELAY 100000
#define TAURSON_DELAY 2000000


// Some globals for signal handling.
bool got_hup = false;
bool got_term = false;


// Test for a files presence using fopen().
bool file_test(const char* file_name)
{
    FILE * test = fopen(file_name, "r");
    if(test)
    {
        fclose(test);
        return(true);
    }
    else
    {
        return(false);
    }
}


// Exactly what is says on the box.
void print_help(void)
{
    puts("Usage: mkvol [OPTIONS]... FILE...");
    puts("Creates a file for as long as the original calling process is alive.\n");
    puts("  -f                    overwrite the file if it exists");
    puts("  -h                    display this help screen");
    puts("  -T                    narcolepsy mode");
    //puts("  -t                    set time in milliseconds between checks");
    //puts("  -v                    verbose output");
    puts("\nExamples:");
    puts("  mkvol file.out");
}


// Parent signal handler for hangup.
void psig_hup(int signum)
{
    return;
}


// Signal handler for unexpected child termination.
void psig_chld(int signum)
{
    fputs("mkvol: Child exited unexpectedly!\n", stderr);
    exit(EXIT_FAILURE);
}


// Child signal handler for hangup.
void csig_hup(int signum)
{
    got_hup = true;
    return;
}


// Child signal handler for terminate.
void csig_term(int signum)
{
    got_term = true;
    return;
}


// Test to see if the calling process has exited.
// Spin until it has.
int pid_spin(PID_TYPE caller_id, const char* file_name, int sleep_time)
{
    // Define a pointer we can use for SIGHUPs.
    FILE * new_file = NULL;
    bool crash = false;
    bool finish = false;
    
    // Loop until we reach an exit condition or
    // need to crash.
    while(!crash && !finish)
    {
        // Make the correct checks to see if the
        // calling process has died.
        if(!CHECK_PID(caller_id))
        {
            // Die if the file was removed or made
            // unaccessable.
            if(!file_test(file_name))
            {
                exit(EXIT_SUCCESS);
            }
            
            // Sleep a bit to save CPU time.
            usleep(sleep_time);
        }
        else if(errno == ESRCH)
        {
            finish = true;
        }
        else
        {
            crash = true;
        }
        
        // Check if there are any sig actions to take.
        if(got_term)
        { 
            finish = true;
        }
        else if(got_hup)
        {
            // For SIGHUP, remove the file and make a
            // new one.
            // Die on failure.
            remove(file_name);
            
            new_file = fopen(file_name, "w");
            if(new_file == NULL)
            {
                exit(EXIT_FAILURE);
            }
            fclose(new_file);
            
            got_hup = false;
        }
    }
    
    if(finish)
    {
        return(EXIT_SUCCESS);
    }
    else
    {
        return(EXIT_FAILURE);
    }
}


int main(int argc, char **argv)
{
    // Fail if no arguments are specified.
    if(argc < 2)
    {
        fputs("mkvol: No arguments given!\n", stderr);
        puts("Try \'mkvol -h\' for more information");
        exit(EXIT_FAILURE);
    }
    
    // Get callers ID based on what we're using.
    PID_TYPE caller_id = GET_PID_TYPE;
    
    // Set some defaults.
    bool overwrite = false;
    int spin_time = 100000;
    const char* file_name = NULL;
    
    // Parse commandline argument.
    for(int index = 1; index < argc; index++)
    {
        if(argv[index][0] == '-' && argv[index][2] == '\0')
        {
            switch(argv[index][1])
            {
                case 'f' :
                  overwrite = true;
                  break;
                case 'h' :
                  print_help();
                  exit(EXIT_SUCCESS);
                case 'T' :
                  spin_time = TAURSON_DELAY;
                  break;
                default :
                  fprintf(stderr, "mkvol: Invalid option \'-%c\'!\n", argv[index][1]);
                  puts("Try \'mkvol -h\' for more information");
                  exit(EXIT_FAILURE);
            }
        }else if(file_name == NULL)
        {
            file_name = argv[index];
        }
        else
        {
            fprintf(stderr, "mkvol: Unexpected argument \'%s\'!\n", argv[index]);
            puts("Try \'mkvol -h\' for more information");
            exit(EXIT_FAILURE);
        }
    }
    
    if(file_name == NULL)
    {
        fputs("mkvol: No file specified!\n", stderr);
        puts("Try \'mkvol -h\' for more information");
        exit(EXIT_FAILURE);
    }
    
    // Fail by default if the file already exists.
    if(file_test(file_name) && !overwrite)
    {
        fputs("mkvol: File already exists!\n", stderr);
        puts("Try -f to override");
        puts("Try \'mkvol -h\' for more information");
        exit(EXIT_FAILURE);
    }
    else if(errno == ENOENT || (file_test(file_name) && overwrite))
    {
        pid_t fork_result = fork();
        if(fork_result == -1)
        {
            fputs("mkvol: Unable to fork!\n", stderr);
            exit(EXIT_FAILURE);
        }
        else if(fork_result > 0)
        {
            // Set up signal handlers in parent.
            struct sigaction hangup;
            hangup.sa_handler = psig_hup;
            struct sigaction childterm;
            childterm.sa_handler = psig_chld;
            
            sigaction(SIGHUP, &hangup, NULL);
            sigaction(SIGCHLD, &childterm, NULL);
            
            // Wait for signal, then die.
            pause();
            puts("mkvol: Switching to background.");
            printf("mkvol: Child is PID %d\n", fork_result);
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Set up child signal handlers.
            struct sigaction hangup;
            hangup.sa_handler = csig_hup;
            struct sigaction terminate;
            terminate.sa_handler = csig_term;
            
            sigaction(SIGHUP, &hangup, NULL);
            sigaction(SIGTERM, &terminate, NULL);
            
            // Try to create the file, but fail and die if we 
            // can't.
            FILE * file_pointer = fopen(file_name, "w");
            if(file_pointer == NULL)
            {
                fputs("mkvol: Failed to create file!", stderr);
                exit(EXIT_FAILURE);
            }
            fclose(file_pointer);
            
            // We're all set.
            // Wait a bit then send SIGHUP to parent.
            usleep(10000);
            kill(getppid(), SIGHUP);
            
            // Spin until it's time to cleanup.
            pid_spin(caller_id, file_name, spin_time);
            
            // Cleanup the file and exit.
            remove(file_name);
            exit(EXIT_SUCCESS);
        }
    }
    else
    {
        // I need to put some extra error handling here.
        fputs("mkvol: Unhandled error!\n", stderr);
        fputs("mkvol: I just don't know what went wrong!\n", stderr);
        exit(EXIT_FAILURE);
    }
}
