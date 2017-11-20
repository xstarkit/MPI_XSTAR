/*  
 * mpi_xstar: A parallel execution of multiple XSTAR runs using 
 * Message Passing Interface (MPI). XSTAR is part of the HEASARC's 
 * standard HEADAS package, and is a computer program used for
 * calculating the physical conditions and emission spectra of 
 * photoionized gases (Kallman & Bautista 2001).
 * 
 * The master program (rank=0) runs xstinitable from the HEADAS to 
 * create a list of XSTAR commands for given physical parameters, 
 * so called joblist, as well as a xstinitable.fits file, which is 
 * necessary for producing xout_ain.fits, xout_aout.fits, and 
 * xout_mtable.fits. The joblist is used to create directories in 
 * ascending order, where each individual xstar is spawned on each 
 * processor and outputs are saved. When each processor spawns the 
 * xstar, the main thread is waited until the xstar execution is 
 * completed.
 * 
 * The master program (rank=0) then invokes xstar2table from the 
 * HEADAS upon the contents of each directory in order to produce 
 * table model files, namely xout_ain.fits, xout_aout.fits, and 
 * xout_mtable.fits, suitable for spectroscopy analysis tools such 
 * as ISIS or XSPEC.
 * 
 * Developed by A. Danehkar (ashkbiz.danehkar AT cfa.harvard.edu)
 * Copyright (c) 2016, Smithsonian Astrophysical Observatory
*/

#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <spawn.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <ctime>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

//using namespace std;

int countlines(char *filename);
int execute(char *command,char *task);
int ReadLine(char *filename, char* line_buffer, int linenum);
int DeleteLine(char *filename, int linenum);
int FileExists(char *filename);
int readlastLine(char *filename, char* line_buffer);

int main(int argc, char * argv[]) 
{
	int proc_num, proc_rank; 
	int proc_total;
	char hostname[256];
	pid_t pid;
	char xstinitable_run[4096];
	char xstinitable_argv[4096];
	char xstinitable_cmd[255];
	char xstar2table_run[2048];
	char xstar2table_cmd[255];
	char xstar_run[2048];
	char xstar_argv[2048];
	char xstar_cmd[255];
	char xstar_dir[255];
	char pathstr[15];
	char filestr[1024];
	char cwd[1024];
	int status;
	int index;
	int i;
	int JobListNum, JobListTotal; 
	char DefaultJobList[32];
	MPI_Info info;
	int proc_loop, job_num;
	int len;
	struct stat sb;
	// varibales for writing xstar2xspec.log
	FILE *fp1, *fp2, *fp3, *fp_err;
	char c;
	ssize_t commandlength;
	size_t len2 = 0;
	char * commandline = NULL;
	
	MPI_Request send_req, recv_req;    // Request object for send and receive
	MPI_Request request= MPI_REQUEST_NULL;;
	
	MPI_Init(&argc, &argv); //start mpi
	//MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE,&provided);
	
	//determine rnak of this processor
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank); 
	proc_num = proc_rank +1 ;
	
	//determine total number of processor
	MPI_Comm_size(MPI_COMM_WORLD, &proc_total);
	gethostname(hostname,255);
  
	if (proc_rank == 0)
	{
		strcpy(xstinitable_run, "xstinitable ");
		for(index = 1; index < argc; index++) 
		{
			strcat(xstinitable_run, argv[index]);
			strcat(xstinitable_run, " ");
		}
		len=strlen(xstinitable_run);
		memset(xstinitable_argv, 0, sizeof(xstinitable_argv));
		strncpy(xstinitable_argv, xstinitable_run+12, len-12);
		printf("%s\n", xstinitable_run);
		strcpy(xstinitable_cmd, "xstinitable");
		status=execute(xstinitable_cmd,xstinitable_run);
		status = system("cp xstinitable.fits xout_ain.fits");
		status = system("cp xstinitable.fits xout_aout.fits");
		status = system("cp xstinitable.fits xout_mtable.fits");
		status = system("cp xstinitable.lis xstinitable.txt");
	} 
	MPI_Barrier(MPI_COMM_WORLD);
	
	printf("Run the process %d of %d on host %s\n", proc_num, proc_total, hostname);
	
	strcpy(DefaultJobList, "xstinitable.txt"); 
	
	if (proc_rank == 0)
	{
		JobListNum=countlines(DefaultJobList);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(&JobListNum, 1, MPI_INT, 0, MPI_COMM_WORLD);
	JobListNum=JobListNum-1;
	printf("Total Jobs: %d \n", JobListNum);
	
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		printf("Current working dir: %s\n", cwd);
	else
		perror("getcwd() error");
	strcat(cwd, "/");
  
	job_num=proc_num;
	proc_loop = proc_num;
	while(proc_loop<= JobListNum){
		sprintf(pathstr, "%06d", proc_loop);
		if (stat(pathstr, &sb) == 0 && S_ISDIR(sb.st_mode))
		{
			sleep(proc_num);
			job_num++;
		}
		else
		{
			if (0 == (status = mkdir(pathstr,S_IRWXU))) 
			{
				status = ReadLine(DefaultJobList, xstar_run, proc_loop);
				
				strcpy(xstar_dir, cwd);
				strcat(xstar_dir, pathstr);
				printf("Xstar working dir: %s\n", xstar_dir);
				
				chdir(xstar_dir);
				//status = system("cp $HEADAS/syspfiles/xstar.par .");
				
				MPI_Info_create(&info);
				MPI_Info_set(info, "host", hostname);
				MPI_Info_set(info, "wdir", xstar_dir);
				printf("Running Xstar: %s\n", xstar_run);
				
				len=strlen(xstar_run);
				memset(xstar_argv, 0, sizeof(xstar_argv));
				strncpy(xstar_argv, xstar_run+6, len-6);
				printf("Running Xstar Arg: %s\n", xstar_argv);  
				strcpy(xstar_cmd, "xstar");
				status=execute(xstar_cmd,xstar_run);
				chdir(cwd);
			}
			//MPI_recv(done, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			//MPI_Wait(&request, &status);
			//proc_loop++;
			sleep(proc_num);
			if (job_num<proc_total)
			{
				job_num=proc_total;
			job_num++;
			}
			else
			{
				job_num++;
			}
		}
		MPI_Bcast(&job_num, 1, MPI_INT, proc_rank, MPI_COMM_WORLD);
		proc_loop=job_num;
		//for (i=1; i<tprocs; i++) MPI_Isend(&done, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if (proc_num == 1)
	{
		// Create xstar2xspec.log to store xout_step.log
		fp2 = fopen("xstar2xspec.log", "wb");
		if (!fp2) printf("Could not create xstar2xspec.log \n");
		fp3 = fopen("xstinitable.lis", "r");
		if (!fp3) printf("Could not find xstinitable.lis \n");
		// start xstar2xspec loop
		for(index = 1; index <= JobListNum; index++) 
		{
			sprintf(pathstr, "%06d", index);
			strcpy(filestr,"./");
			strcat(filestr, pathstr);
			strcat(filestr, "/xout_spect1.fits");
			// does the xout_spect1.fits file exist?
			if( access(filestr, F_OK ) == -1 ) {
				// file doesn't exist
			}
			strcpy(xstar2table_run,"xstar2table xstarspec=");
			strcat(xstar2table_run, filestr);  
			printf("command: %s\n", xstar2table_run);
			strcpy(xstar2table_cmd, "xstar2table");
			status=execute(xstar2table_cmd,xstar2table_run);
			// write xstar2xspec.log
			printf("run xstar2xspec on %06d \n", index);
			sprintf(pathstr, "%06d", index);
			strcpy(filestr,"./");
			strcat(filestr, pathstr);
			strcat(filestr, "/xout_step.log");
			// does the xout_step.log file exist?
			if( access(filestr, F_OK ) == -1 ) {
				// file doesn't exist
			}
			commandlength = getline(&commandline, &len2, fp3); 
			fprintf(fp2, "=============================================\n");
			fprintf(fp2, "Command Line:\n");
			fprintf(fp2, "%s", commandline);
			fprintf(fp2, "\n\nXSTAR Log:");
			fp1 = fopen(filestr, "r");
			if (fp1 == NULL) printf("Could not open xout_step.log of %s", pathstr);
			while ((c = fgetc(fp1)) != EOF) fputc(c, fp2);
			fclose(fp1);
			// update xstar2xspec.log with xout_step.log
		}
		if (commandline) free(commandline);
		fclose(fp2);
		fclose(fp3);
	} 
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();  // EXIT MPI 
}

int execute(char *command,char *run)
{
	int world_rank;
	int rc;
	MPI_Info info;
	char hostname[256];
	//char task[2048];
	
	gethostname(hostname,255);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); 
    
    pid_t child_pid, pid;
    int status;
    MPI_Comm parent, child;
    // If we get COMM_NULL back, then we're the parent 
    MPI_Comm_get_parent(&parent);
    if (parent == MPI_COMM_NULL) 
    {
       pid = getpid();
       printf("Parent [pid %ld] about to spawn!\n", (long)pid);
    }
    //Prepare command line arguments as a null-terminated array
    /*strcpy(task, run);
    std::vector<char*> args;
    char* tasks = strtok(task, " ");
    while(tasks != NULL) 
    {       
        args.push_back(tasks);
        tasks = strtok(NULL, " ");
    }
    args.push_back(NULL);*/
   
    /*MPI_Info_create(&info);
    MPI_Info_set(info, "host", hostname);
    //MPI_Info_set(info, "wdir", rdir);
    // MPI_Info_set(info, "ompi_prefix", prefix);
    MPI_Info_set(info, "ompi_local_slave", "true");
    //if (NULL != rdir) {
    //       MPI_Info_set(info, "ompi_preload_binary", "true");
    //       MPI_Info_set(info, "ompi_preload_files_dest_dir", rdir);
    //}         
    // Spawn application
    if (MPI_SUCCESS != (rc = MPI_Comm_spawn(command, &args.front(), 1, info, 
                                           world_rank, MPI_COMM_WORLD, &child, MPI_ERRCODES_IGNORE))) {
        //if (MPI_SUCCESS != (rc = MPI_Comm_spawn("xstar", argv, 1, info, 
        //                                    proc_rank, MPI_COMM_SELF, &child, MPI_ERRCODES_IGNORE))) {
       printf("Child failed to spawn\n");
       return rc;
    }    
    // Wait for the waiters to enter the barrier
    printf("Parent wait for child\n");
    MPI_Barrier(child);
    printf("Parent wait for child\n");*/
     
    if ((child_pid = vfork()) < 0) 
    {        
		exit(1);
    }        
    //Child process
    if (child_pid == 0) 
    {               
        //Execute program args[0] with arguments args[0], args[1], args[2], etc.
        printf("Child [pid %ld] about to run!\n", (long)child_pid);
        //execvp(command, &args.front());
        status = system(run); 
        _exit(1);
        //MPI_Finalize(); 
    } 
    else if (child_pid > 0)
    {
		// PARENT 
		if( (pid = waitpid(child_pid,&status,0)) < 0)
		{
			printf("wait\n");
			_exit(1);
		}
		printf("Parent: finished\n");
	}
	else
	{
		printf("fork failed\n");
		_exit(1);
	}
	/*    
    //Parent process
    else {
        //Wait for child process
         printf("Parent [pid %ld] wait!\n", (long)child_pid);
        //wait(&status);  
         while (wait(&status) != child_pid)       // wait for completion  
              ; 
    }  */     
    if (parent != MPI_COMM_NULL)
    {
		MPI_Barrier(parent);
    }
    //Return status message
    return status;
}

char *mpi_fgets(char *buffer, int num, MPI_File fh) 
{
	int i,err;

	for (i=0; i<num-1; i++) 
	{
		err=MPI_File_read(fh,buffer+i,1,MPI_CHAR,MPI_STATUS_IGNORE);
		if (err!=MPI_SUCCESS) 
		{
			printf("mpi_fgets: MPI_File_read: Error\n");
			return NULL;
		}
		if (buffer[i]=='\n' || buffer[i]=='\0') 
		{
			i++; 
			break;
		} // Always have i be 1 more char ahead   
	}
	if (i==num && buffer[i-1]=='\n') buffer[i-1]='\0';
	else buffer[i]='\0';
	
	return buffer;
}

int mpi_ReadLine(char *filename, char* line_buffer, int linenum)
{
	// count the number of lines in the file called filename   
	int err;
	MPI_File fh;
	int mysize;
	int proc_total, proc_rank; 
	MPI_Offset filesize, globalstart, globalend;
  
	//determine rnak of this processor
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank); 
	
	//determine total number of processor
	MPI_Comm_size(MPI_COMM_WORLD, &proc_total);
	
	printf("ReadLine\n");
	err=MPI_File_open(MPI_COMM_WORLD, filename, 
					MPI_MODE_RDONLY, // | MPI_MODE_SEQUENTIAL, 
					MPI_INFO_NULL, &fh );
	if (err!= MPI_SUCCESS)
	{
		printf("MPI_File_open: Error\n");
		//MPI_Abort(MPI_COMM_WORLD, 911);
	}         
	err=MPI_File_get_size(fh, &filesize);
	if (err!= MPI_SUCCESS)
	{
		printf("MPI_File_get_size: Error\n");
		//MPI_Abort(MPI_COMM_WORLD, 911);
	}  
	filesize--;
	mysize = filesize/sizeof(char); 
	printf("filesize: %d\n", mysize);  

	int line_number=0;

	line_number++;
	while(mpi_fgets(line_buffer, 2024, fh) != NULL)
	{
		line_buffer[strlen(line_buffer) - 1] = '\0';
		printf("Line: %s\n", line_buffer); 
		if (line_number == linenum) break;
		line_number++;
	}
  
	err = MPI_File_close( &fh );
	return 0;
}

int ReadLine(char *filename, char* line_buffer, int linenum)
{
	int ch=0;
	int line_number=0;    
	                            
	FILE *fp = fopen(filename,"r");
	if (fp == NULL) return 0;
	line_number++;
	while(fgets(line_buffer, 2024, fp) != NULL)
	{
		line_buffer[strlen(line_buffer) - 1] = '\0';
		if (line_number == linenum) break;
		line_number++;
	}
	fclose(fp);
	return 0;
}

int DeleteLine(char *filename, int linenum)
{
	int status;
	int ch=0;
	int line_number=0;
	char line_buffer[2024];
	                          
	FILE *fp1 = fopen(filename,"r");
	if (fp1 == NULL) return 0;
	remove("xstinitable.tmp");
	FILE *fp2 = fopen("xstinitable.tmp","w+");
	if (fp2 == NULL) return 0;

	line_number++;
	while(fgets(line_buffer, 2024, fp1) != NULL)
	{
		line_buffer[strlen(line_buffer) - 1] = '\0';
		if (line_number != linenum) 
		{
			fprintf(fp2, "%s", line_buffer);
		}
		line_number++;
	}
  
	fclose(fp1);
	fclose(fp2);

	remove(filename);
	status = system("cp xstinitable.tmp xstinitable.txt");
	remove("xstinitable.tmp");
	return 0;
}

int ReadDeleteLine(char *filename, char* line_buffer, int linenum)
{
	int status; 
	int ch=0;
	int line_number=0;
	char line1[2024];
                               
	FILE *fp1 = fopen(filename,"r");
	//remove("xstinitable.tmp");
	FILE *fp2 = fopen("xstinitable.tmp","w+");
	if (fp1 == NULL) return 0;
	if (fp2 == NULL) return 0;
	line_number++;
	while(fgets(line1, 2024, fp1) != NULL)
	{
		line1[strlen(line1) - 1] = '\0';
		if (line_number == linenum) strcpy(line_buffer, line1);
		if (line_number != linenum) 
		{
			fprintf(fp2, "%s", line1);
		}
		line_number++;
	}
	fclose(fp1);
	fclose(fp2);

	remove(filename);
	status = system("cp xstinitable.tmp xstinitable.txt");
	remove("xstinitable.tmp");
	return 0;
}

int countlines(char *filename)
{    
	int ch=0;
	int lines=0;                            
	FILE *fp1 = fopen(filename,"r");
	if (fp1 == NULL) return 0;

	lines++;
	while(!feof(fp1))
	{
		ch = fgetc(fp1);
		if(ch == '\n')
		{
			lines++;
		}
	}
	fclose(fp1);
	return lines;
}


int FileExists(char *filename)
{
	FILE *fp = fopen(filename, "r"); 
	if (NULL == fp) 
	{ // file does not exist
		return 0;
	}
	else 
	{ // file exists
		fclose(fp);
		return 1;
	}
}

int readlastLine(char *filename, char* line_buffer)
{                                
	int ch=0;
	int line_number=0;
	static const long max_len = 2024;
	FILE *fp = fopen(filename,"r");
	if (fp == NULL) return 0;
	fseek(fp, -max_len, SEEK_END);         
	fread(line_buffer, max_len-1, 1, fp); 
	fclose(fp);
	return 0;
}
