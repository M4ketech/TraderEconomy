class CfgPatches
{
	class TraderEconomy
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"Trader"
		};
	};
};

class CfgMods
{
	class TraderEconomy
	{
		
		dir = "TraderEconomy";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "";
		credits = "";
		author = "Astral";
		authorID = "0"; 
		version = "0.2"; 
		extra = 0;
		type = "mod";
		dependencies[] = 
		{
			"Game",
			"World",
			"Mission"
		};
		
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] =
				{
					"TraderEconomy/scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value = "";
				files[] =
				{ 
					"TraderEconomy/scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value = "";
				files[] = 
				{
					"TraderEconomy/scripts/5_Mission"
				};
			};
		};
	};
};