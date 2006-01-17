
#include "GlobalIni.h"

ArchivalData* INI;

static const char* defaultXmlDoc =
				"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
				"<iniroot>"
				"</iniroot>";

ArchivalData::ArchivalData(path filename)
{
	LPSTR pathBuffer = static_cast<LPSTR>(malloc(1024));
	GetCurrentDirectoryA(1024,pathBuffer);
	workingFile = path(pathBuffer,native)/filename;
	free(static_cast<void*>(pathBuffer));
}

