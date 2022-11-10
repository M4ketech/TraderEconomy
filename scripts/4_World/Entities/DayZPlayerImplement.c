modded class DayZPlayerImplement
{
	static const string m_Trader_StockDirPath2 = "$profile:Trader/Traderstock/";
		
	static ref array<int> wa_boughtItems = new array<int>;
	static ref array<int> wa_soldItems = new array<int>;
	
	static bool RefreshMenu = false;
		
	override void handleBuyRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param3<int, int, string> rpb = new Param3<int, int, string>( -1, -1, "" );
		ctx.Read(rpb);

		int traderUID = rpb.param1;
		int itemID = rpb.param2;
		itemDisplayNameClient = rpb.param3;

		m_Trader_IsSelling = false;

		if (GetGame().GetTime() - m_Trader_LastBuyedTime < m_Trader_BuySellTimer * 1000)
			return;
		m_Trader_LastBuyedTime = GetGame().GetTime();

		if (itemID >= m_Trader_ItemsClassnames.Count() || itemID < 0 || traderUID >= m_Trader_TraderPositions.Count() || traderUID < 0)
			return;

		string itemType = m_Trader_ItemsClassnames.Get(itemID);
		int itemQuantity = m_Trader_ItemsQuantity.Get(itemID);
		int itemCosts = m_Trader_ItemsBuyValue.Get(itemID);

		vector playerPosition = this.GetPosition();	

		if (vector.Distance(playerPosition, m_Trader_TraderPositions.Get(traderUID)) > 1.7)
		{
			traderServerLog("tried to access the Trader out of Range! This could be an Hacker!");
			return;
		}

		m_Player_CurrencyAmount = getPlayerCurrencyAmount();

		if (itemCosts < 0)
		{
			TraderMessage.PlayerWhite("#tm_cant_be_bought", this);
			return;
		}

		if (m_Player_CurrencyAmount < itemCosts)
		{
			TraderMessage.PlayerWhite("#tm_cant_afford", this);
			return;
		}

		int vehicleKeyHash = 0;

		bool isDuplicatingKey = false;
		if (itemQuantity == -7) // is Key duplication
		{
			VehicleKeyBase vehicleKeyinHands = VehicleKeyBase.Cast(this.GetHumanInventory().GetEntityInHands());

			if (!vehicleKeyinHands)
			{
				TraderMessage.PlayerWhite("Put the Key you\nwant to duplicate\nin your Hands!", this);
				return;
			}

			isDuplicatingKey = true;
			vehicleKeyHash = vehicleKeyinHands.GetHash();
			itemType = vehicleKeyinHands.GetType();
			itemQuantity = 1;
		}

		traderServerLog("bought " + getItemDisplayName(itemType) + "(" + itemType + ")");

		if (itemQuantity == -2 || itemQuantity == -6) // Is a Vehicle
		{
			string blockingObject = isVehicleSpawnFree(traderUID);

			if (blockingObject != "FREE")
			{
				TraderMessage.PlayerWhite(getItemDisplayName(blockingObject) + " " + "#tm_way_blocked", this);
				return;
			}

			if (itemQuantity == -2)
			{
				if (canCreateItemInPlayerInventory("VehicleKeyBase", 1))
				{
					TraderMessage.PlayerWhite(getItemDisplayName("VehicleKey") + "\n " + "#tm_added_to_inventory", this);
					
					vehicleKeyHash = createVehicleKeyInPlayerInventory();
				}
				else
				{
					TraderMessage.PlayerWhite("#tm_inventory_full" + "\n" + getItemDisplayName("VehicleKey") + "\n" + "#tm_was_placed_on_ground", this);
										
					vehicleKeyHash = spawnVehicleKeyOnGround();
					
					GetGame().RPCSingleParam(this, TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>( false ), true, this.GetIdentity());
				}
				//TraderMessage.PlayerWhite("KeyHash:\n" + vehicleKeyHash, this);
			}

			deductPlayerCurrency(itemCosts);

			if (!itemType.Contains("MoneyRuble"))
				wa_boughtItems.Insert(itemID);
			TraderMessage.PlayerWhite("" + itemDisplayNameClient + "\n" + "#tm_parked_next_to_you", this);

			spawnVehicle(traderUID, itemType, vehicleKeyHash);

			GetGame().RPCSingleParam(this, TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>( false ), true, this.GetIdentity());
		}
		else // Is not a Vehicle
		{		
			deductPlayerCurrency(itemCosts);
			
			if (!itemType.Contains("MoneyRuble"))
				wa_boughtItems.Insert(itemID);

			if (canCreateItemInPlayerInventory(itemType, itemQuantity))
			{
				TraderMessage.PlayerWhite("" + itemDisplayNameClient + "\n" + "#tm_added_to_inventory", this);
				
				if (isDuplicatingKey)
					createVehicleKeyInPlayerInventory(vehicleKeyHash, itemType);
				else
					createItemInPlayerInventory(itemType, itemQuantity);
			}
			else
			{
				TraderMessage.PlayerWhite("#tm_inventory_full" + "\n " + itemDisplayNameClient + "\n" + "#tm_was_placed_on_ground", this);
									
				if (isDuplicatingKey)
					spawnVehicleKeyOnGround(vehicleKeyHash, itemType);
				else			
					spawnItemOnGround(itemType, itemQuantity, playerPosition);
				
				GetGame().RPCSingleParam(this, TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>( false ), true, this.GetIdentity());
			}
		}
		//deductPlayerCurrency(itemCosts);
	}
	
	override void handleSellRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param3<int, int, string> rps = new Param3<int, int, string>( -1, -1, "" );
		ctx.Read(rps);

		int traderUID = rps.param1;
		int itemID = rps.param2;
		itemDisplayNameClient = rps.param3;

		m_Trader_IsSelling = true;

		if (GetGame().GetTime() - m_Trader_LastSelledTime < m_Trader_BuySellTimer * 1000)
			return;
		m_Trader_LastSelledTime = GetGame().GetTime();

		if (itemID >= m_Trader_ItemsClassnames.Count() || itemID < 0 || traderUID >= m_Trader_TraderPositions.Count() || traderUID < 0)
			return;

		string itemType = m_Trader_ItemsClassnames.Get(itemID);
		int itemQuantity = m_Trader_ItemsQuantity.Get(itemID);
		int itemSellValue = m_Trader_ItemsSellValue.Get(itemID);

		vector playerPosition = this.GetPosition();	

		if (vector.Distance(playerPosition, m_Trader_TraderPositions.Get(traderUID)) > 1.7)
		{
			traderServerLog("tried to access the Trader out of Range! This could be an Hacker!");
			return;
		}


		Object vehicleToSell = GetVehicleToSell(traderUID, itemType);
		bool isValidVehicle = ((itemQuantity == -2 || itemQuantity == -6) && vehicleToSell);

		if (itemSellValue < 0)
		{
			TraderMessage.PlayerWhite("#tm_cant_be_sold", this);
			return;
		}

		if (!isInPlayerInventory(itemType, itemQuantity) && !isValidVehicle)
		{
			TraderMessage.PlayerWhite("#tm_you_cant_sell", this);

			if (itemQuantity == -2 || itemQuantity == -6)
				TraderMessage.PlayerWhite("#tm_cant_sell_vehicle", this);
				//TraderMessage.PlayerWhite("Turn the Engine on and place it inside the Traffic Cones!", this);

			return;
		}

		traderServerLog("#tm_sold" + " " + getItemDisplayName(itemType) + " (" + itemType + ")");

		TraderMessage.PlayerWhite("" + itemDisplayNameClient + "\n" + "#tm_was_sold", this);

		if (isValidVehicle)
			deleteObject(vehicleToSell);
		else
			removeFromPlayerInventory(itemType, itemQuantity);
		
		increasePlayerCurrency(itemSellValue);
		
		if (!itemType.Contains("MoneyRuble"))
				wa_soldItems.Insert(itemID);
	}
	
	override void handleClientRPCs(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.handleClientRPCs(sender, rpc_type, ctx);
		
		switch(rpc_type)
			{
				case TPRPCs.RPC_SEND_TRADER_PRICES_UPDATE:
					Param3<int, int, int> itemPrices_rp = new Param3<int, int, int>( 0, 0, 0);
					ctx.Read( itemPrices_rp );
					
					m_Trader_ItemsBuyValue[itemPrices_rp.param3] = itemPrices_rp.param1;
					m_Trader_ItemsSellValue[itemPrices_rp.param3] = itemPrices_rp.param2;
				break;
				
				case TPRPCs.RPC_SEND_TRADER_PRICES_UPDATE_REFRESH:
					RefreshMenu = true;
				break;
			}
	}

}