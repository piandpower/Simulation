// Copyright 2018 Michal Cieciura. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
    public class PointCloudEditor : ModuleRules
    {
        public PointCloudEditor(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "EditorStyle",
                "KismetWidgets",
                "PropertyEditor",
                "ApplicationCore",
                "InputCore",
                "MainFrame",
                "PointCloudRuntime",
                "Projects",
                "UMG",
                "Core",
                "CoreUObject",
                "Engine",
                "RenderCore",
                "ShaderCore",
                "RHI"
                }
            );
        }
    }
}