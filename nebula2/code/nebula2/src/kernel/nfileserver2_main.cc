//------------------------------------------------------------------------------
//  (C) 2002 RadonLabs GmbH
//------------------------------------------------------------------------------
#include "kernel/nkernelserver.h"
#include "kernel/nenv.h"
#include "kernel/nfileserver2.h"
#include "kernel/nfile.h"
#include "kernel/ndirectory.h"

#if defined(__MACOSX__)
#include <Carbon/carbon.h>
#endif

nNebulaScriptClass(nFileServer2, "nroot");

//------------------------------------------------------------------------------
/**

    history:
     - 30-Jan-2002   peter    created
*/
nFileServer2::nFileServer2() :
    bytesRead(0),
    bytesWritten(0),
    numSeeks(0)
{
    this->assignDir = kernelServer->New("nroot", "/sys/share/assigns");
    this->InitHomeAssign();
    this->InitBinAssign();
}

//------------------------------------------------------------------------------
/**

    history:
     - 30-Jan-2002   peter    created
*/
nFileServer2::~nFileServer2()
{
    if (this->assignDir.isvalid())
    {
        this->assignDir->Release();
    }
}

//------------------------------------------------------------------------------
/**
    creates new or modifies existing assign under /sys/share/assigns

    @param assignName      the name of the assign
    @param pathName        the path to which the assign links

    history:
     - 30-Jan-2002   peter    created
*/
bool
nFileServer2::SetAssign(const char* assignName, const char* pathName)
{
    if (pathName[strlen(pathName)-1] != '/') 
    {
        n_error("nFileServer2::SetAssign: Path '%s' must end with a '/'\n", pathName);
        return false;
    }
        
    // ex. das Assign schon?
    kernelServer->PushCwd(this->assignDir.get());
    nEnv *env = (nEnv *) this->assignDir->Find(assignName);
    if (!env) 
    {
        env = (nEnv *) kernelServer->New("nenv", assignName);
        n_assert(env);
    }
    env->SetS(pathName);
    kernelServer->PopCwd();
    return true;
}

//------------------------------------------------------------------------------
/**
    queries existing assign under /sys/share/assigns

    @param assignName      the name of the assign
    @return                the path to which the assign links

    history:
     - 30-Jan-2002   peter    created
*/
const char*
nFileServer2::GetAssign(const char* assignName)
{
    nEnv *env = (nEnv *) this->assignDir->Find(assignName);
    if (env) 
    {
        return env->GetS();
    }
    else 
    {
        return 0;
    }
}

//------------------------------------------------------------------------------
/**
    Cleanup the path name inplace (replace any backslashes with slashes),
    removes a trailing slash if exists, and does a tolower on each char.
*/
void
nFileServer2::CleanupPathName(char* path)
{
    n_assert(path);

    char* ptr = path;
    char c;

#ifdef __XBxX__
    // on xbox replace slashes with backslashes
    while (c = *ptr)
    {
        if (c == '/')
        {
            *ptr = '\\';
        }
        else
        {
            *ptr = tolower(c);
        }
        ptr++;
    }
    // remove trailing slash
    if ((ptr > path) && (*(--ptr) == '\\'))
    {
        *ptr = 0;
    }
#else
    // on all other systems replace backslashes with slashes
    while ((c = *ptr))
    {
        if (c == '\\')
        {
            *ptr = '/';
        }
        else
        {
            *ptr = tolower(c);
        }
        ptr++;
    }
    // remove trailing slash
    if ((ptr > path) && (*(--ptr) == '/'))
    {
        *ptr = 0;
    }
#endif
}

//------------------------------------------------------------------------------
/**
    Expands assign in path to full absolute path, replaces any backslashes
    by slashes, and returns any trainling slash, and makes the path absolute.

    Please note that Nebula does not know the concept of a current working
    directory, thus, all paths MUST be absolute (please note that Nebula
    assigns can be used to create position independent absolute paths).
      
    @param pathName        the path to expand
    @param buf             buffer for result
    @param bufSize         size of the buffer
    @return                result buffer

    history:
     - 30-Jan-2002   peter    created
*/
const char* 
nFileServer2::ManglePath(const char* pathName, char* buf, int bufSize)
{
    char *pathBuf;
    char *colon;
    pathBuf = (char*)n_malloc(bufSize);
    n_strncpy2(pathBuf,pathName, bufSize);
    buf[0] = 0;

    // check for assigns
    while ((colon = strchr(pathBuf,':')))
    {
        *colon++ = 0;
        if (strlen(pathBuf) > 1)
        {
            const char *replace = this->GetAssign(pathBuf);
            if (replace)
            {
                n_strncpy2(buf, replace, bufSize);
                n_strcat(buf, colon, bufSize);
                n_strncpy2(pathBuf, buf, bufSize);
            }
        }
    }
    n_free(pathBuf);
    
    // no assigns, just do a copy.
    if (0 == buf[0])
    {
        n_strncpy2(buf, pathName, bufSize);
    }
    this->CleanupPathName(buf);
    return buf;
}

//------------------------------------------------------------------------------
/**
    creates a new nDirectory object

    @return          the nDirectory object

    history:
     - 30-Jan-2002   peter    created
*/
nDirectory* 
nFileServer2::NewDirectoryObject()
{
    return new nDirectory(this);
}

//------------------------------------------------------------------------------
/**
    creates a new nFile object
  
    @return          the nFile object

    history:
     - 30-Jan-2002   peter    created
*/
nFile*
nFileServer2::NewFileObject()
{
    return new nFile(this);
}

//------------------------------------------------------------------------------
/**
    Initialize Nebula's home directory assign ("home:").
*/
void 
nFileServer2::InitHomeAssign(void)
{
#ifdef __XBxX__
    this->SetAssign("home", "d:/");
#else    
    char buf[N_MAXPATH];
    #if __WIN32__
        // Win32: Check for the NEBULADIR environment variable first,
        // then try to find the nkernel.dll module handle's filename
        // and cut off the last 2 directories
        char* s = getenv("NEBULADIR");
        if (s)
        {
            n_strncpy2(buf,s,sizeof(buf));
        }
        else
        {
            HMODULE hmod = GetModuleHandle("nkernel.dll");
            DWORD res = GetModuleFileName(hmod,buf,sizeof(buf));
            if (res == 0) 
            {
                n_printf("nFileServer::initHomeAssign(): GetModuleFileName() failed!\n");
            }

            // "x\y\bin\win32\xxx.exe" -> "x\y\"
            int i;
            char *p;
            for (i=0; i<3; i++) 
            {
                p = strrchr(buf,'\\');
                n_assert(p);
                p[0] = 0;
            }
        }

        if (strlen(buf) > 0)
        {
            // convert all backslashes to slashes
            char c, *p;
            p = buf;
            while ((c = *p)) 
            {
                if (c == '\\') *p = '/';
                p++;
            }
            // if last char is not a /, append one
            if (buf[strlen(buf)-1] != '/') strcat(buf,"/");
        }
    #elif defined(__LINUX__)
        // under Linux, the NEBULADIR environment variable must be set,
        // otherwise the current working directory will be used
        char *s = getenv("NEBULADIR");
        if (s) 
        {
            n_strncpy2(buf,s,sizeof(buf));
        } 
        else 
        {
            n_error("Env variable NEBULADIR not set! Aborting.");
        }
        // if last char is not a /, append one
        if ((strlen(buf) > 0) && (buf[strlen(buf)-1] != '/'))
        {
            strcat(buf,"/");
        }
    #elif defined(__MACOSX__)
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFURLRef bundleURL = CFBundleCopyBundleURL(mainBundle);
        FSRef bundleFSRef;
        CFURLGetFSRef(bundleURL, &bundleFSRef);
        FSRefMakePath(&bundleFSRef, (unsigned char*)buf, N_MAXPATH);
        // if last char is not a /, append one
        if ((strlen(buf) > 0) && (buf[strlen(buf)-1] != '/'))
        {
            strcat(buf,"/");
        }
    #else
    #error nFileServer::initHomeAssign() not implemented!
    #endif
    
    // finally, set the assign
    this->SetAssign("home", buf);
#endif
}

//------------------------------------------------------------------------------
/**
*/
void 
nFileServer2::InitBinAssign(void)
{
#ifdef __XBxX__
    this->SetAssign("bin", "d:/");
#else
    char buf[N_MAXPATH];

    const char *home_dir = this->GetAssign("home");
    n_assert(home_dir);
    n_strncpy2(buf,home_dir,sizeof(buf));

    #ifdef __WIN32__
        strcat(buf,"bin/win32/");
    #elif defined(__LINUX__)
        strcat(buf,"bin/linux/");
    #elif defined(__MACOSX__)
        strcat(buf, "bin/macosx/");
    #else
    #error nFileServer::initBinAssign() not implemented!
    #endif

    this->SetAssign("bin",buf);
#endif
}

//------------------------------------------------------------------------------
// EOF
//------------------------------------------------------------------------------
