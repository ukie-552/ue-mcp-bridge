using UnrealBuildTool;

public class McpAutomationBridge : ModuleRules
{
	public McpAutomationBridge(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Json",
			"JsonUtilities",
			"Sockets",
			"Networking",
			"WebSockets"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"EditorSubsystem",
				"AssetRegistry",
				"BlueprintGraph",
				"Kismet",
				"KismetCompiler",
				"GraphEditor",
				"EditorScriptingUtilities",
				"LevelEditor",
				"DetailCustomizations"
			});
		}

		bEnableExceptions = true;
	}
}
