/*
 *  "Unique PID" Handling
 *
 *  This file created as part of the mkvol utility.
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

// Struct for unique PID.
struct upid_t
{
    unsigned long stime;
    pid_t pid;
};


// Generate a UPID struct for a givin PID.
struct upid_t get_upid(pid_t pid_origin)
{
    // Generate the path for the proc file of a PID.
    char proc_path[24];
    sprintf(proc_path, "/proc/%d/stat", pid_origin);
    
    int line_index = 0;
    int spacer_index = 0;
    int word_index = 0;
    char line_buffer[512];
    char word_buffer[32];
    struct upid_t generated_upid;
    
    generated_upid.pid = pid_origin;
    generated_upid.stime = 0;
    
    // Try to open and read proc file, or return an 
    // error.
    FILE * proc_file = fopen(proc_path, "r");
    if(!proc_file)
    {
        generated_upid.pid = -1;
        return(generated_upid);
    }
    else
    {
        if(!fgets(line_buffer, 512, proc_file))
        {
            generated_upid.pid = -1;
            return(generated_upid); 
        }
        
        // Get the start time of the process from 
        // it's stat file.
        // Best not to look at this too hard...
        for(line_index = 0; !(word_buffer[word_index] == '\0' && spacer_index == 22); line_index++)
        {
            if(line_buffer[line_index] == ' ')
            {
                spacer_index++;
            }
            else if(spacer_index == 21)
            {
                word_buffer[word_index] = line_buffer[line_index];
                word_index++;
            }
            else if(spacer_index == 22)
            {
                word_buffer[word_index] = '\0';
            }
            else if(spacer_index < 22 && (line_buffer[line_index] == '\n' || line_buffer[line_index] == '\0'))
            {
                generated_upid.pid = -1;
                return(generated_upid); 
            }
        }
        
        generated_upid.stime = strtoul(word_buffer, NULL, 10);
        return(generated_upid); 
    }
}


// Test a UPID to see if it died or the PID changed
// hands.
int check_upid(struct upid_t target_upid)
{
    struct upid_t current_upid = get_upid(target_upid.pid);
    if(!current_upid.pid || (current_upid.stime != target_upid.stime))
    {
        return(EXIT_FAILURE);
    }
    else
    {
        return(EXIT_SUCCESS);
    }
}
