// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DenisesVRExpansionExpansion : ModuleRules
{
	public DenisesVRExpansionExpansion(ReadOnlyTargetRules Target) : base(Target)
	{
		//PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		   PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                    "Core",
                    "NetCore",
                    "CoreUObject",
                    "Engine",
					"InputCore",
                    "PhysicsCore",
                    //"FLEX", remove comment if building in the NVIDIA flex branch - NOTE when put in place FLEX only listed win32 and win64 at compatible platforms
                    "HeadMountedDisplay",
					// "RHI",
                    //"RenderCore",
                    //"ShaderCore",
                    //"NetworkReplayStreaming",
                    //"AIModule",
                    "UMG",
                    "NavigationSystem",
                    "AIModule",
                    "AnimGraphRuntime",

                    //"Renderer",
                    //"UtilityShaders,"
					"VRExpansionPlugin",
					"Slate",
					"SlateCore",
					"PropertyEditor",
					"UnrealEd"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// "Core",
				// "CoreUObject",
                //"Engine",
                "InputCore",
                //"FLEX", remove comment if building in the NVIDIA flex branch - NOTE when put in place FLEX only listed win32 and win64 at compatible platforms
                //"HeadMountedDisplay",
                "RHI",
				"ApplicationCore",
                "RenderCore",
				// "ShaderCore",
                "NetworkReplayStreaming",
                "AIModule",
                "UMG",
                "GameplayTags",
                //"Renderer",
				// "UtilityShaders",
				"VRExpansionPlugin"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
