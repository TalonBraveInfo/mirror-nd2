//------------------------------------------------------------------------------
/**
	@page NebulaToolsnviewer nviewer

    nviewer

    Selfcontained viewer application for Nebula.

    <dl>
     <dt>-startup</dt>
       <dd>startup script to run, default is: home:code/contrib/nopengl/bin/startup.tcl</dd>
     <dt>-sceneserver</dt>
       <dd>scene server to use</dd>
     <dt>-scriptserver</dt>
       <dd>script server to use</dd>
     <dt>-input</dt>
       <dd>input binding script to run, default is: home:bin/stdinput.tcl</dd>
     <dt>-view</dt>
       <dd>data to load and view with the default lighting setup</dd>
	 <dt>-stage</dt>
		<dd>the light stage to load, default is: home:bin/stdlight.tcl </dd>
     <dt>-fullscreen</dt>
       <dd>if present, then nviewer will go fullscreen</dd>
	 <dt>-alwaysontop</dt>
		<dd>if present the window will be allways on top</dd>
     <dt>-w</dt>
       <dd>width of window to open (default: 640)</dd>
     <dt>-h</dt>
       <dd>height of window to open (default: 480)</dd>
	 <dt>-x</dt>
		<dd>the x position of the window (default: 0)</dd>
	 <dt>-y</dt>
		<dd>the y position of the window (default: 0)</dd>
	 <dt>-projdir</dt>
	    <dd>the optional project directory (assigns it to the projdir: alias, for use in the user's scripts)</dd>
    </dl>

    nviewer also defines some default input handling:

    @todo Document default inputhandling

    (C) 2002 RadonLabs GmbH
*/
//------------------------------------------------------------------------------
#include "kernel/nkernelserver.h"
#include "tools/nviewerapp.h"

#ifdef __WIN32__
#include "kernel/nwin32loghandler.h"
#include "tools/nwinmaincmdlineargs.h"
#else
#include "tools/ncmdlineargs.h"
#endif

nNebulaUsePackage(nnebula);
nNebulaUsePackage(ndinput8);
nNebulaUsePackage(nopengl);
nNebulaUsePackage(ndshow);
nNebulaUsePackage(ngui);
nNebulaUsePackage(nnetwork);

//------------------------------------------------------------------------------
/*
    Win32 specific method which checks the registry for the Nebula2
    Toolkit's project path. If the reg keys are not found, the 
    routine just returns 0.
*/
#ifdef __WIN32__
const char*
ReadProjRegistryKey()
{
    // read the project directory from the registry
    HKEY hKey;
    LONG err;
    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\RadonLabs\\Toolkit", 0, KEY_QUERY_VALUE, &hKey);
    if (ERROR_SUCCESS != err)
    {
        return 0;
    }
    DWORD keyType;
    static char projectDir[512];
    DWORD projectDirSize = sizeof(projectDir);
    err = RegQueryValueEx(hKey, "project", 0, &keyType, (LPBYTE) &projectDir, &projectDirSize);
    if (ERROR_SUCCESS != err)
    {
        return 0;
    }
    return projectDir;
}
#endif

//------------------------------------------------------------------------------
/*
*/
#ifdef __WIN32__
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    nWinMainCmdLineArgs args(lpCmdLine);
#else
int
main(int argc, const char** argv)
{
    nCmdLineArgs args(argc, argv);
#endif

    const char* scriptserverArg = args.GetStringArg("-scriptserver", "ntclserver");
    const char* sceneserverArg = args.GetStringArg("-sceneserver", 0);
    const char* startupArg = args.GetStringArg("-startup", "home:code/contrib/nopengl/bin/startup.tcl");
    const char* viewArg   = args.GetStringArg("-view", 0);
    const char* stageArg  = args.GetStringArg("-stage", "home:bin/stdlight.tcl");
    const char* inputArg  = args.GetStringArg("-input", "home:bin/stdinput.tcl");
    bool fullscreenArg    = args.GetBoolArg("-fullscreen");
    bool alwaysOnTopArg   = args.GetBoolArg("-alwaysontop");
    bool helpArg           = args.GetBoolArg("-help");
    int xPosArg           = args.GetIntArg("-x", 0);
    int yPosArg           = args.GetIntArg("-y", 0);
    int widthArg          = args.GetIntArg("-w", 640);
    int heightArg         = args.GetIntArg("-h", 480);
    const char* projDir   = args.GetStringArg("-projdir", 0);

    const char* gfxServerClass   = args.GetStringArg("-gfxserver", "nglserver2");
    const char* featureSetArg    = args.GetStringArg("-featureset", 0);

    // If the user needs an explanation, just provide one, and don't do anything else this execution
    if (helpArg)
    {
        printf("(C) 2003 RadonLabs GmbH\n"
               "nviewer - Nebula2 Object Viewer\n"
               "Command line args:\n"
               "------------------\n"
               "-help                   show this help\n"
               "-startup                startup script to run, default is: home:bin/startup.tcl\n"
               "-sceneserver            scene server to use, default is nmrtsceneserver\n"
               "-scriptserver           script server to use\n"
               "-gfxserver              graphics server to use; default is platform dependent"
               "-featureset             Which shader feature set to use; I'm not sure how this is expressed\n"
               "-input                  input binding script to run, default is: home:bin/stdinput.tcl\n"
               "-view                   data to load and view with the lighting setup specified (either -stage or the default)\n"
               "-stage                  light stage to load, default is: home:bin/stdlight.tcl\n"
               "-fullscreen             if present, then nviewer will go fullscreen\n"
               "-alwaysontop            present the window will be allways on top\n"
               "-w                      width of window to open (default: 640)\n"
               "-h                      height of window to open (default: 480)\n"
               "-x                      the x position of the window (default: 0)\n"
               "-y                      y position of the window (default: 0)\n"
               "-projdir                the optional project directory (assigns it to the projdir: alias, for use in the user's scripts)\n");
        return 5;
    }

    // initialize Nebula runtime
    nKernelServer kernelServer;
    #ifdef __WIN32__
        nWin32LogHandler logHandler("nviewer");
        kernelServer.SetLogHandler(&logHandler);
    #endif
    kernelServer.AddPackage(nnebula);
    kernelServer.AddPackage(ndinput8);
	kernelServer.AddPackage(nopengl);
    kernelServer.AddPackage(ndshow);
    kernelServer.AddPackage(ngui);
    kernelServer.AddPackage(nnetwork);

    // initialize a display mode object
    nString title;
    if (viewArg)
    {
        title.Append(viewArg);
        title.Append(" - ");
    }
    title.Append("Nebula2 viewer");
    nDisplayMode2 displayMode;
    displayMode.SetIcon("NebulaIcon");
    displayMode.SetDialogBoxMode(true);
    if (fullscreenArg)
    {
        displayMode.Set(title.Get(), nDisplayMode2::Fullscreen, xPosArg, yPosArg, widthArg, heightArg, false);
    }
    else if (alwaysOnTopArg)
    {
        displayMode.Set(title.Get(), nDisplayMode2::AlwaysOnTop, xPosArg, yPosArg, widthArg, heightArg, false);
    }
    else
    {
        displayMode.Set(title.Get(), nDisplayMode2::Windowed, xPosArg, yPosArg, widthArg, heightArg, false);
    }

    // under Win32 check if we should read the project directory from the registry
    // (registry keys used by the Nebula2 toolkit)
    #ifdef __WIN32__
        if (0 == projDir)
        {
            const char* regProjDir = ReadProjRegistryKey();
            if (regProjDir)
            {
                projDir = regProjDir;
            }
        }
    #endif

    // initialize a viewer app object
    nViewerApp viewerApp;
    viewerApp.SetDisplayMode(displayMode);
    if (gfxServerClass)   viewerApp.SetGfxServerClass(gfxServerClass);
    if (viewArg)          viewerApp.SetSceneFile(viewArg);
    if (projDir)          viewerApp.SetProjDir(projDir);
    if (featureSetArg)
    {
        nGfxServer2::FeatureSet featureSet = nGfxServer2::StringToFeatureSet(featureSetArg);
        viewerApp.SetFeatureSetOverride(featureSet);
    }
    
    viewerApp.SetScriptServerClass(scriptserverArg);
    if (sceneserverArg)   viewerApp.SetSceneServerClass(sceneserverArg);
    viewerApp.SetStartupScript(startupArg);
    viewerApp.SetStageScript(stageArg);
    viewerApp.SetInputScript(inputArg);

    // open and run viewer
    if (viewerApp.Open())
    {
        viewerApp.Run();
        viewerApp.Close();
    }
    return 0;
}

