modded class TraderMenu extends UIScriptedMenu
{
	
	
	
	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (m_UiSellTimer > 0)
			m_UiSellTimer -= timeslice;

		if (m_UiBuyTimer > 0)
			m_UiBuyTimer -= timeslice;

		
		if (m_UiUpdateTimer >= 0.05)
		{
			updatePlayerCurrencyAmount();				
			updateItemListboxColors();

			local int row_index = m_ListboxItems.GetSelectedRow();
			if ((m_LastRowIndex != row_index) || (m_LastCategoryCurrentIndex != m_CategorysCurrentIndex))
			{
				m_LastRowIndex = row_index;
				m_LastCategoryCurrentIndex = m_CategorysCurrentIndex;

				string itemType = m_ListboxItemsClassnames.Get(row_index);
				updateItemPreview(itemType);
			}

			PlayerBase player = g_Game.GetPlayer();
			float playerDistanceToTrader = vector.Distance(player.GetPosition(), player.m_Trader_TraderPositions.Get(m_TraderUID));
			if (playerDistanceToTrader > 1.7)
				GetGame().GetUIManager().Back();
			
			if (DayZPlayerImplement.RefreshMenu)
			{
				updateItemListboxContent();		
				m_ListboxItems.SelectRow(row_index);
				DayZPlayerImplement.RefreshMenu = false;
			}

			m_UiUpdateTimer = 0;
		}
		else
		{
			m_UiUpdateTimer = m_UiUpdateTimer + timeslice;
		}

		if (!m_active)
			GetGame().GetUIManager().Back();
		
	}
}