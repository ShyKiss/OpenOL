/**
 *
 * Copyright 1998-2011 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;

namespace UnrealBuildTool
{
	class UE3BuildOLGame : UE3BuildGame
	{
        /** Returns the singular name of the game being built ("OLGame", "UDKGame", etc) */
		public string GetGameName()
		{
			return "OLGame";
		}
		
		/** Returns a subplatform (e.g. dll) to disambiguate object files */
		public string GetSubPlatform()
		{
			return ( "" );
		}

		/** Get the desired OnlineSubsystem. */
		public string GetDesiredOnlineSubsystem( CPPEnvironment CPPEnv, UnrealTargetPlatform Platform )
		{
			string ForcedOSS = UE3BuildTarget.ForceOnlineSubsystem( Platform );
			if( ForcedOSS != null )
			{
				return ( ForcedOSS );
			}

            if (Platform == UnrealTargetPlatform.Mac)
            {
                return ("PC");
            }
            else if (Platform == UnrealTargetPlatform.Orbis)
            {
                return ("NP");
            }
            else if (Platform == UnrealTargetPlatform.Dingo)
            {
                return ("Dingo");
            }
            else
            {
                return ("Steamworks");
            }
		}

		/** Returns true if the game wants to have PC ES2 simulator (ie ES2 Dynamic RHI) enabled */
		public bool ShouldCompileES2()
		{
			return false;
		}

        /** Returns whether PhysX should be compiled on mobile platforms */
        public bool ShouldCompilePhysXMobile()
        {
            return UE3BuildConfiguration.bCompilePhysXWithMobile;
        }

        /** Allows the game add any global environment settings before building */
        public void GetGameSpecificGlobalEnvironment(CPPEnvironment GlobalEnvironment, UnrealTargetPlatform Platform)
        {
            GlobalEnvironment.IncludePaths.Add("UDKBase/Inc");
            GlobalEnvironment.IncludePaths.Add("OLGame/Inc");

            if (UE3BuildConfiguration.bBuildEditor &&
                (GlobalEnvironment.TargetPlatform == CPPTargetPlatform.Win32 || GlobalEnvironment.TargetPlatform == CPPTargetPlatform.Win64))
            {
                GlobalEnvironment.IncludePaths.Add("OLEditor/Inc");
            }
        }
		
		/** Allows the game to add any Platform/Configuration environment settings before building */
        public void GetGameSpecificPlatformConfigurationEnvironment(CPPEnvironment GlobalEnvironment, LinkEnvironment FinalLinkEnvironment)
        {

        }

        /** Returns the xex.xml file for the given game */
		public FileItem GetXEXConfigFile()
		{
			return FileItem.GetExistingItemByPath("OLGame/Live/xex.xml");
		}

        /** Allows the game to add any additional environment settings before building */
		public void SetUpGameEnvironment(CPPEnvironment GameCPPEnvironment, LinkEnvironment FinalLinkEnvironment, List<UE3ProjectDesc> GameProjects)
		{
			GameProjects.Add(new UE3ProjectDesc("UDKBase/UDKBase.vcxproj"));
            GameCPPEnvironment.IncludePaths.Add("UDKBase/Inc");
	
			GameProjects.Add(new UE3ProjectDesc("OLGame/OLGame.vcxproj"));
            GameCPPEnvironment.IncludePaths.Add("OLGame/Inc");
            GameCPPEnvironment.IncludePaths.Add("Multiplayer/Inc");
            GameCPPEnvironment.IncludePaths.Add("TASGame/Inc");

			if (UE3BuildConfiguration.bBuildEditor &&
				(GameCPPEnvironment.TargetPlatform == CPPTargetPlatform.Win32 || GameCPPEnvironment.TargetPlatform == CPPTargetPlatform.Win64))
			{
 				GameProjects.Add(new UE3ProjectDesc("OLEditor/OLEditor.vcxproj"));
 				GameCPPEnvironment.IncludePaths.Add("OLEditor/Inc");
			}

			GameCPPEnvironment.Definitions.Add("GAMENAME=OLGAME");
			GameCPPEnvironment.Definitions.Add("IS_OLGAME=1");

		}
	}
}
