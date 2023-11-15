// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DenisesVRExpansionExpansion : ModuleRules
{
    public DenisesVRExpansionExpansion(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Common dependencies for both editor and game
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "NetCore",
                "CoreUObject",
                "Engine",
                "InputCore",
                "PhysicsCore",
                "HeadMountedDisplay",
                "UMG",
                "NavigationSystem",
                "AIModule",
                "AnimGraphRuntime",
                "VRExpansionPlugin",
                "Slate",
                "SlateCore",
            }
        );

        // Editor-specific dependencies
        if (Target.Type == TargetRules.TargetType.Editor)
        {
            PublicDependencyModuleNames.Add("PropertyEditor");
        }
		        // Editor-specific dependencies
        if (Target.Type == TargetRules.TargetType.Editor)
        {
            PublicDependencyModuleNames.Add("UnrealEd");
        }

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "InputCore",
                "RHI",
                "ApplicationCore",
                "RenderCore",
                "NetworkReplayStreaming",
                "AIModule",
                "UMG",
                "GameplayTags",
                "VRExpansionPlugin"
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
        );
    }
}
