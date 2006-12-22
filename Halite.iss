; -- Halite.iss --

#define MyAppName "Halite"
#define MyAppVerName "Halite 0.2.8"
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
LicenseFile=G:\Develop\C++\Personal\Halite\LICENSE_1_0.txt
OutputBaseFilename=Halite.0_2_8.setup
Compression=lzma
SolidCompression=true
UninstallDisplayIcon={app}\{#MyAppName}
OutputDir=.\bin

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: associate_torrent; Description: {cm:AssocFileExtension, Halite, .torrent}; GroupDescription: Other tasks:; Flags: unchecked

[Files]
Source: bin\msvc-7.1\release\asynch-exceptions-on\runtime-link-static\threading-multi\Halite.exe; DestDir: {app}; Flags: ignoreversion 32bit
Source: Readme.txt; DestDir: {app}; Flags: ignoreversion isreadme

[INI]
Filename: {app}\{#MyAppUrlName}; Section: InternetShortcut; Key: URL; String: {#MyAppURL}

[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{#MyAppExeName}
Name: {group}\{cm:ProgramOnTheWeb,{#MyAppName}}; Filename: {app}\{#MyAppUrlName}
Name: {group}\{cm:UninstallProgram,{#MyAppName}}; Filename: {uninstallexe}
Name: {userdesktop}\{#MyAppName}; Filename: {app}\{#MyAppExeName}; Tasks: desktopicon
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}; Filename: {app}\{#MyAppExeName}; Tasks: quicklaunchicon

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCR; Subkey: .torrent; ValueType: string; ValueName: ; ValueData: Halite; Flags: uninsdeletevalue; Tasks: associate_torrent
Root: HKCR; Subkey: Halite; ValueType: string; ValueName: ; ValueData: Torrent File; Flags: uninsdeletekey; Tasks: associate_torrent
Root: HKCR; Subkey: Halite\DefaultIcon; ValueType: string; ValueName: ; ValueData: {app}\halite.exe,0; Tasks: associate_torrent
Root: HKCR; Subkey: Halite\shell\open\command; ValueType: string; ValueName: ; ValueData: """{app}\halite.exe"" ""%1"""; Tasks: associate_torrent

[UninstallDelete]
Type: files; Name: {app}\{#MyAppUrlName}
