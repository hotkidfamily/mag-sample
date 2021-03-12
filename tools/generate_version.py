import os
import re
import codecs
import subprocess
import xml.etree.ElementTree
import time

def getMainVer(fileName):
    with open(fileName, encoding='utf-8') as f:
        for line in f:
            line = line.replace(' ', '')
            result = re.search("mainVersion=", line)
            if not result:
                continue
            keyValue = line.split("=")
            return keyValue[1]
    return "1.0"

def getDataVer():
    return time.strftime('%m%d%H%M')

def getGitRevision():
    revision = subprocess.check_output('git rev-parse --short HEAD')
    revision = bytes.decode(revision)
    revision = revision[:-1]
    revision = str(int(revision, 16))
    return revision
    
def setRcVersion(rcfile, mainVerStr, fileVer, productVer):
    mainVer = mainVerStr.split(".")
    ver1Str = [mainVer[0], mainVer[1], fileVer[:4], fileVer[4:]]
    ver2Str = [mainVer[0], mainVer[1], productVer[:4], productVer[4:]]
    version1 = 'FILEVERSION ' + ','.join(ver1Str)
    version2 = 'PRODUCTVERSION ' + ','.join(ver2Str)
    fileDesc = R'            VALUE "FileVersion", "' + '.'.join(ver1Str) + r'"'
    productDesc = R'            VALUE "ProductVersion", "' + '.'.join(ver2Str) + r'"'

    with codecs.open(rcfile, 'r', 'utf-8') as f:
        content = f.read()
    pattern = re.compile('FILEVERSION \d+,\d+,\d+,\d+')
    content = pattern.sub(version1, content)
    pattern = re.compile('PRODUCTVERSION \d+,\d+,\d+,\d+')
    content = pattern.sub(version2, content)
    pattern = re.compile(R'            VALUE "FileVersion", "[\d.]+"')
    content = pattern.sub(fileDesc, content)
    pattern = re.compile(R'            VALUE "ProductVersion", "[\d.]+"')
    content = pattern.sub(productDesc, content)
    with codecs.open(rcfile, 'w', 'utf-8') as f:
        f.write(content)
    return

if __name__ == "__main__":
    slnPath = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    dataVer = getDataVer()
    svnVer = getGitRevision()
    mainVerStr = getMainVer(os.path.join(slnPath, "tools", "config", "version.info"))

    rcFile1 = os.path.join(slnPath, 'captureImp', 'captureImp.rc')
    rcFile2 = os.path.join(slnPath, 'my-mag-sample', 'my-mag-sample.rc')
    setRcVersion(rcFile1, mainVerStr, dataVer, svnVer)
    setRcVersion(rcFile2, mainVerStr, dataVer, svnVer)
    
