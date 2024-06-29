# Search for git
import subprocess

def xec(cmd, decode=True, chomp=True, verbose=False):
  '''Run a command a returns its stdout output.

  decode -- run (utf8) decode of the resulting string
  chomp -- get rid of the last newline (like in perl)
  '''
  with subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE) as ph:
    if verbose:
      sys.stderr.write(cmd + '\n')
    res = b''
    maxsize = 1024
    while True:
      buf = ph.stdout.read()
      if len(buf)==0:
        break
      res += buf
    ph.wait()
  if decode:
    res=res.decode()
  if chomp:
    res = res[:-1]
  return res


import sys,os
import json

def GetGitSha1(Ref="HEAD"):
    """Get the SHA1 of the Git reference"""
    return xec('git show -s --pretty=format:%H HEAD')

def GetGitCommitTime(Ref="HEAD"):
    """Get the SHA1 of the Git reference"""
    return xec('git show -s --pretty=\'%ci\' -n1')

def CompareSha(NewSha1, filename):
    try:
        with open(filename,'r') as fh: 
            s = fh.readline()
    
        if not len(s):
          return 0
        OldSha1 = s.split()[-1].replace('\r','').replace('\n','').replace(' ','')
        if OldSha1 == NewSha1:
          return 1
        else:
          return 0
    except FileNotFoundError:
        return 0

if __name__=='__main__':
    filename = sys.argv[1]
    
    GitSha = GetGitSha1()
    GitCommitTime = GetGitCommitTime()
      
    if CompareSha(GitSha, filename):
        print("Sha1 has not changed")
        exit()
    
    with open(filename,'w') as fh:
      fh.write(f'// Sha1: {GitSha}\n'
               f'// CommitDate: {GitCommitTime}\n'
               f'#ifndef __SHA1__\n'
               f'#define __SHA1__\n'
               f'#define BUILD_SHA1 "{GitSha}"\n'
               f'#define BUILD_COMMIT_TIME "{GitCommitTime}"\n'
               f'#endif\n')
    
