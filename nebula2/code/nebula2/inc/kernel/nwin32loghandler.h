#if (defined(__WIN32__) && !defined(__XBxX__)) || defined(DOXYGEN)
#ifndef N_WIN32LOGHANDLER_H
#define N_WIN32LOGHANDLER_H
//------------------------------------------------------------------------------
/**
    @class nWin32LogHandler
    @ingroup NebulaLogging

    A log handler class for Win32 apps:
    
    - maintains a log file in the application directory where ALL
      output is recorded
    - creates a message box for nKernelServer::Message() 
      and nKernelServer::Error()
    - Places log files in C:\Documents and Settings\<your username>\
      Local Settings\Application Data\RadonLabs\Nebula2

    (C) 2003 RadonLabs GmbH
*/
#include "kernel/ndefaultloghandler.h"
#include "kernel/nfile.h"
#include "util/nstring.h"
#ifdef __WIN32__
#include "kernel/nshell32wrapper.h"
#endif

//------------------------------------------------------------------------------
class nWin32LogHandler : public nDefaultLogHandler
{
public:
    /// constructor
    nWin32LogHandler(const char* appName, const char* subDir = 0);
    /// destructor
    virtual ~nWin32LogHandler();
    /// open the log handler (called by nKernelServer when necessary)
    virtual bool Open();
    /// close the log handler (called by nKernelServer when necessary)
    virtual void Close();
    /// print a message to the log dump
    virtual void Print(const char* msg, va_list argList);
    /// show an important message (may block the program until the user acks)
    virtual void Message(const char* msg, va_list argList);
    /// show an error message (may block the program until the user acks)
    virtual void Error(const char* msg, va_list argList);

private:
    enum MsgType
    {
        MsgTypeMessage,
        MsgTypeError,
    };

    /// put a message box on screen
    void PutMessageBox(MsgType msgType, const char* msg, va_list argList);

    nString appName;
    nString subDir;
    nFile* logFile;

    #ifdef __WIN32__
    nShell32Wrapper shell32Wrapper;
    #endif
};

//------------------------------------------------------------------------------
#endif
#endif /* defined(__WIN32__) && !defined(__XBxX__) */
