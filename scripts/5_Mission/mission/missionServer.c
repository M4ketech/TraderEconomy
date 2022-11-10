modded class MissionServer
{
	static const string m_Trader_ConfigFilePath2 = "$profile:Trader/TraderConfig2.txt";
	static const string m_Trader_StockDirPath = "$profile:Trader/Traderstock/";
	static string m_Trader_TypesXMLPath = "$CurrentDir:mpmissions/dayzOffline.chernarusplus/db/types.xml";
	
	bool ShowDebugRestock = false;
	
	bool SendTraderPrices;
	float m_Trader_Taxes = 1;
	bool ReplaceSellPrices = false;
	bool TraderDBInit = false;
	bool m_Trader_AutoCalculateRestockTimer = true;
	int MinBalancedStock = 10;
	int MaxBalancedStock = 1000;
	float BalancedStockMultiplier = 1;
	float RestockPercent = 0;
	float m_Trader_RestockUpdateTimer = 0;
	float m_Trader_RestockUpdateTimerMax = 6000;
	float m_Trader_DefaultRestockTime = 86400;
	float m_Trader_RestockTimeSum = 0;
	int LowestRestockTimer = 86400;
	int HighestRestockTimer = 86400;
	float NextRestock = 0;
	
	ref array<string> TraderDB_Classnames = new array<string>;
	ref array<int> TraderDB_Nominal = new array<int>;
	ref array<int> m_Trader_Restock_Times = new array<int>;
	ref array<int> m_Trader_Restock_moddedTimes = new array<int>;
	ref array<float> m_Trader_Restock_Percents = new array<float>;
	ref array<string> m_Trader_Economy_DefaultItemValues = new array<string>;
	ref array<int> m_Trader_Restock_moddedTimes_UniqueArray = new array<int>;
	
	
	
	//ref array<int> itemData;
	
	override void OnInit()
	{		
		super.OnInit();
				
		if (ReplaceSellPrices)
			RemodTraderConfig();
		LoadTimeStamp();
	}
	
	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		
		if (DayZPlayerImplement.wa_boughtItems.Count() > 0)
			OnItemBought();
		if (DayZPlayerImplement.wa_soldItems.Count() > 0)
			OnItemSold();
		
		if (m_Trader_RestockUpdateTimerMax > 0)
		{
			m_Trader_RestockUpdateTimer += timeslice;
			if (m_Trader_RestockUpdateTimer >= m_Trader_RestockUpdateTimerMax)
			{
				//TraderMessage.ServerLog("[TRADER] DEBUG2: (m_Trader_RestockUpdateTimerMax > m_Trader_RestockUpdateTimerMax) = true");
				RestockTraderNew(m_Trader_RestockUpdateTimer);
				m_Trader_RestockUpdateTimer = 0;
			}
		}
	}
	
	
	override void readTraderVariables()
	{
		TraderMessage.ServerLog("[TRADER] DEBUG START");

		FileHandle file_index = OpenFile(m_Trader_VariableFilePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO TRADERVARIABLE FILE!");
			return;
		}

		int variableCounter = 0;
		
		string line_content = "";
		while (variableCounter <= 500 && !line_content.Contains("<FileEnd>"))
		{
			bool validEntry = false;

			line_content = "";
			int char_count = FGets( file_index,  line_content );
			line_content = FileReadHelper.TrimComment(line_content);

			if (line_content.Contains("<BuySellTimer>"))
			{
				line_content.Replace("<BuySellTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_BuySellTimer = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] BUYSELLTIMER = " + line_content);
			}
			
			if (line_content.Contains("<StatUpdateTimer>"))
			{
				line_content.Replace("<StatUpdateTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_StatUpdateTimeMax = line_content.ToFloat();
				m_Trader_StatUpdateTime = m_Trader_StatUpdateTimeMax;
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] STATUPDATETIMER = " + line_content);
			}

			if (line_content.Contains("<FireBarrelUpdateTimer>"))
			{
				line_content.Replace("<FireBarrelUpdateTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_SpawnedFireBarrelsUpdateTimerMax = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] FIREBARRELUPDATETIMER = " + line_content);
			}

			if (line_content.Contains("<ZombieCleanupTimer>"))
			{
				line_content.Replace("<ZombieCleanupTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_ZombieCleanupUpdateTimerMax = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] ZOMBIECLEANUPTIMER = " + line_content);
			}

			if (line_content.Contains("<VehicleCleanupTimer>"))
			{
				line_content.Replace("<VehicleCleanupTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_VehicleCleanupUpdateTimerMax = line_content.ToFloat() * 60;
				m_Trader_VehicleCleanupUpdateTimer = m_Trader_VehicleCleanupUpdateTimerMax;
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] VEHICLECLEANUPTIMER = " + line_content);
			}

			if (line_content.Contains("<SafezoneTimeout>"))
			{
				line_content.Replace("<SafezoneTimeout>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_SafezoneTimeout = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] SAFEZONETIMEOUT = " + line_content);
			}
				
			if (line_content.Contains("<TraderTaxes>"))
			{
				line_content.Replace("<TraderTaxes>", "");
				line_content = FileReadHelper.TrimComment(line_content);
					
				m_Trader_Taxes = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] TAXES = " + line_content);
			}
			
			if (line_content.Contains("<ReplaceSellPrices>"))
			{
				line_content.Replace("<ReplaceSellPrices>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				if (line_content.Contains("true"))
					ReplaceSellPrices = true;
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] ReplaceSellPrices = <" + line_content + ">");
			}
			
			if (line_content.Contains("<MinBalancedStock>"))
			{
				line_content.Replace("<MinBalancedStock>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				MinBalancedStock = line_content.ToInt();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] MinBalancedStock = <" + line_content + ">");
			}
			
			if (line_content.Contains("<MaxBalancedStock>"))
			{
				line_content.Replace("<MaxBalancedStock>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				MaxBalancedStock = line_content.ToInt();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] MaxBalancedStock = <" + line_content + ">");
			}
			
			if (line_content.Contains("<BalancedStockMultiplier>"))
			{
				line_content.Replace("<BalancedStockMultiplier>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				BalancedStockMultiplier = line_content.ToFloat();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] BalancedStockMultiplier = <" + line_content + ">");
			}
			
			if (line_content.Contains("<PathToTypesXml>"))
			{
				line_content.Replace("<PathToTypesXml>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				m_Trader_TypesXMLPath = "$CurrentDir:" + line_content;
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] PathToTypesXml = <" + line_content + ">");
			}
			
			if (line_content.Contains("<RestockUpdateTimer>"))
			{
				line_content.Replace("<RestockUpdateTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				if (line_content.Contains("A") || line_content.Contains("a"))
					m_Trader_AutoCalculateRestockTimer = true;
				else
				{
					m_Trader_RestockUpdateTimerMax = line_content.ToFloat();
					m_Trader_AutoCalculateRestockTimer = false;
				}
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] RestockUpdateTimer = <" + line_content + ">");
			}
			
			if (line_content.Contains("<RestockPercent>"))
			{
				line_content.Replace("<RestockPercent>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				RestockPercent = line_content.ToFloat();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] RestockPercent = <" + line_content + ">");
			}
			
			if (line_content.Contains("<DefaultRestockTime>"))
			{
				line_content.Replace("<DefaultRestockTime>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				m_Trader_DefaultRestockTime = line_content.ToInt();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] DefaultRestockTime = <" + line_content + ">");
			}
			
			
			
			if (validEntry)
				variableCounter++;
		}

		CloseFile(file_index);

		TraderMessage.ServerLog("[TRADER] DONE END!");
	}

	override void readTraderData()
	{		
		// clear all data here:
		m_Trader_ReadAllTraderData = false;	
		m_Trader_CurrencyName = "";
		m_Trader_CurrencyClassnames = new array<string>;
		m_Trader_CurrencyValues = new array<int>;
		m_Trader_TraderNames = new array<string>;
		m_Trader_TraderPositions = new array<vector>;
		m_Trader_TraderIDs = new array<int>;
		m_Trader_TraderSafezones = new array<int>;
		m_Trader_TraderVehicleSpawns = new array<vector>;
		m_Trader_TraderVehicleSpawnsOrientation = new array<vector>;
		m_Trader_Categorys = new array<string>;
		m_Trader_CategorysTraderKey = new array<int>;
		m_Trader_ItemsTraderId = new array<int>;
		m_Trader_ItemsCategoryId = new array<int>;
		m_Trader_ItemsClassnames = new array<string>;
		m_Trader_ItemsQuantity = new array<int>;
		m_Trader_ItemsBuyValue = new array<int>;
		m_Trader_ItemsSellValue = new array<int>;
		m_Trader_Vehicles = new array<string>;
		m_Trader_VehiclesParts = new array<string>;
		m_Trader_VehiclesPartsVehicleId = new array<int>;
			
		TraderMessage.ServerLog("[TRADER] DEBUG START");

		FileHandle file_index = OpenFile(m_Trader_VariableFilePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO TRADERVARIABLE FILE!");
			return;
		}

		int variableCounter = 0;
		
		string line_content = "";
		while (variableCounter <= 500 && !line_content.Contains("<FileEnd>"))
		{
			bool validEntry = false;

			line_content = "";
			int char_count = FGets( file_index,  line_content );
			line_content = FileReadHelper.TrimComment(line_content);

			if (line_content.Contains("<BuySellTimer>"))
			{
				line_content.Replace("<BuySellTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_BuySellTimer = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] BUYSELLTIMER = " + line_content);
			}
			
			if (line_content.Contains("<StatUpdateTimer>"))
			{
				line_content.Replace("<StatUpdateTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_StatUpdateTimeMax = line_content.ToFloat();
				m_Trader_StatUpdateTime = m_Trader_StatUpdateTimeMax;
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] STATUPDATETIMER = " + line_content);
			}

			if (line_content.Contains("<FireBarrelUpdateTimer>"))
			{
				line_content.Replace("<FireBarrelUpdateTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_SpawnedFireBarrelsUpdateTimerMax = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] FIREBARRELUPDATETIMER = " + line_content);
			}

			if (line_content.Contains("<ZombieCleanupTimer>"))
			{
				line_content.Replace("<ZombieCleanupTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_ZombieCleanupUpdateTimerMax = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] ZOMBIECLEANUPTIMER = " + line_content);
			}

			if (line_content.Contains("<VehicleCleanupTimer>"))
			{
				line_content.Replace("<VehicleCleanupTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_VehicleCleanupUpdateTimerMax = line_content.ToFloat() * 60;
				m_Trader_VehicleCleanupUpdateTimer = m_Trader_VehicleCleanupUpdateTimerMax;
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] VEHICLECLEANUPTIMER = " + line_content);
			}

			if (line_content.Contains("<SafezoneTimeout>"))
			{
				line_content.Replace("<SafezoneTimeout>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_SafezoneTimeout = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] SAFEZONETIMEOUT = " + line_content);
			}
				
			if (line_content.Contains("<TraderTaxes>"))
			{
				line_content.Replace("<TraderTaxes>", "");
				line_content = FileReadHelper.TrimComment(line_content);
					
				m_Trader_Taxes = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] TAXES = " + line_content);
			}
			
			if (line_content.Contains("<ReplaceSellPrices>"))
			{
				line_content.Replace("<ReplaceSellPrices>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				if (line_content.Contains("true"))
					ReplaceSellPrices = true;
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] ReplaceSellPrices = <" + line_content + ">");
			}
			
			if (line_content.Contains("<MinBalancedStock>"))
			{
				line_content.Replace("<MinBalancedStock>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				MinBalancedStock = line_content.ToInt();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] MinBalancedStock = <" + line_content + ">");
			}
			
			if (line_content.Contains("<MaxBalancedStock>"))
			{
				line_content.Replace("<MaxBalancedStock>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				MaxBalancedStock = line_content.ToInt();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] MaxBalancedStock = <" + line_content + ">");
			}
			
			if (line_content.Contains("<BalancedStockMultiplier>"))
			{
				line_content.Replace("<BalancedStockMultiplier>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				BalancedStockMultiplier = line_content.ToFloat();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] BalancedStockMultiplier = <" + line_content + ">");
			}
			
			if (line_content.Contains("<PathToTypesXml>"))
			{
				line_content.Replace("<PathToTypesXml>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				m_Trader_TypesXMLPath = "$CurrentDir:" + line_content;
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] PathToTypesXml = <" + line_content + ">");
			}
			
			if (line_content.Contains("<RestockUpdateTimer>"))
			{
				line_content.Replace("<RestockUpdateTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				if (line_content.Contains("A") || line_content.Contains("a"))
					m_Trader_AutoCalculateRestockTimer = true;
				else
				{
					m_Trader_RestockUpdateTimerMax = line_content.ToFloat();
					m_Trader_AutoCalculateRestockTimer = false;
				}
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] RestockUpdateTimer = <" + line_content + ">");
			}
			
			if (line_content.Contains("<RestockPercent>"))
			{
				line_content.Replace("<RestockPercent>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				RestockPercent = line_content.ToFloat();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] RestockPercent = <" + line_content + ">");
			}
			
			if (line_content.Contains("<DefaultRestockTime>"))
			{
				line_content.Replace("<DefaultRestockTime>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				m_Trader_DefaultRestockTime = line_content.ToInt();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] DefaultRestockTime = <" + line_content + ">");
			}
			
			
			
			if (validEntry)
				variableCounter++;
		}
		CloseFile(file_index);		
		
		m_Trader_Economy_DefaultItemValues = new array<string>;
		m_Trader_Economy_DefaultItemValues.Insert("0");
		m_Trader_Economy_DefaultItemValues.Insert(MinBalancedStock.ToString());
		m_Trader_Economy_DefaultItemValues.Insert("-1");
		m_Trader_Economy_DefaultItemValues.Insert("-1");
		m_Trader_Economy_DefaultItemValues.Insert("-1");
		m_Trader_Economy_DefaultItemValues.Insert("0");
		m_Trader_Economy_DefaultItemValues.Insert("0");
		m_Trader_Economy_DefaultItemValues.Insert("0.00");
		
		file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO TRADERCONFIG FILE!");
			return;
		}
		
		line_content = "";
		
		TraderMessage.ServerLog("[TRADER] READING CURRENCY NAME ENTRY..");
		line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<CurrencyName>", "");
		line_content.Replace("<CurrencyName>", "");
		line_content = FileReadHelper.TrimComment(line_content);
		m_Trader_CurrencyName = line_content;

		int currencyCounter = 0;

		line_content = "";
		while (currencyCounter <= 500 && !line_content.Contains("<Trader>"))
		{
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<Currency>", "<Trader>");
			line_content.Replace("<Currency>", "");
			line_content = FileReadHelper.TrimComment(line_content);

			if (line_content.Contains("<Trader>"))
				break;

			//TraderMessage.ServerLog("[TRADER] READING CURRENCY ENTRY..");

			TStringArray crys = new TStringArray;
			line_content.Split( ",", crys );

			string currencyClassname = crys.Get(0);
			currencyClassname = FileReadHelper.TrimSpaces(currencyClassname);
			
			string currencyValue = crys.Get(1);
			currencyValue = FileReadHelper.TrimSpaces(currencyValue);

			m_Trader_CurrencyClassnames.Insert(currencyClassname);
			m_Trader_CurrencyValues.Insert(currencyValue.ToInt());
		}

		bool traderInstanceDone = true;
		int traderCounter = 0;
		
		//line_content = "";
		while (traderCounter <= 5000 && line_content != "<FileEnd>")
		{
			//TraderMessage.ServerLog("[TRADER] READING TRADER ENTRY..");
			
			if (traderInstanceDone == false)
				line_content = FileReadHelper.SearchForNextTermsInFile(file_index, {"<Trader>", "<OpenFile>"}, "");
			else
				traderInstanceDone = false;
			
			if (line_content.Contains("<OpenFile>"))
			{
				if (OpenNewFileForReading(line_content, file_index))
					continue;
				else
					return;
			}

			line_content.Replace("<Trader>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			m_Trader_TraderNames.Insert(line_content);
				
			int categoryCounter = 0;
			
			line_content = "";
			while (categoryCounter <= 5000 && line_content != "<FileEnd>")
			{
				line_content = FileReadHelper.TrimComment(FileReadHelper.SearchForNextTermsInFile(file_index, {"<Category>", "<OpenFile>"}, "<Trader>"));
				
				if (line_content.Contains("<OpenFile>"))
				{
					if (OpenNewFileForReading(line_content, file_index))
						continue;
					else
						return;
				}

				if (line_content.Contains("<Trader>"))
				{
					traderInstanceDone = true;
					break;
				}
				
				if (line_content == string.Empty)
				{
					line_content = "<FileEnd>";
					break;
				}
				
				//TraderMessage.ServerLog("[TRADER] READING CATEGORY ENTRY..");
				line_content.Replace("<Category>", "");
				m_Trader_Categorys.Insert(FileReadHelper.TrimComment(line_content));
				m_Trader_CategorysTraderKey.Insert(traderCounter);
				
				categoryCounter++;
			}
			
			traderCounter++;
		}
		
		CloseFile(file_index);
		
		//------------------------------------------------------------------------------------
		
		file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
		
		int itemCounter = 0;
		int itemCounterTotal = 0;
		char_count = 0;
		int traderID = -1;
		int categoryId = -1;
		
		int Stock;
		int Nominal;
		string BasePrice;
		string BaseSellPrice;
		int RealNominal;
		int excludeFromRestock;
		int ItemRestockTimer;
		float ItemRestockPercent;
		
		bool overwrite;
		float fRealNominal;
		int modifiedNominal;
		int modifiedItemRestockTimer;
		
		
		line_content = "";
		while (char_count != -1 && !line_content.Contains("<FileEnd>"))
		{	
			char_count = FGets( file_index,  line_content );
			line_content = FileReadHelper.TrimComment(line_content);
			
			overwrite = false;
			ItemRestockTimer = 0;
			ItemRestockPercent = 0.00;
			excludeFromRestock = 0;	

			if (line_content.Contains("<OpenFile>"))
			{
				if (OpenNewFileForReading(line_content, file_index))
					continue;
				else
					return;
			}
			
			if (line_content.Contains("<Trader>"))
			{
				traderID++;
				itemCounter = 0;
				
				continue;
			}
			
			if (line_content.Contains("<Category>"))
			{
				categoryId++;
				itemCounter = 0;
				
				continue;
			}
		
			if (!line_content.Contains(","))
				continue;

			if (line_content.Contains("<Currency"))
				continue;
		
			TStringArray strs = new TStringArray;
			line_content.Split( ",", strs );
			
			string itemStr = strs.Get(0);
			itemStr = FileReadHelper.TrimSpaces(itemStr);
			
			//TraderMessage.ServerLog("[TRADER] READING ITEM ENTRY: " + itemStr);
			
			string qntStr = strs.Get(1);
			qntStr = FileReadHelper.TrimSpaces(qntStr);
			
			if (qntStr.Contains("*") || qntStr.Contains("-1"))
			{
				qntStr = GetItemMaxQuantity(itemStr).ToString();
			}

			if (qntStr.Contains("VNK") || qntStr.Contains("vnk"))
				qntStr = "-6";

			if (qntStr.Contains("V") || qntStr.Contains("v"))
				qntStr = "-2";

			if (qntStr.Contains("M") || qntStr.Contains("m"))
				qntStr = "-3";

			if (qntStr.Contains("W") || qntStr.Contains("w"))
				qntStr = "-4";

			if (qntStr.Contains("S") || qntStr.Contains("s"))
				qntStr = "-5";

			if (qntStr.Contains("K") || qntStr.Contains("k"))
				qntStr = "-7";
			
			string buyStr = strs.Get(2);
			buyStr = FileReadHelper.TrimSpaces(buyStr);
			
			string sellStr = strs.Get(3);
			sellStr = FileReadHelper.TrimSpaces(sellStr);
			
			if (!FileExist(m_Trader_StockDirPath))
    	    {
				MakeDirectory(m_Trader_StockDirPath);
			}
			if (!itemStr.Contains("MoneyRuble"))
			{
				if (FileExist(m_Trader_StockDirPath + traderID.ToString() + "_" + itemStr + ".txt"))
	    	    {				
					ref array<string> ItemConfig = LoadStockData(-1, traderID, itemStr);
					Stock = ItemConfig.Get(0).ToInt();
					Nominal = ItemConfig.Get(1).ToInt();
					BasePrice = ItemConfig.Get(2);
					BaseSellPrice = ItemConfig.Get(3);
					RealNominal = ItemConfig.Get(4).ToInt();
					excludeFromRestock = ItemConfig.Get(5).ToInt();
					ItemRestockTimer = ItemConfig.Get(6).ToInt();
					ItemRestockPercent = ItemConfig.Get(7).ToFloat();
				
										
					fRealNominal = RealNominal;
					fRealNominal *= BalancedStockMultiplier;
					modifiedNominal = fRealNominal;
					if (modifiedNominal < MinBalancedStock && RealNominal != 0)
						modifiedNominal = MinBalancedStock;
					if (modifiedNominal > MaxBalancedStock && RealNominal != 0)
						modifiedNominal = MaxBalancedStock;
				}
				else
				{
					RealNominal = GetNominalFromString(itemStr); //Slow AF, thats why database is required ** Fixxed speed, takes 5 sec to load 12164*45000 lines
					
					fRealNominal = RealNominal;
					fRealNominal *= BalancedStockMultiplier;
					modifiedNominal = fRealNominal;
					
					if (modifiedNominal < MinBalancedStock && RealNominal != 0)
						modifiedNominal = MinBalancedStock;
					if (modifiedNominal > MaxBalancedStock && RealNominal != 0)
						modifiedNominal = MaxBalancedStock;
					
					Nominal = modifiedNominal;
					
					if (RealNominal <= 0)
					{
						Stock = 0;
						excludeFromRestock = 1;
					}
					else
					{
						Stock = Nominal;
					}
					
					BasePrice = buyStr;
					BaseSellPrice = sellStr;
					overwrite = true;
				}
				
				if (Nominal != modifiedNominal)
				{
					Nominal = modifiedNominal;
					overwrite = true;
				}
								
				if (BasePrice != buyStr || BaseSellPrice != sellStr)
					overwrite = true;
				
				m_Trader_ItemsBuyValue.Insert(GetBuyPrice(Stock, Nominal, buyStr.ToInt()));
				if (sellStr.Contains("A"))
					m_Trader_ItemsSellValue.Insert(GetSellPrice(Stock, Nominal, buyStr.ToInt(), true));
				else
					m_Trader_ItemsSellValue.Insert(GetSellPrice(Stock, Nominal, sellStr.ToInt(), false));
				
				if (!ItemRestockTimer > 0)
					modifiedItemRestockTimer = m_Trader_DefaultRestockTime;
				else
					modifiedItemRestockTimer = ItemRestockTimer;
				if (LowestRestockTimer > ItemRestockTimer)
					LowestRestockTimer = ItemRestockTimer;
				if (HighestRestockTimer < ItemRestockTimer)
					HighestRestockTimer = ItemRestockTimer;
				AddValueToUniqueArray(modifiedItemRestockTimer);
				m_Trader_Restock_Times.Insert(modifiedItemRestockTimer);
				m_Trader_Restock_moddedTimes.Insert(modifiedItemRestockTimer);
				m_Trader_Restock_Percents.Insert(ItemRestockPercent);
			}
			else
			{
				m_Trader_ItemsBuyValue.Insert(buyStr.ToInt());
				m_Trader_ItemsSellValue.Insert(sellStr.ToInt());
				m_Trader_Restock_Times.Insert(-1);
				m_Trader_Restock_Percents.Insert(-1);
			}
			
			m_Trader_ItemsTraderId.Insert(traderID);
			m_Trader_ItemsCategoryId.Insert(categoryId);
			m_Trader_ItemsClassnames.Insert(itemStr);
			m_Trader_ItemsQuantity.Insert(qntStr.ToInt());
			
			if (overwrite)
				SaveStockData(itemCounter, Stock.ToString(), Nominal.ToString(), buyStr, sellStr, RealNominal.ToString(), excludeFromRestock.ToString(), ItemRestockTimer.ToString(), ItemRestockPercent.ToString());
			
			
			itemCounter++;
		}
		CloseFile(file_index);
		if (m_Trader_AutoCalculateRestockTimer)
			m_Trader_RestockUpdateTimerMax = GetLowestDeltaFromIntArray(m_Trader_Restock_moddedTimes_UniqueArray);
		//------------------------------------------------------------------------------------
		
		file_index = OpenFile(m_Trader_ObjectsFilePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO TRADEROBJECTS FILE!");
			return;
		}
		
		bool skipLine = false;
		int markerCounter = 0;				

		line_content = "";
		while ( markerCounter <= 5000 && line_content.Contains("<FileEnd>") == false)
		{
			// Get Trader Marker Trader ID:
			if (!skipLine)
				line_content = FileReadHelper.SearchForNextTermsInFile(file_index, {"<TraderMarker>", "<OpenFile>"}, "<FileEnd>");
			else
				skipLine = false;					
			
			if (line_content.Contains("<OpenFile>"))
			{
				if (!OpenNewFileForReading(line_content, file_index))
					return;
			}	
			
			if (!line_content.Contains("<TraderMarker>"))
				continue;
			
			line_content.Replace("<TraderMarker>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);
			
			//TraderMessage.ServerLog("[TRADER] READING MARKER ID ENTRY..");
								
			m_Trader_TraderIDs.Insert(line_content.ToInt());
			
			// Get Trader Marker Position:		
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<TraderMarkerPosition>", "<FileEnd>");
			
			line_content.Replace("<TraderMarkerPosition>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			TStringArray strsm = new TStringArray;
			line_content.Split( ",", strsm );
			
			string traderMarkerPosX = strsm.Get(0);
			traderMarkerPosX = FileReadHelper.TrimSpaces(traderMarkerPosX);
			
			string traderMarkerPosY = strsm.Get(1);
			traderMarkerPosY = FileReadHelper.TrimSpaces(traderMarkerPosY);
			
			string traderMarkerPosZ = strsm.Get(2);
			traderMarkerPosZ = FileReadHelper.TrimSpaces(traderMarkerPosZ);
			
			vector markerPosition = "0 0 0";
			markerPosition[0] = traderMarkerPosX.ToFloat();
			markerPosition[1] = traderMarkerPosY.ToFloat();
			markerPosition[2] = traderMarkerPosZ.ToFloat();
			
			//TraderMessage.ServerLog("[TRADER] READING MARKER POSITION ENTRY..");
			
			m_Trader_TraderPositions.Insert(markerPosition);
			
			// Get Trader Marker Safezone Radius:					
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<TraderMarkerSafezone>", "<FileEnd>");
			
			line_content.Replace("<TraderMarkerSafezone>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);
			
			//TraderMessage.ServerLog("[TRADER] READING MARKER SAFEZONE ENTRY..");
			
			m_Trader_TraderSafezones.Insert(line_content.ToInt());

			// Get Trader Marker Vehicle Spawnpoint:					
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<VehicleSpawn>", "<TraderMarker>");

			if(line_content == string.Empty)
				break;

			if (line_content.Contains("<TraderMarker>"))
			{
				skipLine = true;
				m_Trader_TraderVehicleSpawns.Insert("0 0 0");
				m_Trader_TraderVehicleSpawnsOrientation.Insert("0 0 0");
				continue;
			}

			line_content.Replace("<VehicleSpawn>", "");
			line_content = FileReadHelper.TrimComment(line_content);

			TStringArray strtmv = new TStringArray;
			line_content.Split( ",", strtmv );
			
			string traderMarkerVehiclePosX = strtmv.Get(0);
			traderMarkerVehiclePosX = FileReadHelper.TrimSpaces(traderMarkerVehiclePosX);
			
			string traderMarkerVehiclePosY = strtmv.Get(1);
			traderMarkerVehiclePosY = FileReadHelper.TrimSpaces(traderMarkerVehiclePosY);
			
			string traderMarkerVehiclePosZ = strtmv.Get(2);
			traderMarkerVehiclePosZ = FileReadHelper.TrimSpaces(traderMarkerVehiclePosZ);
			
			vector markerVehiclePosition = "0 0 0";
			markerVehiclePosition[0] = traderMarkerVehiclePosX.ToFloat();
			markerVehiclePosition[1] = traderMarkerVehiclePosY.ToFloat();
			markerVehiclePosition[2] = traderMarkerVehiclePosZ.ToFloat();

			//TraderMessage.ServerLog("[TRADER] READING MARKER VEHICLE ENTRY..");

			m_Trader_TraderVehicleSpawns.Insert(markerVehiclePosition);

			// Get Trader Marker Vehicle Orientation:
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<VehicleSpawnOri>", "<TraderMarker>");

			if(line_content == string.Empty)
				break;

			if (line_content.Contains("<TraderMarker>"))
			{
				skipLine = true;
				m_Trader_TraderVehicleSpawnsOrientation.Insert("0 0 0");
				continue;
			}

			line_content.Replace("<VehicleSpawnOri>", "");
			line_content = FileReadHelper.TrimComment(line_content);

			TStringArray strtmvd = new TStringArray;
			line_content.Split( ",", strtmvd );
			
			string traderMarkerVehicleOriX = strtmvd.Get(0);
			traderMarkerVehicleOriX = FileReadHelper.TrimSpaces(traderMarkerVehicleOriX);
			
			string traderMarkerVehicleOriY = strtmvd.Get(1);
			traderMarkerVehicleOriY = FileReadHelper.TrimSpaces(traderMarkerVehicleOriY);
			
			string traderMarkerVehicleOriZ = strtmvd.Get(2);
			traderMarkerVehicleOriZ = FileReadHelper.TrimSpaces(traderMarkerVehicleOriZ);
			
			vector markerVehicleOrientation = "0 0 0";
			markerVehicleOrientation[0] = traderMarkerVehicleOriX.ToFloat();
			markerVehicleOrientation[1] = traderMarkerVehicleOriY.ToFloat();
			markerVehicleOrientation[2] = traderMarkerVehicleOriZ.ToFloat();

			m_Trader_TraderVehicleSpawnsOrientation.Insert(markerVehicleOrientation);

			markerCounter++;
		}
		
		CloseFile(file_index);
		
		//------------------------------------------------------------------------------------
		
		file_index = OpenFile(m_Trader_VehiclePartsFilePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO VEHICLEPARTS FILE!");
			return;
		}
		
		skipLine = false;
		int vehicleCounter = 0;
		
		line_content = "";
		while ( vehicleCounter <= 5000 && line_content.Contains("<FileEnd>") == false)
		{
			// Get Vehicle Name Entrys:
			if (!skipLine)
				line_content = FileReadHelper.SearchForNextTermsInFile(file_index, {"<VehicleParts>", "<OpenFile>"}, "<FileEnd>");
			else
				skipLine = false;

			if (line_content.Contains("<OpenFile>"))
			{
				if (!OpenNewFileForReading(line_content, file_index))
					return;
			}	
			
			if (!line_content.Contains("<VehicleParts>"))
				continue;
			
			line_content.Replace("<VehicleParts>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);
			
			//TraderMessage.ServerLog("[TRADER] READING VEHICLE NAME ENTRY..");

			m_Trader_Vehicles.Insert(line_content);

			char_count = 0;
			int vehiclePartsCounter = 0;
			//while ( vehiclePartsCounter <= 5000  && char_count != -1 && line_content.Contains("<FileEnd>") == false)
			while (true)
			{
				// Get Vehicle Parts Entrys:
				char_count = FGets( file_index,  line_content );

				line_content = FileReadHelper.TrimComment(line_content);
				line_content = FileReadHelper.TrimSpaces(line_content);

				if (line_content == "")
					continue;
				
				if (line_content.Contains("<OpenFile>"))
				{
					if (OpenNewFileForReading(line_content, file_index))
						continue;
					else
						return;
				}	

				if (line_content.Contains("<VehicleParts>"))
				{
					skipLine = true;						
					break;
				}

				if (line_content.Contains("<FileEnd>") || char_count == -1 || vehiclePartsCounter > 5000)
				{
					line_content = "<FileEnd>";
					break;
				}

				m_Trader_VehiclesParts.Insert(line_content);
				m_Trader_VehiclesPartsVehicleId.Insert(vehicleCounter);

				vehiclePartsCounter++;
			}
			
			vehicleCounter++;
		}
		
		CloseFile(file_index);
		
		//------------------------------------------------------------------------------------

		TraderMessage.ServerLog("[TRADER] DONE READING!");
		m_Trader_ReadAllTraderData = true;
	}
	
	void TraderEconomyPatch()
	{
	//	Read TraderVariables.txt

		FileHandle file_index = OpenFile(m_Trader_VariableFilePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO TRADERVARIABLE FILE!");
			return;
		}

		int variableCounter = 0;
		
		string line_content = "";
		while (variableCounter <= 500 && !line_content.Contains("<FileEnd>"))
		{
			bool validEntry = false;

			line_content = "";
			int char_count = FGets( file_index,  line_content );
			line_content = FileReadHelper.TrimComment(line_content);
			
			if (line_content.Contains("<TraderTaxes>"))
			{
				line_content.Replace("<TraderTaxes>", "");
				line_content = FileReadHelper.TrimComment(line_content);
					
				m_Trader_Taxes = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] TAXES = " + line_content);
			}
			
			if (line_content.Contains("<ReplaceSellPrices>"))
			{
				line_content.Replace("<ReplaceSellPrices>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				if (line_content.Contains("true"))
					ReplaceSellPrices = true;
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] ReplaceSellPrices = <" + line_content + ">");
			}
			
			if (line_content.Contains("<MinBalancedStock>"))
			{
				line_content.Replace("<MinBalancedStock>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				MinBalancedStock = line_content.ToInt();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] MinBalancedStock = <" + line_content + ">");
			}
			
			if (line_content.Contains("<MaxBalancedStock>"))
			{
				line_content.Replace("<MaxBalancedStock>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				MaxBalancedStock = line_content.ToInt();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] MaxBalancedStock = <" + line_content + ">");
			}
			
			if (line_content.Contains("<BalancedStockMultiplier>"))
			{
				line_content.Replace("<BalancedStockMultiplier>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				BalancedStockMultiplier = line_content.ToFloat();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] BalancedStockMultiplier = <" + line_content + ">");
			}
			
			if (line_content.Contains("<PathToTypesXml>"))
			{
				line_content.Replace("<PathToTypesXml>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				m_Trader_TypesXMLPath = "$CurrentDir:" + line_content;
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] PathToTypesXml = <" + line_content + ">");
			}
			
			if (line_content.Contains("<RestockUpdateTimer>"))
			{
				line_content.Replace("<RestockUpdateTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				if (line_content.Contains("A") || line_content.Contains("a"))
					m_Trader_AutoCalculateRestockTimer = true;
				else
				{
					m_Trader_RestockUpdateTimerMax = line_content.ToFloat();
					m_Trader_AutoCalculateRestockTimer = false;
				}
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] RestockUpdateTimer = <" + line_content + ">");
			}
			
			if (line_content.Contains("<RestockPercent>"))
			{
				line_content.Replace("<RestockPercent>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				RestockPercent = line_content.ToFloat();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] RestockPercent = <" + line_content + ">");
			}
			
			if (line_content.Contains("<DefaultRestockTime>"))
			{
				line_content.Replace("<DefaultRestockTime>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				m_Trader_DefaultRestockTime = line_content.ToInt();
				validEntry = true;
				
				TraderMessage.ServerLog("[TRADER] DefaultRestockTime = <" + line_content + ">");
			}
			
			
			
			if (validEntry)
				variableCounter++;
		}

		CloseFile(file_index);
		
	//	Init default Values
		m_Trader_Economy_DefaultItemValues = new array<string>;
		m_Trader_Economy_DefaultItemValues.Insert("0");
		m_Trader_Economy_DefaultItemValues.Insert(MinBalancedStock.ToString());
		m_Trader_Economy_DefaultItemValues.Insert("-1");
		m_Trader_Economy_DefaultItemValues.Insert("-1");
		m_Trader_Economy_DefaultItemValues.Insert("-1");
		m_Trader_Economy_DefaultItemValues.Insert("0");
		m_Trader_Economy_DefaultItemValues.Insert("0");
		m_Trader_Economy_DefaultItemValues.Insert("0.00");
		
		
	//	Edit Prices
		file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
		
		int itemCounter = 0;
		char_count = 0;
		int traderID = -1;
		int categoryId = -1;
		
		int Stock;
		int Nominal;
		string BasePrice;
		string BaseSellPrice;
		int RealNominal;
		int excludeFromRestock;
		int ItemRestockTimer;
		float ItemRestockPercent;
		
		bool overwrite;
		float fRealNominal;
		int modifiedNominal;
		int modifiedItemRestockTimer;
		
		
		line_content = "";
		while (char_count != -1 && !line_content.Contains("<FileEnd>"))
		{	
			char_count = FGets( file_index,  line_content );
			line_content = FileReadHelper.TrimComment(line_content);
			
			overwrite = false;
			ItemRestockTimer = 0;
			ItemRestockPercent = 0.00;
			excludeFromRestock = 0;	

			if (line_content.Contains("<Trader>"))
			{
				traderID++;
				
				continue;
			}
			
			if (line_content.Contains("<Category>"))
			{
				categoryId++;
				
				continue;
			}
		
			if (!line_content.Contains(","))
				continue;

			if (line_content.Contains("<Currency"))
				continue;
		
			TStringArray strs = new TStringArray;
			line_content.Split( ",", strs );
			
			string itemStr = strs.Get(0);
			itemStr = FileReadHelper.TrimSpaces(itemStr);
			
			//TraderMessage.ServerLog("[TRADER] READING ITEM ENTRY: " + itemStr);
			
			string qntStr = strs.Get(1);
			qntStr = FileReadHelper.TrimSpaces(qntStr);
			
			if (qntStr.Contains("*") || qntStr.Contains("-1"))
			{
				qntStr = GetItemMaxQuantity(itemStr).ToString();
			}

			if (qntStr.Contains("VNK") || qntStr.Contains("vnk"))
				qntStr = "-6";

			if (qntStr.Contains("V") || qntStr.Contains("v"))
				qntStr = "-2";

			if (qntStr.Contains("M") || qntStr.Contains("m"))
				qntStr = "-3";

			if (qntStr.Contains("W") || qntStr.Contains("w"))
				qntStr = "-4";

			if (qntStr.Contains("S") || qntStr.Contains("s"))
				qntStr = "-5";

			if (qntStr.Contains("K") || qntStr.Contains("k"))
				qntStr = "-7";
			
			string buyStr = strs.Get(2);
			buyStr = FileReadHelper.TrimSpaces(buyStr);
			
			string sellStr = strs.Get(3);
			sellStr = FileReadHelper.TrimSpaces(sellStr);
			
			if (!FileExist(m_Trader_StockDirPath))
    	    {
				MakeDirectory(m_Trader_StockDirPath);
			}
			if (!itemStr.Contains("MoneyRuble"))
			{
				if (FileExist(m_Trader_StockDirPath + traderID.ToString() + "_" + itemStr + ".txt"))
	    	    {				
					ref array<string> ItemConfig = LoadStockData(-1, traderID, itemStr);
					Stock = ItemConfig.Get(0).ToInt();
					Nominal = ItemConfig.Get(1).ToInt();
					BasePrice = ItemConfig.Get(2);
					BaseSellPrice = ItemConfig.Get(3);
					RealNominal = ItemConfig.Get(4).ToInt();
					excludeFromRestock = ItemConfig.Get(5).ToInt();
					ItemRestockTimer = ItemConfig.Get(6).ToInt();
					ItemRestockPercent = ItemConfig.Get(7).ToFloat();
				
										
					fRealNominal = RealNominal;
					fRealNominal *= BalancedStockMultiplier;
					modifiedNominal = fRealNominal;
					if (modifiedNominal < MinBalancedStock && RealNominal != 0)
						modifiedNominal = MinBalancedStock;
					if (modifiedNominal > MaxBalancedStock && RealNominal != 0)
						modifiedNominal = MaxBalancedStock;
				}
				else
				{
					RealNominal = GetNominalFromString(itemStr); //Slow AF, thats why database is required ** Fixxed speed, takes 5 sec to load 12164*45000 lines
					
					fRealNominal = RealNominal;
					fRealNominal *= BalancedStockMultiplier;
					modifiedNominal = fRealNominal;
					
					if (modifiedNominal < MinBalancedStock && RealNominal != 0)
						modifiedNominal = MinBalancedStock;
					if (modifiedNominal > MaxBalancedStock && RealNominal != 0)
						modifiedNominal = MaxBalancedStock;
					
					Nominal = modifiedNominal;
					
					if (RealNominal <= 0)
					{
						Stock = 0;
						excludeFromRestock = 1;
					}
					else
					{
						Stock = Nominal;
					}
					
					BasePrice = buyStr;
					BaseSellPrice = sellStr;
					overwrite = true;
				}
				
				if (Nominal != modifiedNominal)
				{
					Nominal = modifiedNominal;
					overwrite = true;
				}
								
				if (BasePrice != buyStr || BaseSellPrice != sellStr)
					overwrite = true;
				
				m_Trader_ItemsBuyValue[itemCounter] = GetBuyPrice(Stock, Nominal, buyStr.ToInt());
				if (sellStr.Contains("A"))
					m_Trader_ItemsSellValue[itemCounter] = GetSellPrice(Stock, Nominal, buyStr.ToInt(), true);
				else
					m_Trader_ItemsSellValue[itemCounter] = GetSellPrice(Stock, Nominal, sellStr.ToInt(), false);
				
				if (!ItemRestockTimer > 0)
					modifiedItemRestockTimer = m_Trader_DefaultRestockTime;
				else
					modifiedItemRestockTimer = ItemRestockTimer;
				if (LowestRestockTimer > ItemRestockTimer)
					LowestRestockTimer = ItemRestockTimer;
				if (HighestRestockTimer < ItemRestockTimer)
					HighestRestockTimer = ItemRestockTimer;
				AddValueToUniqueArray(modifiedItemRestockTimer);
				m_Trader_Restock_Times.Insert(modifiedItemRestockTimer);
				m_Trader_Restock_moddedTimes.Insert(modifiedItemRestockTimer);
				m_Trader_Restock_Percents.Insert(ItemRestockPercent);
			}
			else
			{
				m_Trader_ItemsBuyValue[itemCounter] = buyStr.ToInt();
				m_Trader_ItemsSellValue[itemCounter] = sellStr.ToInt();
				m_Trader_Restock_Times.Insert(-1);
				m_Trader_Restock_moddedTimes.Insert(-1);
				m_Trader_Restock_Percents.Insert(-1);
			}
			
			if (overwrite)
				SaveStockData(itemCounter, Stock.ToString(), Nominal.ToString(), buyStr, sellStr, RealNominal.ToString(), excludeFromRestock.ToString(), ItemRestockTimer.ToString(), ItemRestockPercent.ToString());
			
			
			itemCounter++;
		}
		CloseFile(file_index);
		if (m_Trader_AutoCalculateRestockTimer)
			m_Trader_RestockUpdateTimerMax = GetLowestDeltaFromIntArray(m_Trader_Restock_moddedTimes_UniqueArray);
	}
	
	
	int GetBuyPrice(int stock, int nominal, int Price)
	{
		if (Price == -1)
			return -1;
		
		if (stock == 0)
			return -1;
		
		if (nominal == 0)
			return Price;
		
		int BuyPrice = -1;
		float fBuyPrice = -1;
		float fStock = stock;
		float fNominal = nominal;
		float fPrice = Price;
		
		fBuyPrice = fPrice * (fNominal / fStock);
		BuyPrice = fBuyPrice;
		return BuyPrice;
	}
	
	int GetSellPrice(int stock, int nominal, int Price, bool modifier)
	{
		if (Price == -1)
			return -1;
		
		if (stock <= -1)
			stock = 0;
		
		if (nominal == 0)
			return Price;
		
		int SellPrice = -1;
		float fSellPrice = -1;
		float fStock = stock;
		float fNominal = nominal;
		float fPrice = Price;
		
		fSellPrice = fPrice * (fNominal / (fStock + 1));
		if (modifier)
			fSellPrice *= m_Trader_Taxes;
		SellPrice = fSellPrice;
		return SellPrice;
	}
	
	int GetNominalFromString(string item)
	{
		string DBItem;
		if (!TraderDBInit)
			InitTraderDB();
		
		for ( int i = 0; DBItem != item && i < TraderDB_Classnames.Count(); i++ )
		{
			DBItem = TraderDB_Classnames[i];
		}
		i-=1;
		if (DBItem == "NULL")
		{
			return -1;
		}
		else
		{
			return TraderDB_Nominal[i];
		}
	}
	
	void InitTraderDB()
	{
		string line_content;
		FileHandle m_types = OpenFile(m_Trader_TypesXMLPath, FileMode.READ);
		
		if ( m_types == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO TYPES.XML FILE!");
			return;
		}
		
		line_content = FileReadHelper.SearchForNextTermInFile(m_types, "<type name=\"", "</types>");	
		while (!line_content.Contains("</types>"))
		{
			line_content.Replace("<type name=\"", "");
			line_content.Replace("\">", "");
			line_content = FileReadHelper.TrimSpaces(line_content);
			TraderDB_Classnames.Insert(line_content);
			
			line_content = FileReadHelper.SearchForNextTermInFile(m_types, "<nominal>", "</type>");
			if (line_content.Contains("</type>"))
				TraderDB_Nominal.Insert(-1);
			else
			{
				line_content.Replace("<nominal>", "");
				line_content.Replace("</nominal>", "");
				line_content = FileReadHelper.TrimSpaces(line_content);
				TraderDB_Nominal.Insert(line_content.ToInt());
			}
			
			line_content = FileReadHelper.SearchForNextTermInFile(m_types, "<type name=\"", "</types>");	
		}
		CloseFile(m_types);
		TraderDB_Classnames.Insert("NULL");
		TraderDB_Nominal.Insert(-1);
		TraderDBInit = true;
	}
	
	void OnItemBought()
	{
		while (DayZPlayerImplement.wa_boughtItems.Count() > 0)
		{
			int itemID = DayZPlayerImplement.wa_boughtItems[0];
			
			int traderID = m_Trader_ItemsTraderId.Get(itemID);
			string itemClassname = m_Trader_ItemsClassnames.Get(itemID);
			
			TraderMessage.ServerLog("[TRADER] Update Item: " + itemClassname + " (" + itemID.ToString() + ") | Price: " + m_Trader_ItemsBuyValue.Get(itemID).ToString() + " | " + m_Trader_ItemsSellValue.Get(itemID).ToString());
		
			Param4<int, int, int, string> itemData = ModifyTraderStock(itemID, -1);
			int Stock = itemData.param1;
			int Nominal = itemData.param2;
			int BasePrice = itemData.param3;
			string BaseSellPrice = itemData.param4;
			
			m_Trader_ItemsBuyValue[itemID] = GetBuyPrice(Stock, Nominal, BasePrice);
			if (BaseSellPrice.Contains("A"))
				m_Trader_ItemsSellValue[itemID] = GetSellPrice(Stock, Nominal, BasePrice, true);
			else
				m_Trader_ItemsSellValue[itemID] = GetSellPrice(Stock, Nominal, BaseSellPrice.ToInt(), false);	
			
			TraderMessage.ServerLog("[TRADER] Price modified to: " + m_Trader_ItemsBuyValue.Get(itemID).ToString() + " | " + m_Trader_ItemsSellValue.Get(itemID).ToString());
			TraderSendPriceUpdate(itemID);
			DayZPlayerImplement.wa_boughtItems.Remove(0);
		}
	}


	void OnItemSold()
	{
		while (DayZPlayerImplement.wa_soldItems.Count() > 0)
		{
			int itemID = DayZPlayerImplement.wa_soldItems[0];
			
			int traderID = m_Trader_ItemsTraderId.Get(itemID);
			string itemClassname = m_Trader_ItemsClassnames.Get(itemID);
			
			TraderMessage.ServerLog("[TRADER] Item: " + itemClassname + " (" + itemID.ToString() + ") | Price: " + m_Trader_ItemsBuyValue.Get(itemID).ToString() + " | " + m_Trader_ItemsSellValue.Get(itemID).ToString());
			
			Param4<int, int, int, string> itemData = ModifyTraderStock(itemID, 1);
			int Stock = itemData.param1;
			int Nominal = itemData.param2;
			int BasePrice = itemData.param3;
			string BaseSellPrice = itemData.param4;
		
			m_Trader_ItemsBuyValue[itemID] = GetBuyPrice(Stock, Nominal, BasePrice);
			if (BaseSellPrice.Contains("A"))
				m_Trader_ItemsSellValue[itemID] = GetSellPrice(Stock, Nominal, BasePrice, true);
			else
				m_Trader_ItemsSellValue[itemID] = GetSellPrice(Stock, Nominal, BaseSellPrice.ToInt(), false);		
			
			TraderMessage.ServerLog("[TRADER] Price modified to: " + m_Trader_ItemsBuyValue.Get(itemID).ToString() + " | " + m_Trader_ItemsSellValue.Get(itemID).ToString());
			TraderSendPriceUpdate(itemID);
			DayZPlayerImplement.wa_soldItems.Remove(0);
		}
	}

	Param4<int, int, int, string> ModifyTraderStock(int itemID, int value)
	{
		int traderUID = m_Trader_ItemsTraderId.Get(itemID);
		string itemType = m_Trader_ItemsClassnames.Get(itemID);
		
		
		ref array<string> ItemConfig = LoadStockData(itemID);
		int Stock = ItemConfig.Get(0).ToInt();
		string Nominal = ItemConfig.Get(1);
		string BasePrice = ItemConfig.Get(2);
		string BaseSellPrice = ItemConfig.Get(3);
		string RealNominal = ItemConfig.Get(4);
		string excludeFromRestock = ItemConfig.Get(5);
		string ItemRestockTimer = ItemConfig.Get(6);
		string ItemRestockPercent = ItemConfig.Get(7);
		
		int newStock = 0;
		
		newStock = Stock + value;
		if (newStock < 0)
			newStock =  0;
		
		if (Stock == -1 || Nominal == "NULL" || BasePrice == "NULL" || RealNominal == "NULL" || BaseSellPrice == "NULL")
			TraderMessage.ServerLog("ERROR: NULL VALUE LOADED ... ");
		
		SaveStockData(itemID, newStock.ToString(), Nominal, BasePrice, BaseSellPrice, RealNominal, excludeFromRestock, ItemRestockTimer, ItemRestockPercent);	
		
		Param4<int, int, int, string> itemData = new Param4<int, int, int, string>( newStock, Nominal.ToInt(), BasePrice.ToInt(), BaseSellPrice);
		return itemData;
	}
	
	void TraderSendPriceUpdate(int itemID)
	{
		for (int o = 0; o < m_Players.Count(); o++)
		{
			PlayerBase player = PlayerBase.Cast(m_Players.Get(o));
			
			player.m_Trader_ItemsBuyValue[itemID] = m_Trader_ItemsBuyValue.Get(itemID);
			player.m_Trader_ItemsSellValue[itemID] = m_Trader_ItemsSellValue.Get(itemID);
			
			Param3<int, int, int> crp6 = new Param3<int, int, int>( m_Trader_ItemsBuyValue.Get(itemID), m_Trader_ItemsSellValue.Get(itemID), itemID );
			GetGame().RPCSingleParam(player, TPRPCs.RPC_SEND_TRADER_PRICES_UPDATE, crp6, true, player.GetIdentity());
			
			Param1<bool> crpRM = new Param1<bool>( true );
			GetGame().RPCSingleParam(player, TPRPCs.RPC_SEND_TRADER_PRICES_UPDATE_REFRESH, crpRM, true, player.GetIdentity());
		}
	}
	
	void RemodTraderConfig()
	{
		int char_count = 0;
		int char_count2 = 0;
		float fchar_count2 = 0;
		string spaces1 = "";
		string spaces2 = "";
		string spaces3 = "";	
		string line_content = "";
		string line_content2 = "";
		
		FileHandle file_index;
		FileHandle file_index2;
		
		
		file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
		file_index2 = OpenFile(m_Trader_ConfigFilePath2, FileMode.WRITE);
		
		while (char_count != -1 && !line_content.Contains("<FileEnd>"))
		{
			char_count = FGets( file_index,  line_content );
			line_content2 = FileReadHelper.TrimComment(line_content);

			if (line_content2.Contains("<Trader>") || line_content2.Contains("<Category>") || line_content2.Contains("<Currency") || !line_content2.Contains(","))
			{
				FPrintln(file_index2, line_content);
				
				continue;
			}
			
			TStringArray strs = new TStringArray;
			line_content.Split( ",", strs );
			
			string itemStr = strs.Get(0);
			itemStr = FileReadHelper.TrimSpaces(itemStr);
			
			char_count2 = 43 - itemStr.Length();
			fchar_count2 = char_count2 / 4;
			spaces1= "";
			
			for (; fchar_count2 > 0; fchar_count2--)
			{
				spaces1 += "	";
			}
			
			string qntStr = strs.Get(1);
			qntStr = FileReadHelper.TrimSpaces(qntStr);
			
			char_count2 = 11 - qntStr.Length();
			fchar_count2 = char_count2 / 4;
			spaces2= "";
			
			for (; fchar_count2 > 0; fchar_count2--)
			{
				spaces2 += "	";
			}
			
			string buyStr = strs.Get(2);
			buyStr = FileReadHelper.TrimSpaces(buyStr);
			
			char_count2 = 11 - buyStr.Length();
			fchar_count2 = char_count2 / 4;
			spaces3= "";
			
			for (; fchar_count2 > 0; fchar_count2--)
			{
				spaces3 += "	";
			}
			
			FPrintln(file_index2, "		" + itemStr + "," + spaces1 + qntStr + "," + spaces2 + buyStr + "," + spaces3 + "A");
		}
		CloseFile(file_index);
		CloseFile(file_index2);
	}
	
	void RestockTrader()
	{
		//TraderMessage.ServerLog("[TRADER] DEBUG3: RestockTrader()");
		int ID = -1;
		int char_count = 0;
		int traderUID = -1;
		string line_content = "";
		
		FileHandle file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
		
		while (char_count != -1 && !line_content.Contains("<FileEnd>"))
		{	
			
			
			char_count = FGets( file_index,  line_content );
			
			line_content = FileReadHelper.TrimComment(line_content);

			if (line_content.Contains("<Trader>"))
			{
				traderUID++;
				
				continue;
			}
		
			if (line_content.Contains("<Category>"))
				continue;
			
			if (!line_content.Contains(","))
				continue;
			
			if (line_content.Contains("<Currency"))
				continue;
			
			
			TStringArray strs = new TStringArray;
			line_content.Split( ",", strs );
			
			string itemType = strs.Get(0);
			itemType = FileReadHelper.TrimSpaces(itemType);
			
			ID++;
			//TraderMessage.ServerLog("[TRADER] DEBUG4: traderUID = " + traderUID + " / Item = " + itemType + "(" + ID.ToString() + ")" );
			
			
			//----------------------------------------
		
			int char_count3 = 0;
			FileHandle file_index3;
			string line_content3 = "";
			float Stock = -1;
			int newStock = 0;
			string Nominal = "NULL";
			string BasePrice = "NULL";
			string RealNominal = "NULL";
			string BaseSellPrice = "NULL";
			string excludeFromRestock= "0";
			
			file_index3 = OpenFile(m_Trader_StockDirPath + traderUID.ToString() + "_" + itemType + ".txt", FileMode.READ);
			
			
			char_count3 = FGets(file_index3, line_content3);
			while (char_count3 != -1 && !line_content3.Contains("<FileEnd>"))
			{
				if (line_content3.Contains("<Stock>"))
				{
					line_content3.Replace("<Stock>", "");
					line_content3 = FileReadHelper.TrimComment(line_content3);
					
					Stock = line_content3.ToFloat();
					
				}
				
				if (line_content3.Contains("<BalancedStock>"))
				{
					line_content3.Replace("<BalancedStock>", "");
					line_content3 = FileReadHelper.TrimComment(line_content3);
					
					Nominal = line_content3;
				}
				
				if (line_content3.Contains("<BasePrice>"))
				{
					line_content3.Replace("<BasePrice>", "");
					line_content3 = FileReadHelper.TrimComment(line_content3);
					
					BasePrice = line_content3;
				}
				
				if (line_content3.Contains("<BaseSellPrice>"))
				{
					line_content3.Replace("<BaseSellPrice>", "");
					line_content3 = FileReadHelper.TrimComment(line_content3);
					
					BaseSellPrice = line_content3;
				}
				
				if (line_content3.Contains("<RealNominal>"))
				{
					line_content3.Replace("<RealNominal>", "");
					line_content3 = FileReadHelper.TrimComment(line_content3);
					
					RealNominal = line_content3;
				}
			
				if (line_content3.Contains("<ExcludeFromRestock>"))
				{
					line_content3.Replace("<ExcludeFromRestock>", "");
					line_content3 = FileReadHelper.TrimComment(line_content3);
					
					excludeFromRestock = line_content3;
				}
				
				char_count3 = FGets(file_index3, line_content3);
			}
			
			if (excludeFromRestock == "1")
				continue;
			
			CloseFile(file_index3);
			
			float value = (Nominal.ToFloat() - Stock) * RestockPercent;
			
			if (value > 0 && value < 1)
				value += Math.RandomFloatInclusive(0.0, 1.0);
			
			newStock = Stock + value;
			
			//TraderMessage.ServerLog("[TRADER] DEBUG5: " + Stock.ToString() + ", " + Nominal + ", " + BasePrice + ", " + BaseSellPrice + ", " + RealNominal);
			
			if (newStock != Stock)
			{
				
				//TraderMessage.ServerLog("[TRADER] DEBUG6: (newStock != Stock) -> true");
				file_index3 = OpenFile(m_Trader_StockDirPath + traderUID.ToString() + "_" + itemType + ".txt", FileMode.WRITE);
				
				FPrintln(file_index3, "<Stock> " + newStock.ToString());
				FPrintln(file_index3, "<BalancedStock> " + Nominal);
				FPrintln(file_index3, "<BasePrice> " + BasePrice);
				FPrintln(file_index3, "<BaseSellPrice> " + BaseSellPrice);
				FPrintln(file_index3, "<RealNominal> " + RealNominal);
				FPrintln(file_index3, "<ExcludeFromRestock> " + excludeFromRestock);
				FPrintln(file_index3, "<FileEnd>");
				
				CloseFile(file_index3);
				
				
				//TraderMessage.ServerLog("[TRADER] DEBUG7: ID = " + ID);
				if (ID != -1)
				{
					m_Trader_ItemsBuyValue[ID] = GetBuyPrice(newStock, Nominal.ToInt(), BasePrice.ToInt());
					if (BaseSellPrice != "A")
						m_Trader_ItemsSellValue[ID] = GetSellPrice(newStock, Nominal.ToInt(), BaseSellPrice.ToInt(), false);
					else
						m_Trader_ItemsSellValue[ID] = GetSellPrice(newStock, Nominal.ToInt(), BasePrice.ToInt(), true);
					TraderSendPriceUpdate(ID);
				}
			}
		}
	}
	
	void AddValueToUniqueArray(int time)
	{
		bool duplicate = false;
		for (int i = 0; i < m_Trader_Restock_moddedTimes_UniqueArray.Count(); i++)
		{
			if (time == m_Trader_Restock_moddedTimes_UniqueArray.Get(i))
				duplicate = true;
		}
		if (!duplicate)
			m_Trader_Restock_moddedTimes_UniqueArray.Insert(time);
	}
	
	int GetLowestDeltaFromIntArray(array<int> values)
	{
		values.Insert(0);
		int LowestDelta = 999999999;
		for (int i = 0; i < m_Trader_Restock_moddedTimes_UniqueArray.Count(); i++)
		{
			int value1 = m_Trader_Restock_moddedTimes_UniqueArray.Get(i);
			for (int j = 0; i < m_Trader_Restock_moddedTimes_UniqueArray.Count(); i++)
			{
				int value2 = m_Trader_Restock_moddedTimes_UniqueArray.Get(i);
				int delta = value1 - value2;
				if (delta < 0)
					delta *= -1;
				if (LowestDelta > delta && delta > 0)
					LowestDelta = delta;
			}
		}
		return LowestDelta;
	}
	
	void RestockTraderNew(float timeslice)
	{
		m_Trader_RestockTimeSum += timeslice;
		
		int i;
		if (m_Trader_RestockTimeSum >= NextRestock)
		{
			for (i = 0; i < m_Trader_Restock_moddedTimes.Count(); i++)
			{
				if (m_Trader_Restock_moddedTimes.Get(i) == -1)
					continue;
				
				while (m_Trader_RestockTimeSum > m_Trader_Restock_moddedTimes.Get(i))
				{
					RestockItem(i);
					m_Trader_Restock_moddedTimes[i] = m_Trader_Restock_moddedTimes.Get(i) + m_Trader_Restock_Times.Get(i);
				}
				
				
				if (NextRestock > m_Trader_Restock_moddedTimes.Get(i))
					NextRestock = m_Trader_Restock_moddedTimes.Get(i);
			}
		}
		if (m_Trader_RestockTimeSum >= HighestRestockTimer)
		{
			for (i = 0; i < m_Trader_Restock_moddedTimes.Count(); i++)
			{
				if (m_Trader_Restock_moddedTimes.Get(i) == -1)
					continue;
				
				m_Trader_Restock_moddedTimes[i] = m_Trader_Restock_moddedTimes.Get(i) - m_Trader_RestockTimeSum;
			}
			NextRestock -= m_Trader_RestockTimeSum;
			m_Trader_RestockTimeSum = 0;
		}
		SaveTimeStamp();
	}
	
	void RestockItem(int itemID)
	{
		int traderUID = m_Trader_ItemsTraderId.Get(itemID);
		string itemType = m_Trader_ItemsClassnames.Get(itemID);
		if (itemType.Contains("MoneyRuble"))
			return;
		ref array<string> ItemConfig = LoadStockData(itemID);
		float Stock = ItemConfig.Get(0).ToFloat();
		string Nominal = ItemConfig.Get(1);
		string BasePrice = ItemConfig.Get(2);
		string BaseSellPrice = ItemConfig.Get(3);
		string RealNominal = ItemConfig.Get(4);
		string excludeFromRestock = ItemConfig.Get(5);
		int ItemRestockTimer = ItemConfig.Get(6).ToInt();
		float ItemRestockPercent = ItemConfig.Get(7).ToFloat();
		
		float Percent = 0.00;
		
		if (excludeFromRestock == "1")
			return;
		
		if (!ItemRestockTimer > 0 && m_Trader_Restock_Times.Get(itemID) != m_Trader_DefaultRestockTime)
			m_Trader_Restock_Times[itemID] = m_Trader_DefaultRestockTime;
		else
			if (m_Trader_Restock_Times.Get(itemID) != ItemRestockTimer)
				m_Trader_Restock_Times[itemID] = ItemRestockTimer;
		
		if (ItemRestockPercent > 0)
			Percent = ItemRestockPercent;
		else
			Percent = RestockPercent;
			
		float value = (Nominal.ToFloat() - Stock) * Percent;
		
		if (value > 0 && value < 1)
			value += Math.RandomFloatInclusive(0.0, 1.0);
		if (value < 0 && value > -1)
			value -= Math.RandomFloatInclusive(0.0, 1.0);
		
		int newStock = Stock + value;
		
		//TraderMessage.ServerLog("[TRADER] DEBUG5: " + Stock.ToString() + ", " + Nominal + ", " + BasePrice + ", " + BaseSellPrice + ", " + RealNominal);
		
		if (newStock != Stock)
		{
			
			//TraderMessage.ServerLog("[TRADER] DEBUG6: (newStock != Stock) -> true");
			SaveStockData(itemID, newStock.ToString(), Nominal, BasePrice, BaseSellPrice, RealNominal, excludeFromRestock, ItemRestockTimer.ToString(), ItemRestockPercent.ToString());
			
			//TraderMessage.ServerLog("[TRADER] DEBUG7: ID = " + ID);
			if (itemID != -1)
			{
				m_Trader_ItemsBuyValue[itemID] = GetBuyPrice(newStock, Nominal.ToInt(), BasePrice.ToInt());
				if (BaseSellPrice != "A")
					m_Trader_ItemsSellValue[itemID] = GetSellPrice(newStock, Nominal.ToInt(), BaseSellPrice.ToInt(), false);
				else
					m_Trader_ItemsSellValue[itemID] = GetSellPrice(newStock, Nominal.ToInt(), BasePrice.ToInt(), true);
				TraderSendPriceUpdate(itemID);
			}
		}
	
	}
	
	void LoadTimeStamp()
	{
		string line_content;
		FileHandle file_index = OpenFile(m_Trader_StockDirPath + "RestockTimeStamp" + ".txt", FileMode.READ);
		if ( file_index == 0 )
		{
			m_Trader_RestockTimeSum = 0;
			return;
		}
		int char_count = FGets(file_index, line_content);
		float TimeStamp = line_content.ToFloat();
		m_Trader_RestockTimeSum = TimeStamp;
		for (int i = 0; i < m_Trader_Restock_moddedTimes.Count(); i++)
		{
			if (m_Trader_Restock_moddedTimes.Get(i) == -1)
				continue;
			
			while (m_Trader_RestockTimeSum > m_Trader_Restock_moddedTimes.Get(i))
			{
				m_Trader_Restock_moddedTimes[i] = m_Trader_Restock_moddedTimes.Get(i) + m_Trader_Restock_Times.Get(i);
			}
			
			if (NextRestock > m_Trader_Restock_moddedTimes.Get(i))
				NextRestock = m_Trader_Restock_moddedTimes.Get(i);
		}
	}
	
	void SaveTimeStamp()
	{
		FileHandle file_index = OpenFile(m_Trader_StockDirPath + "RestockTimeStamp" + ".txt", FileMode.WRITE);
		FPrintln(file_index, m_Trader_RestockTimeSum.ToString());
	}
	
	array<string> LoadStockData(int itemID, int traderUID = -1, string itemType = "NULL")
	{
		ref array<string> ItemConfig = new array<string>;
		ItemConfig.Insert("NULL");
		ItemConfig.Insert("NULL");
		ItemConfig.Insert("NULL");
		ItemConfig.Insert("NULL");
		ItemConfig.Insert("NULL");
		ItemConfig.Insert("NULL");
		ItemConfig.Insert("NULL");
		ItemConfig.Insert("NULL");
		
		if (itemID >= 0)
		{
			traderUID = m_Trader_ItemsTraderId.Get(itemID);
			itemType = m_Trader_ItemsClassnames.Get(itemID);
		}
		
		if (traderUID != -1 && itemType != "NULL")
		{
			FileHandle file_index;
			int char_count = 0;
			string line_content = "";
					
			file_index = OpenFile(m_Trader_StockDirPath + traderUID.ToString() + "_" + itemType + ".txt", FileMode.READ);
			if (file_index == 0)
			{
				TraderMessage.ServerLog("!!! ERROR COULDN'T FIND " + m_Trader_StockDirPath + traderUID.ToString() + "_" + itemType + ".txt!!!");
				TraderMessage.ServerLog("traderUID = " + traderUID.ToString());
				TraderMessage.ServerLog("itemType = " + itemType);
				TraderMessage.ServerLog("itemID = " + itemID.ToString());
			}
			char_count = FGets(file_index, line_content);
			while (char_count != -1 && !line_content.Contains("<FileEnd>"))
			{
				if (line_content.Contains("<Stock>"))
				{
					line_content.Replace("<Stock>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					
					ItemConfig[0] = line_content;
				}
				
				if (line_content.Contains("<BalancedStock>"))
				{
					line_content.Replace("<BalancedStock>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					
					ItemConfig[1] = line_content;
				}
				
				if (line_content.Contains("<BasePrice>"))
				{
					line_content.Replace("<BasePrice>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					
					ItemConfig[2] = line_content;
				}
				
				if (line_content.Contains("<BaseSellPrice>"))
				{
					line_content.Replace("<BaseSellPrice>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					
					ItemConfig[3] = line_content;
				}
				
				if (line_content.Contains("<RealNominal>"))
				{
					line_content.Replace("<RealNominal>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					
					ItemConfig[4] = line_content;
				}
			
				if (line_content.Contains("<ExcludeFromRestock>"))
				{
					line_content.Replace("<ExcludeFromRestock>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					
					ItemConfig[5] = line_content;
				}
							
				if (line_content.Contains("<RestockTimer>"))
				{
					line_content.Replace("<RestockTimer>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					
					ItemConfig[6] = line_content;
				}
				
				if (line_content.Contains("<RestockPercent>"))
				{
					line_content.Replace("<RestockPercent>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					
					ItemConfig[7] = line_content;
				}
				
				char_count = FGets(file_index, line_content);
			}
			CloseFile(file_index);
		}
		
		for (int i = 0; i < ItemConfig.Count(); i++)
		{
			if (ItemConfig.Get(i) == "NULL")
				ItemConfig[i] = m_Trader_Economy_DefaultItemValues.Get(i);
		}
		
		
		return ItemConfig;
	}
	
	void SaveStockData(int itemID, string Stock, string Nominal, string BasePrice, string BaseSellPrice, string RealNominal, string excludeFromRestock, string ItemRestockTimer, string ItemRestockPercent)
	{
		int traderUID = m_Trader_ItemsTraderId.Get(itemID);
		string itemType = m_Trader_ItemsClassnames.Get(itemID);
		
		FileHandle file_index = OpenFile(m_Trader_StockDirPath + traderUID.ToString() + "_" + itemType + ".txt", FileMode.WRITE);
		
		FPrintln(file_index, "<Stock> " + Stock);
		FPrintln(file_index, "<BalancedStock> " + Nominal);
		FPrintln(file_index, "<BasePrice> " + BasePrice);
		FPrintln(file_index, "<BaseSellPrice> " + BaseSellPrice);
		FPrintln(file_index, "<RealNominal> " + RealNominal);
		FPrintln(file_index, "<ExcludeFromRestock> " + excludeFromRestock);
		FPrintln(file_index, "<RestockTimer> " + ItemRestockTimer);
		FPrintln(file_index, "<RestockPercent> " + ItemRestockPercent);
		FPrintln(file_index, "<FileEnd>");
			
		CloseFile(file_index);
	}
	

	
	//void SendPriceUpdatesTo(PlayerBase player)
	//{
	//	Param1<bool> crpPin = new Param1<bool>( true );
	//	GetGame().RPCSingleParam(player, TPRPCs.RPC_SEND_TRADER_PRICES_UPDATE_INIT, crpPin, true, player.GetIdentity());
	//	int i = 0;
	//	for ( i = 0; i < m_Trader_ItemsClassnames.Count(); i++ )
	//	{
	//		player.m_Trader_ItemsBuyValue.Insert(m_Trader_ItemsBuyValue.Get(i));
	//		player.m_Trader_ItemsSellValue.Insert(m_Trader_ItemsSellValue.Get(i));

	//		Param2<int, int> crp6 = new Param2<int, int>( m_Trader_ItemsBuyValue.Get(i), m_Trader_ItemsSellValue.Get(i) );
	//		GetGame().RPCSingleParam(player, TPRPCs.RPC_SEND_TRADER_PRICES_UPDATE, crp6, true, player.GetIdentity());
	//		//TraderMessage.ServerLog("[TRADER] ITEMENTRY: " + m_Trader_ItemsTraderId.Get(i) + ", " + m_Trader_ItemsCategoryId.Get(i) + ", " + m_Trader_ItemsClassnames.Get(i) + ", " + m_Trader_ItemsQuantity.Get(i) + ", " + m_Trader_ItemsBuyValue.Get(i) + ", " + m_Trader_ItemsSellValue.Get(i));
	//	}
	//	Param1<bool> crpRM = new Param1<bool>( true );
	//	GetGame().RPCSingleParam(player, TPRPCs.RPC_SEND_TRADER_PRICES_UPDATE_REFRESH, crpRM, true, player.GetIdentity());
	//}
	
	
}