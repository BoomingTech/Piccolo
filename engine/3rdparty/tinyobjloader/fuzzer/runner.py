import os, sys
import glob
import subprocess

def main():
    for g in glob.glob("../tests/afl/id*"):
        print(g)
    
        cmd = ["../a.out", g]

        proc = subprocess.Popen(cmd)
        try:
            outs, errs = proc.communicate(timeout=15)
            print(outs)
        except TimeoutExpired:
            proc.kill()
            outs, errs = proc.communicate()


main()
