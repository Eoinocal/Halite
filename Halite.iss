[Files]
Source: bin\msvc-7.1\release\link-static\threading-multi\Halite.exe; DestDir: {app}
[Setup]
AppCopyright=© Eoin O'Callaghan
AppName=Halite
AppVerName=0.1.0.1
RestartIfNeededByRun=false
DefaultDirName={pf}\Halite
[UninstallDelete]
Name: {app}\Halite.exe; Type: filesandordirs
[Icons]
Name: Halite\; Filename: {app}\Halite.exe; IconIndex: 0
