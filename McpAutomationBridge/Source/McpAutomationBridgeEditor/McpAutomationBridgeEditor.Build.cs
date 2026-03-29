using UnrealBuildTool;

public class McpAutomationBridgeEditor : ModuleRules
{
	public McpAutomationBridgeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"McpAutomationBridge"
		});

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
			"DetailCustomizations",
			"Json",
			"JsonUtilities",
			"Sockets",
			"Networking",
			"HTTPServer",
			"Slate",
			"SlateCore",
			"ToolMenus",
			"PropertyEditor",
			"ProjectSettingsViewer",
			"UMG",
			"UMGEditor",
			"MaterialEditor",
			"AnimGraph",
			"AnimGraphRuntime",
			"LevelSequence",
			"MovieScene",
			"MovieSceneTracks",
			"CinematicCamera",
			"Niagara",
			"NiagaraEditor"
		});

		bEnableExceptions = true;
	}
}
