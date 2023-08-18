; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Ouroboros 2 Debug Build"
#include "version.txt"
#define MyAppPublisher "Arcadia"
#define MyAppURL "https://www.digipen.com/"
#define SourceAppExeName "Editor.exe"
#define MyAppExeName "Ouroboros_Debug.exe"
#define MyAppAssocName MyAppName + "Engine"
#define MyAppAssocExt ".exe"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{8E669F06-B0F3-4C35-9E63-BC712112402D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={src}\{#MyAppName}
ChangesAssociations=yes
DisableProgramGroupPage=yes
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=../../../Installer
OutputBaseFilename=OuroborosSetup{#MyAppVersion}_Debug
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
;Source: "..\bin\Production-Executable-windows-x86_64\Editor\*"; DestDir: "{app}\Editor"; Flags: ignoreversion recursesubdirs createallsubdirs

;standard files
Source: "{#SourceAppExeName}"; DestDir: "{app}"; DestName: "{#MyAppExeName}"; Flags: ignoreversion

;editor folders
Source: "shaders\*"; DestDir: "{app}\shaders"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "Data\*"; DestDir: "{app}\Data"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "mono\*"; DestDir: "{app}\mono"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "Icons\*"; DestDir: "{app}\Icons"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "tracy_server\*"; DestDir: "{app}\tracy_server"; Flags: ignoreversion recursesubdirs createallsubdirs

;dll
Source:"assimp-vc142-mt.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"fmodL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"SDL2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"discord_game_sdk.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"ScriptCore.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"rttr_core_d.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"dbghelp.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"mono-2.0-sgen.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"PhysXCommon_64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"PhysXCooking_64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"PhysXDevice64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"PhysXFoundation_64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"PhysXGpu_64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"PhysX_64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"SLikeNet_DLL_Debug_x64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source:"steam_api64.dll"; DestDir: "{app}"; Flags: ignoreversion

;settings files                       
Source: "EditorMode.settings"; DestDir: "{app}"; Flags: ignoreversion
Source: "PlayMode.settings"; DestDir: "{app}"; Flags: ignoreversion
Source: "imgui.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "default.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "gamecontrollerdb.txt"; DestDir: "{app}"; Flags: ignoreversion    
Source: "version.txt"; DestDir: "{app}"; Flags: ignoreversion



; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Registry]
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""
Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}\SupportedTypes"; ValueType: string; ValueName: ".myp"; ValueData: ""

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

