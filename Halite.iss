; -- Halite.iss --

#define MyAppName "Halite"
#define MyAppVerName "Halite 0.2.9"
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
LicenseFile=G:\Develop\C++\Personal\Halite\Boost License.txt
OutputBaseFilename=Halite.0_2_9.setup
Compression=lzma
SolidCompression=true
UninstallDisplayIcon={app}\{#MyAppName}
OutputDir=.\bin

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: associate_torrent; Description: {cm:AssocFileExtension,Halite,.torrent}; GroupDescription: Other tasks:; Flags: unchecked

[Files]
Source: bin\msvc-8.0\release\asynch-exceptions-on\threading-multi\Halite.exe; DestDir: {app}; Flags: ignoreversion 32bit
Source: Readme.txt; DestDir: {app}; Flags: ignoreversion isreadme
Source: G:\Program Files (x86)\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcp80.dll; DestDir: {app}; Flags: sharedfile 32bit; Tasks: ; Languages: 
Source: G:\Program Files (x86)\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcr80.dll; DestDir: {app}; Flags: sharedfile 32bit

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

