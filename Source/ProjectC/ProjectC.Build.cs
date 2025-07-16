// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ProjectC : ModuleRules
{
	public ProjectC(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Networking", "Sockets", "NetCore", "AIModule" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PublicIncludePaths.AddRange(new string[] { "flatbuffers" });   //flatbuffers, etc

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS");
            PublicDefinitions.Add("NOMINMAX");
            PublicDefinitions.Add("WIN32_LEAN_AND_MEAN");
        }

        //// Windows 플랫폼에서 컴파일러 경고 억제
        //if (Target.Platform == UnrealTargetPlatform.Win64)
        //{
        //    PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS");
        //}

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
