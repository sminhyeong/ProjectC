// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ProjectC : ModuleRules
{
	public ProjectC(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Networking", "Sockets", "NetCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PublicIncludePaths.AddRange(new string[] { "flatbuffers" });   //flatbuffers, etc

        // 유니코드 설정 추가
        PublicDefinitions.Add("UNICODE=1");
        PublicDefinitions.Add("_UNICODE=1");

        // 또는 MBCS를 명시적으로 비활성화
        PublicDefinitions.Add("_MBCS=0");

        // Windows 플랫폼에서 컴파일러 경고 억제
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS");
        }

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
