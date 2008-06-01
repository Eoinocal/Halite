; -- Halite.iss --

#define MyAppName "Halite"
#define MyAppVerName "Halite 0.3.0.7"
#define MyAppPublisher "BinaryNotions.com"
#define MyAppURL "http://www.binarynotions.com/halite.php"
#define MyAppExeName "Halite.exe"
#define MyAppUrlName "Halite.url"

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppVerName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=LICENSE_1_0.txt
OutputBaseFilename=Halite.0_3_0_7_dev465.setup
Compression=lzma
SolidCompression=true
UninstallDisplayIcon={app}\{#MyAppName}
OutputDir=.\bin

ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: associate_torrent; Description: {cm:AssocFileExtension,Halite,.torrent}; GroupDescription: Other tasks:; Flags: unchecked

[Files]
Source: Release\Halite.exe; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit

Source: lang\bin\Dutch.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\French.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\German.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Italian.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Japanese.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Norwegian.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Polish.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Portuguese.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Russian.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Serbian.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Serbian (Cyrillic).dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Slovenian.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Spanish.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Swedish.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Turkish.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit
Source: lang\bin\Japanese.dll; DestDir: {app}; Check: not Is64BitInstallMode; Flags: ignoreversion 32bit

Source: x64\Release\Halite.exe; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit

Source: lang\bin\x64\Dutch.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\French.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\German.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Italian.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Japanese.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Norwegian.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Polish.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Portuguese.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Russian.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Serbian.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Serbian (Cyrillic).dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Slovenian.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Spanish.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Swedish.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Turkish.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit
Source: lang\bin\x64\Japanese.dll; DestDir: {app}; Check: Is64BitInstallMode; Flags: ignoreversion 64bit

Source: Readme.txt; DestDir: {app}; Flags: ignoreversion isreadme

;Source: G:\Program Files (x86)\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcp80.dll; DestDir: {app}; Flags: sharedfile 32bit; Tasks: ; Languages:
;Source: G:\Program Files (x86)\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcr80.dll; DestDir: {app}; Flags: sharedfile 32bit

[INI]
Filename: {app}\{#MyAppUrlName}; Section: InternetShortcut; Key: URL; String: {#MyAppURL}

[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{#MyAppExeName}
Name: {group}\{cm:ProgramOnTheWeb,{#MyAppName}}; Filename: {app}\{#MyAppUrlName}
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}
Name: {userdesktop}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; WorkingDir: {app}; Tasks: desktopicon
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}; Filename: {app}\{#MyAppExeName}; WorkingDir: {app}; Tasks: quicklaunchicon

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCR; Subkey: .torrent; ValueType: string; ValueName: ; ValueData: Halite; Flags: uninsdeletevalue; Tasks: associate_torrent
Root: HKCR; Subkey: Halite; ValueType: string; ValueName: ; ValueData: Torrent File; Flags: uninsdeletekey; Tasks: associate_torrent
Root: HKCR; Subkey: Halite\DefaultIcon; ValueType: string; ValueName: ; ValueData: {app}\halite.exe,0; Tasks: associate_torrent
Root: HKCR; Subkey: Halite\shell\open\command; ValueType: string; ValueName: ; ValueData: """{app}\halite.exe"" ""%1"""; Tasks: associate_torrent
