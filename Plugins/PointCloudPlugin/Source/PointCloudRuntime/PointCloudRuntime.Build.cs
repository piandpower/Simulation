// Copyright 2018 Michal Cieciura. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
    public class PointCloudRuntime : ModuleRules
    {
        public PointCloudRuntime(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Core",
                "CoreUObject",
                "Engine",
                "RenderCore",
                "ShaderCore",
                "RHI"
                }
            );

            if (Target.bBuildEditor)
            {
                PrivateDependencyModuleNames.Add("UnrealEd");
            }
        }
    }
}