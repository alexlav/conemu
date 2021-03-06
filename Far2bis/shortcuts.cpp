/*
shortcuts.cpp

Folder shortcuts
*/
/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "shortcuts.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "filelist.hpp"
#include "registry.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "interf.hpp"
#include "dialog.hpp"
#include "FarDlgBuilder.hpp"

#include "plugins.hpp"

enum PSCR_RECTYPE
{
	PSCR_RT_SHORTCUT,
	PSCR_RT_PLUGINMODULE,
	PSCR_RT_PLUGINFILE,
	PSCR_RT_PLUGINDATA,
};

static const wchar_t *RecTypeName[]=
{
	L"Shortcut",
	L"PluginModule",
	L"PluginFile",
	L"PluginData",
};

static const wchar_t* FolderShortcutsKeyTest = L"Shortcuts";
static const wchar_t* FolderShortcutsKey = L"Shortcuts\\";
static const wchar_t* OldFolderShortcutsKey = L"FolderShortcuts";
static const wchar_t* HelpFolderShortcuts = L"FolderShortcuts";


Shortcuts::Shortcuts()
{
	HKEY hNewKey = OpenRegKey(FolderShortcutsKeyTest);
	if (hNewKey == NULL)
	{
		for(size_t i = 0; i < KeyCount; i++)
		{
			FormatString ValueName;
			ValueName << RecTypeName[PSCR_RT_SHORTCUT] << i;
			string strValue;
			if(!GetRegKey(OldFolderShortcutsKey, ValueName, strValue, L""))
				continue;
			ValueName.Clear();
			ShortcutItem* Item = Items[i].Push();
			Item->strFolder = strValue;
			ValueName << RecTypeName[PSCR_RT_PLUGINMODULE] << i;
			GetRegKey(OldFolderShortcutsKey, ValueName, Item->strPluginModule, L"");
			ValueName.Clear();
			ValueName << RecTypeName[PSCR_RT_PLUGINFILE] << i;
			GetRegKey(OldFolderShortcutsKey, ValueName, Item->strPluginFile, L"");
			ValueName.Clear();
			ValueName << RecTypeName[PSCR_RT_PLUGINDATA] << i;
			GetRegKey(OldFolderShortcutsKey, ValueName, Item->strPluginData, L"");
			ValueName.Clear();
		}
		return;
	}
	
	for(size_t i = 0; i < KeyCount; i++)
	{
		FormatString strFolderShortcuts;
		strFolderShortcuts << FolderShortcutsKey << i;
		if (!CheckRegKey(strFolderShortcuts))
			continue;

		for(size_t j=0; ; j++)
		{
			FormatString ValueName;
			ValueName << RecTypeName[PSCR_RT_SHORTCUT] << j;
			string strValue;
			if(!GetRegKey(strFolderShortcuts, ValueName, strValue, L""))
				break;
			ValueName.Clear();
			ShortcutItem* Item = Items[i].Push();
			Item->strFolder = strValue;
			ValueName << RecTypeName[PSCR_RT_PLUGINMODULE] << j;
			GetRegKey(strFolderShortcuts, ValueName, Item->strPluginModule, L"");
			ValueName.Clear();
			ValueName << RecTypeName[PSCR_RT_PLUGINFILE] << j;
			GetRegKey(strFolderShortcuts, ValueName, Item->strPluginFile, L"");
			ValueName.Clear();
			ValueName << RecTypeName[PSCR_RT_PLUGINDATA] << j;
			GetRegKey(strFolderShortcuts, ValueName, Item->strPluginData, L"");
			ValueName.Clear();
		}
	}
}

Shortcuts::~Shortcuts()
{
	for(size_t i = 0; i < KeyCount; i++)
	{
		FormatString strFolderShortcuts;
		strFolderShortcuts << FolderShortcutsKey << i;

		int index = 0;
		DeleteKeyTree(strFolderShortcuts);

		for(ShortcutItem* j = Items[i].First(); j; j = Items[i].Next(j), index++)
		{
			FormatString ValueName;
			ValueName << RecTypeName[PSCR_RT_SHORTCUT] << index;
			SetRegKey(strFolderShortcuts, ValueName, j->strFolder);
			ValueName.Clear();
			if(!j->strPluginModule.IsEmpty())
			{
				ValueName << RecTypeName[PSCR_RT_PLUGINMODULE] << index;
				SetRegKey(strFolderShortcuts, ValueName, j->strPluginModule);
				ValueName.Clear();
			}
			if(!j->strPluginFile.IsEmpty())
			{
				ValueName << RecTypeName[PSCR_RT_PLUGINFILE] << index;
				SetRegKey(strFolderShortcuts, ValueName, j->strPluginFile);
				ValueName.Clear();
			}
			if(!j->strPluginData.IsEmpty())
			{
				ValueName << RecTypeName[PSCR_RT_PLUGINDATA] << index;
				SetRegKey(strFolderShortcuts, ValueName, j->strPluginData);
				ValueName.Clear();
			}
		}
	}
}

bool Shortcuts::Get(size_t Pos, string* Folder, string* PluginModule, string* PluginFile, string* PluginData)
{
	bool Result = false;
	if(!Items[Pos].Empty())
	{
		ShortcutItem* RetItem = nullptr;
		if(Items[Pos].Count()>1)
		{
			VMenu FolderList(MSG(MFolderShortcutsTitle),nullptr,0,ScrY-4);
			FolderList.SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
			FolderList.SetHelp(HelpFolderShortcuts);
			FolderList.SetPosition(-1,-1,0,0);
			FolderList.SetBottomTitle(MSG(MFolderShortcutBottomSub));
			for(ShortcutItem* i = Items[Pos].First(); i; i = Items[Pos].Next(i))
			{
				MenuItemEx ListItem={};
				string strFolderName;
				if(!i->strFolder.IsEmpty())
				{
					strFolderName = i->strFolder;
				}
				else
				{
					strFolderName = MSG(i->strPluginModule.IsEmpty()?MShortcutNone:MShortcutPlugin);
				}
				ListItem.strName = strFolderName;
				ListItem.UserData = (char *)(UINT_PTR)i;
				ListItem.UserDataSize = sizeof(UINT);
				FolderList.AddItem(&ListItem);
			}
			FolderList.Show();
			while (!FolderList.Done())
			{
				DWORD Key=FolderList.ReadInput();
				int ItemPos = FolderList.GetSelectPos();
				ShortcutItem* Item = static_cast<ShortcutItem*>(FolderList.GetUserData(nullptr, 0, ItemPos));
				switch (Key)
				{
				case KEY_NUMDEL:
				case KEY_DEL:
				case KEY_NUMPAD0:
				case KEY_INS:
					{
						if (Key == KEY_INS || Key == KEY_NUMPAD0)
						{
							ShortcutItem* NewItem = Items[Pos].InsertBefore(Item);
							Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
							CtrlObject->CmdLine->GetCurDir(NewItem->strFolder);
							if (ActivePanel->GetMode() == PLUGIN_PANEL)
							{
								OpenPluginInfo Info;
								ActivePanel->GetOpenPluginInfo(&Info);
								string strTemp;
								PluginHandle *ph = (PluginHandle*)ActivePanel->GetPluginHandle();
								NewItem->strPluginModule = ph->pPlugin->GetModuleName();
								NewItem->strPluginFile = Info.HostFile;
								NewItem->strPluginData = Info.ShortcutData;
							}
							MenuItemEx NewMenuItem = {};
							NewMenuItem.strName = NewItem->strFolder;
							NewMenuItem.UserData = (char *)(UINT_PTR) NewItem;
							NewMenuItem.UserDataSize = sizeof(UINT);
							FolderList.AddItem(&NewMenuItem, ItemPos);
							FolderList.SetSelectPos(ItemPos, 1);
						}
						else
						{
							if(!Items[Pos].Empty())
							{
								Items[Pos].Delete(Item);
								FolderList.DeleteItem(FolderList.GetSelectPos());
							}
						}
						FolderList.SetPosition(-1, -1, -1, -1);
						FolderList.SetUpdateRequired(TRUE);
						FolderList.Show();
					}
					break;

				case KEY_F4:
					{
						EditItem(&FolderList, Item, false);
					}
					break;

				default:
					FolderList.ProcessInput();
					break;
				}
			}
			int ExitCode = FolderList.GetExitCode();
			if (ExitCode>=0)
			{
				RetItem = static_cast<ShortcutItem*>(FolderList.GetUserData(nullptr, 0, ExitCode));
			}
		}
		else
		{
			RetItem = Items[Pos].First();
		}

		if(RetItem)
		{
			if(Folder)
			{
				*Folder = RetItem->strFolder;
				apiExpandEnvironmentStrings(*Folder, *Folder);
			}
			if(PluginModule)
			{
				*PluginModule = RetItem->strPluginModule;
			}
			if(PluginFile)
			{
				*PluginFile = RetItem->strPluginFile;
			}
			if(PluginData)
			{
				*PluginData = RetItem->strPluginData;
			}
			Result = true;
		}
	}
	return Result;
}

void Shortcuts::Set(size_t Pos, const wchar_t* Folder, const wchar_t* PluginModule, const wchar_t* PluginFile, const wchar_t* PluginData)
{
	ShortcutItem* Item = Items[Pos].Empty()?Items[Pos].Push():Items[Pos].First();
	Item->strFolder = Folder;
	Item->strPluginModule = PluginModule;
	Item->strPluginFile = PluginFile;
	Item->strPluginData = PluginData;
}

void Shortcuts::Add(size_t Pos, const wchar_t* Folder, const wchar_t* PluginModule, const wchar_t* PluginFile, const wchar_t* PluginData)
{
	ShortcutItem* Item = Items[Pos].Push();
	Item->strFolder = Folder;
	Item->strPluginModule = PluginModule;
	Item->strPluginFile = PluginFile;
	Item->strPluginData = PluginData;
}

void Shortcuts::MakeItemName(size_t Pos, MenuItemEx* MenuItem)
{
	const wchar_t* Ptr = L"";
	if(!Items[Pos].Empty())
	{
		if(!Items[Pos].First()->strFolder.IsEmpty())
		{
			Ptr = Items[Pos].First()->strFolder;
		}
		else
		{
			Ptr = MSG(Items[Pos].First()->strPluginModule.IsEmpty()?MShortcutNone:MShortcutPlugin);
		}
	}

	FormatString fstr;
	fstr << MSG(MRightCtrl) << L"+&" << Pos << L" \x2502 " << Ptr;
	MenuItem->strName = fstr;
	if(Items[Pos].Count() > 1)
	{
		MenuItem->Flags|=MIF_SUBMENU;
	}
	else
	{
		MenuItem->Flags&=~MIF_SUBMENU;
	}
}

void Shortcuts::EditItem(VMenu* Menu, ShortcutItem* Item, bool Root)
{
	string strNewDir = Item->strFolder;
	string strNewPluginModule = Item->strPluginModule;
	string strNewPluginFile = Item->strPluginFile;
	string strNewPluginData = Item->strPluginData;

	DialogBuilder Builder(MFolderShortcutsTitle, HelpFolderShortcuts);
	Builder.AddText(MFSShortcut);
	Builder.AddEditField(&strNewDir, 50, L"FS_Path", DIF_EDITPATH);
	if (!strNewPluginModule.IsEmpty())
	{
		Builder.AddSeparator();
		Builder.AddText(MFSShortcutModule);
		Builder.AddEditField(&strNewPluginModule, 50, nullptr, DIF_READONLY);
		Builder.AddText(MFSShortcutFile);
		Builder.AddEditField(&strNewPluginFile, 50, nullptr, DIF_READONLY);
		Builder.AddText(MFSShortcutData);
		Builder.AddEditField(&strNewPluginData, 50, nullptr, DIF_READONLY);
	}
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Unquote(strNewDir);

		if (!IsLocalRootPath(strNewDir))
			DeleteEndSlash(strNewDir);

		bool Save=true;
		string strTemp(strNewDir);
		apiExpandEnvironmentStrings(strNewDir,strTemp);

		if (apiGetFileAttributes(strTemp) == INVALID_FILE_ATTRIBUTES)
		{
			Save=!Message(MSG_WARNING | MSG_ERRORTYPE, 2, MSG(MError), strNewDir, MSG(MSaveThisShortcut), MSG(MYes), MSG(MNo));
		}

		if (Save)
		{
			Item->strPluginData.Clear();
			Item->strPluginFile.Clear();
			Item->strPluginModule.Clear();
			Item->strFolder = strNewDir;

			MenuItemEx* MenuItem = Menu->GetItemPtr();
			MenuItem->strName = Item->strFolder;
			if(Root)
			{
				MakeItemName(Menu->GetSelectPos(), MenuItem);
			}
			Menu->SetPosition(-1, -1, -1, -1);
			Menu->SetUpdateRequired(TRUE);
			Menu->Show();
		}
	}
}

void Shortcuts::Configure()
{
	VMenu FolderList(MSG(MFolderShortcutsTitle),nullptr,0,ScrY-4);
	FolderList.SetFlags(VMENU_WRAPMODE);
	FolderList.SetHelp(HelpFolderShortcuts);
	FolderList.SetPosition(-1,-1,0,0);
	FolderList.SetBottomTitle(MSG(MFolderShortcutBottom));

	for (int I=0; I < 10; I++)
	{
		MenuItemEx ListItem={};
		MakeItemName(I, &ListItem);
		FolderList.AddItem(&ListItem);
	}

	FolderList.Show();

	while (!FolderList.Done())
	{
		DWORD Key=FolderList.ReadInput();
		int Pos = FolderList.GetSelectPos();
		ShortcutItem* Item = Items[Pos].First();

		switch (Key)
		{
		case KEY_NUMDEL:
		case KEY_DEL:
		case KEY_NUMPAD0:
		case KEY_INS:
		case KEY_SHIFTINS:
		case KEY_SHIFTNUMPAD0:
			{
				MenuItemEx* MenuItem = FolderList.GetItemPtr();
				if (Key == KEY_INS || Key == KEY_NUMPAD0 || Key&KEY_SHIFT)
				{
					if(!Item || Key&KEY_SHIFT)
					{
						Item = Items[Pos].Push();
					}
					Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
					CtrlObject->CmdLine->GetCurDir(Item->strFolder);
					if (ActivePanel->GetMode() == PLUGIN_PANEL)
					{
						OpenPluginInfo Info;
						ActivePanel->GetOpenPluginInfo(&Info);
						string strTemp;
						PluginHandle *ph = (PluginHandle*)ActivePanel->GetPluginHandle();
						Item->strPluginModule = ph->pPlugin->GetModuleName();
						Item->strPluginFile = Info.HostFile;
						Item->strPluginData = Info.ShortcutData;
					}
					MakeItemName(Pos, MenuItem);
				}
				else
				{
					if(Item)
					{
						Items[Pos].Delete(Item);
						MakeItemName(Pos, MenuItem);
					}
				}
				DWORD Flags = MenuItem->Flags;
				MenuItem->Flags = 0;
				FolderList.UpdateItemFlags(FolderList.GetSelectPos(), Flags);
				FolderList.SetPosition(-1, -1, -1, -1);
				FolderList.SetUpdateRequired(TRUE);
				FolderList.Show();
			}
			break;

		case KEY_F4:
			{
				if(!Item)
				{
					Item = Items[Pos].Push();
				}
				if(Items[Pos].Count()>1)
				{
					FolderList.ProcessKey(KEY_ENTER);
				}
				else
				{
					EditItem(&FolderList, Item, true);
				}
			}
			break;

		default:
			FolderList.ProcessInput();
			break;
		}
	}

	FolderList.Hide();

	int ExitCode = FolderList.Modal::GetExitCode();

	if(ExitCode>=0)
	{
		CtrlObject->Cp()->ActivePanel->ExecShortcutFolder(ExitCode);
	}
}
