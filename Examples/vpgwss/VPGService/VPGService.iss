; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "VPGService"
#define MyAppVersion "1.0.2.0"
#define MyAppPublisher "SystemFailure"
#define MyAppExeName "VPGService.exe"

;Define if you want to build setup for x64 version
;#define AMD64 

#define Opencv "C:\Programming\3rdParties\opencv330\build\x86\mingw\bin" 
#define Qt "C:\Qt\5.9.1\mingw53_32\bin"

#define Points 5

[Setup]
; Do not change GUID in future version because GUID is used to determine if app is installed in system or not
AppId={{A83934D2-A71A-496B-9C30-D263E1891BBF} 
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppPublisher}\{#MyAppName}
InfoAfterFile=C:\Programming\vpglib\Examples\vpgwss\{#MyAppName}\afterinstall.md
OutputDir=C:\Programming\Releases
OutputBaseFilename=SETUP_{#MyAppName}_v{#MyAppVersion}_{#Points}pts_gcc
;Ask user to restart
;AlwaysRestart=yes
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]

Source: "C:\Programming\vpglib\Examples\vpgwss\build\build-{#MyAppName}-Desktop_Qt_5_9_1_MinGW_32bit-Release\release\{#MyAppName}.exe"; DestDir: "{app}"; Flags: ignoreversion
; OpenCV
Source: "{#Opencv}\libopencv_core330.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Opencv}\libopencv_highgui330.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Opencv}\libopencv_imgproc330.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Opencv}\libopencv_objdetect330.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Opencv}\libopencv_video330.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Opencv}\libopencv_videoio330.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Opencv}\libopencv_imgcodecs330.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Opencv}\libopencv_ml330.dll"; DestDir: "{app}"; Flags: ignoreversion
;Qt
Source: "{#Qt}\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Qt}\Qt5Concurrent.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Qt}\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Qt}\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Qt}\Qt5WebSockets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Qt}\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Qt}\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Qt}\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Qt}\..\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
;OpenSSL binaries
Source: "C:\Programming\3rdParties\OpenSSL\openssl-1.0.2n-i386-win32\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Programming\3rdParties\OpenSSL\openssl-1.0.2n-i386-win32\ssleay32.dll"; DestDir: "{app}"; Flags: ignoreversion
;After installation we should register service in system
Source: "C:\Programming\vpglib\Examples\vpgwss\{#MyAppName}\regservice.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Programming\vpglib\Examples\vpgwss\{#MyAppName}\delservice.bat"; DestDir: "{app}"; Flags: ignoreversion
;Resources
Source: "C:\Programming\vpglib\Examples\vpgwss\build\build-{#MyAppName}-Desktop_Qt_5_9_1_MinGW_32bit-Release\release\haarcascade_frontalface_alt2.xml"; DestDir: "{app}"; Flags: ignoreversion; Attribs: hidden
Source: "C:\Programming\vpglib\Examples\vpgwss\build\build-{#MyAppName}-Desktop_Qt_5_9_1_MinGW_32bit-Release\release\shape_predictor_{#Points}_face_landmarks.dat"; DestDir: "{app}"; Flags: ignoreversion; Attribs: hidden
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
;Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
;Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
;Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
;Add SSL certiifcate to the system
;Filename: "{tmp}\install_sslcrt.bat"
;Register service in system and run
Filename: "{app}\regservice.bat"

[UninstallRun]
;Delete service from the system
Filename: "{app}\delservice.bat"

[Code]
//-------------------------------------------------------------------------------------
// Delete service if it had been installed before
//-------------------------------------------------------------------------------------
function GetUninstallString: string;
var
  sUnInstPath: string;
  sUnInstallString: String;
begin
  Result := '';
  sUnInstPath := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{{A83934D2-A71A-496B-9C30-D263E1891BBF}_is1'); //Your App GUID/ID
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  Result := sUnInstallString;
end;

function IsUpgrade: Boolean;
begin
  Result := (GetUninstallString() <> '');
end;

function InitializeSetup: Boolean;
var
  V: Integer;
  iResultCode: Integer;
  sUnInstallString: string;
begin
  Result := True; // in case when no previous version is found
  if RegValueExists(HKEY_LOCAL_MACHINE,'Software\Microsoft\Windows\CurrentVersion\Uninstall\{A83934D2-A71A-496B-9C30-D263E1891BBF}_is1', 'UninstallString') then  //Your App GUID/ID
  begin
    V := MsgBox(ExpandConstant('An old version of app was detected. You should uninstall it before. Uninstall now?'), mbInformation, MB_YESNO); //Custom Message if App installed
    if V = IDYES then
    begin
      sUnInstallString := GetUninstallString();
      sUnInstallString :=  RemoveQuotes(sUnInstallString);
      Exec(ExpandConstant(sUnInstallString), '', '', SW_SHOW, ewWaitUntilTerminated, iResultCode);
      Result := True; //if you want to proceed after uninstall
      //Exit; //if you want to quit after uninstall
    end
    else
      Result := False; //when older version present and not uninstalled
  end;
end;