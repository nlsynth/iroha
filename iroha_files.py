# Script to create tar.gz package. This will replace automake based "make dist".
import os

PACKAGE="iroha"
VERSION="0.1.0"

ARCHIVE=PACKAGE + "-" + VERSION

EXTRA = ["configure.ac", "configure", "ltmain.sh", "depcomp", "Makefile.in", "config.sub", "lib/cxx-rt.h", "missing", "config.guess", "install-sh", "aclocal.m4", "Makefile.am", "compile", "m4/ltoptions.m4", "m4/ltversion.m4", "m4/m4_ax_check_compile_flag.m4", "m4/libtool.m4", "m4/ltsugar.m4", "m4/lt~obsolete.m4", "iroha_files.py", "src/iroha.gyp"]

def GetGypFileList(gyp):
    gypdir = os.path.dirname(gyp) + "/"
    d = eval(open(gyp).read())
    targets = d['targets']
    files = []
    for t in targets:
        for s in t['sources']:
            files.append(gypdir + s)
    return files

def GetExtraFileList(base):
    b = os.path.dirname(base) + "/"
    files = []
    for e in EXTRA:
        files.append(b + e)
    return files

def CopyFiles(archive, files):
    os.system("mkdir " + archive)
    pdir = archive + "/"
    dirs = {}
    for fn in files:
        d = pdir + os.path.dirname(fn)
        if not d in dirs:
            dirs[d] = True
            os.system("mkdir -p " + d)
        os.system("cp -p " + fn + " " + pdir + fn)

def MakeTarBall(archive, files):
    os.system("rm -rf " + archive)
    CopyFiles(archive, files)
    os.system("tar cvzf " + archive + ".tar.gz " + archive)
    os.system("rm -rf " + archive)
    

if __name__ == '__main__':
    files = GetGypFileList("src/iroha.gyp") + GetExtraFileList("./")
    MakeTarBall(ARCHIVE, files)
