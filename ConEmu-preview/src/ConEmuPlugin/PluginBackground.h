
#pragma once

class CPluginBackground
{
	protected:
		struct RegisterBackgroundArg *mp_BgPlugins;
		int mn_BgPluginsCount, mn_BgPluginsMax;
		BOOL mb_BgWasSent;
		BOOL mb_BgErrorShown;
		MSection *csBgPlugins;
		DWORD m_LastThSetCheck;
		BOOL mb_ThNeedLoad;
		PanelViewSetMapping m_ThSet;
		struct PaintBackgroundArg m_Default, m_Last;
		bool IsParmChanged(struct PaintBackgroundArg* p1, struct PaintBackgroundArg* p2);
		//// Buffers
		//wchar_t ms_LeftCurDir[32768], ms_LeftFormat[MAX_PATH], ms_LeftHostFile[32768];
		//wchar_t ms_RightCurDir[32768], ms_RightFormat[MAX_PATH], ms_RightHostFile[32768];

		// bitmask values
		enum RequiredActions
		{
			ra_UpdateBackground = 1,
			ra_CheckPanelFolders = 2,
			//rs_ForceUpdate = 4,
		};
		DWORD mn_ReqActions;

		void ReallocItems(int nAddCount);
		BOOL LoadThSet(BOOL abFromMainThread);
		void SetDcPanelRect(RECT *rcDc, PaintBackgroundArg::BkPanelInfo *Panel, PaintBackgroundArg *Arg);

		/* ���������� ������ � thread-safe (Synchro) - begin */
		void CheckPanelFolders(int anForceSetPlace = 0);
		void UpdateBackground();
		static void UpdateBackground_Exec(struct RegisterBackgroundArg *pPlugin, struct PaintBackgroundArg *pArg);
		/* end- ���������� ������ � thread-safe (Synchro)*/

	public:
		CPluginBackground();
		~CPluginBackground();

		void SetForceCheck();
		void SetForceUpdate(bool bFlagsOnly = false);
		void SetForceThLoad();

		// ����� ���������� � ������������ ������
		int RegisterSubplugin(RegisterBackgroundArg *pbk);

		// ���������� ������ � thread-safe (Synchro)
		void OnMainThreadActivated(int anEditorEvent = -1, int anViewerEvent = -1);

		// ���������� �� �������� ������
		void MonitorBackground();
};

extern BOOL gbBgPluginsAllowed; // TRUE ����� ExitFar
extern BOOL gbNeedBgActivate; // ��������� ��������� ������ � ������� ����
extern CPluginBackground *gpBgPlugin;
