import os
import argparse
import glob
import shutil
from subprocess import call

"""
This program needs to be run with python3!

It will either check whether the SusHi jobs finished and, thus, produced a summarized output file job_n.out and that this file is filled (its size must not be 0).
In that configuration, it will also check for tared outfiles which contain the result of the single grid points which are actually analyzed by the postprocessor.
If any of these files is not found, the input is tared again and the whole job is resubmitted to the grid.

If -a is specified as an option, the script checks the untared output files, i.e. the results of SusHi and 2HDM calculations for each single grid point.
They must contain two blocks, one for SusHi, one for 2HDM. Sometimes the SusHi part can not exist because some phase space regions are not accessible and, thus, integrals do not converge.
Sometimes, the decays in the 2HDM blocks are missing (extremely rare but happening).
If some of the above conditions are not passed, the postprocessor, which relies on all of them, will fail.
These corrupted/bad files will be moved to a specific directory. The user may decide what to do with them. They are also listed in a txt file, which, for large numbers of files, is easier to search.
"""

def Parser():
    parser = argparse.ArgumentParser(description='Checking existence of all outfiles in certain directory')
    parser.add_argument('-d', type=str, help='directory to be checked', dest='pathname')
    parser.add_argument('-a', dest='after_untaring', action='store_true', help='If specified, the untared output files will be checked for containing results of SusHi. If this part is missing, the job will be rerun locally (only the single .in file, not the whole condor job).')
    arguments = parser.parse_args()
    return arguments
command = Parser()

print("Checking individual (untared) outfiles.") if command.after_untaring else print("Checking tared files and input.")

#resubmitting jobs: go to job dir, tar input, submit to condor
def resubmit(workingdir,jobnumber):
    os.chdir(workingdir + '/' + command.pathname + '/sushi_out/' + 'job_' + jobnumber)
    call(('tar czf ../in_' + jobnumber + '.tgz *in sushi'),shell=True)
    call(('condor_submit condor_submit_job_' + jobnumber + '.sub'),shell=True)
    os.chdir(workingdir)

def movefile(file_to_move,target_dir):
    movestring = 'mv ' + file_to_move + ' ' + target_dir
    call(movestring,shell=True)

if command.pathname == None:
    raise TypeError("Please specify a path to check the output with '-d'.")

cwd = os.getcwd()
sushidir = cwd+'/'+command.pathname+'/sushi_out/'
print("Checking " + sushidir)

i = 0
points = 0
failures = 0
outerror = 0
checkedjobs = []
resubmitjobs = []
for jobdir in os.listdir(sushidir):
    if 'Partially_Failed_SusHi_Jobs' in jobdir or 'checked_jobs.txt' in jobdir:
        continue
    i = i+1
    print("Analyzing folder " + str(i) + " of " + str(len(os.listdir(sushidir)) - 2) )
    if jobdir.find('in_') != -1:
        continue
    else:
        jobdirstr = sushidir + jobdir
        jobnumber = jobdirstr.split('_')[-1]
        jobstr = 'job_'+str(jobnumber)            
        print(jobstr)
        with open(sushidir+'checked_jobs.txt','r') as whatwehave:
            analyzed_before = whatwehave.read()
            if ("'"+jobstr+"'") in analyzed_before:
                print("Already checked.")
                continue
        checkedjobs.append("'"+jobstr+"'")
        if not command.after_untaring:#checking tared files for existence/filling
            if len(glob.glob(jobdirstr+'/job*out')) == 0 or len(glob.glob(jobdirstr+'/out*tgz')) == 0:#two possibilities of failure
                outerror = outerror + 1
                if len(glob.glob(jobdirstr+'/job*out')) == 0:#1: no summary file at all
                    print("No summarized outfile found for job " + jobnumber + ". Resubmitting.")
                    resubmitjobs.append(jobnumber)
                elif len(glob.glob(jobdirstr+'/out*tgz')) == 0:#2 no tared file list, i.e. nothing to analyze for postprocessor
                    print("No tared outfile found for job " + jobnumber + ". Resubmitting.")
                    resubmitjobs.append(jobnumber)
                resubmit(cwd,jobnumber)
            else:
                globout = glob.glob(jobdirstr+'/job*out')[0]#if existent, check the summary file
                outsize = os.path.getsize(globout)
                outstrsplit = globout.split('.')[0]
                jobnumber = outstrsplit.split('_')[-1]
                if outsize == 0:#if the summary file does not contain anything, the job just failed on condor and no results were produced
                    outerror = outerror + 1
                    print("Output file of job " + str(jobnumber) + " is 0. Resubmitting.")
                    resubmit(cwd,jobnumber)
        else:#checking untared, individual, results for each grid point
            faileddir = sushidir + 'Partially_Failed_SusHi_Jobs'
            if not os.path.exists(faileddir):#create a storage, we don't know what to do with its content, let's call it Gorleben
                os.makedirs(faileddir)
            with open((faileddir + '/non_existent_in_files.txt'), 'a') as writetxt:#also add failed files to a txt, may be easier to search in it
                untaredoutfiles = glob.glob(jobdirstr+'/type*out')#checking for the individual files
                for outfile in untaredoutfiles:#full path to each output file
                    points = points + 1
                    fileoutname = outfile.split('/')[-1]#actual name of file
                    fileinname = fileoutname.split('.')[0] + '.in'#respective input file
                    fullfileinname = jobdirstr + '/' + fileinname
                    with open(outfile,'r') as labmouse:
                        sushi_success = labmouse.read().find('Supersymmetric')#did SusHi run?
                    with open(outfile,'r') as labrat:#python can just store one find() result for some reason
                        decay_success = labrat.read().find('DECAY')#are the decays in 2HDM calculated?
                    with open(outfile,'r') as labmonkey:
                        twohiggs_success = labmonkey.read().find('Two-Higgs')#
                    if sushi_success == -1:
                        failures = failures + 1
                        print("No SusHi found. Storing respective files in Gorleben and blaming Japan.")
                        movefile(outfile,faileddir)#move file to Gorleben if there is no other file with the name (then the script fails)
                        writetxt.write(outfile + '\n')
                        if not os.path.exists(fullfileinname):#if the input file is not existent for some reason, just copy the name it would have to the txt file
                            print("No .in file found. Sending path to Gorleben.")
                            writetxt.write(fullfileinname + '\n')
                            continue#to prevent this script from failing
                        movefile(fullfileinname,faileddir)#if the input file is there, just move it to Gorleben
                    if decay_success == -1 or twohiggs_success == -1:
                        failures = failures + 1
                        print("2HDM failed (either not or only partially produced). Moving file to Gorleben.")
                        movefile(outfile,faileddir)
                        writetxt.write(outfile + '\n')
                    with open(sushidir+'checked_jobs.txt','w') as alreadychecked:
                        for checkedjob in checkedjobs:
                            alreadychecked.write(checkedjob+'\n')
print("Checking and/or resubmission finished. " + str(failures) + " of " + str(points) + " grid points could not be processed. Check Gorleben at './sushi_out/Partially_Failed_SusHi_Jobs'. " + str(outerror) + " job(s) had to be resubmitted.")
print(resubmitjobs)

##########################################################################################
##########################################################################################
### Later: To-Do: Implement the whole after-untaring part directly into the framework. ###
### It should be possible to run the postprocessor and it should directly move the     ###
### partially failed files to a certain directory/txt-file                             ###
##########################################################################################
##########################################################################################
### Perhaps the if not path.exist part produces error. Might be necessary to think of  ###
### another solution for the copy-issue                                                ###
##########################################################################################
##########################################################################################
