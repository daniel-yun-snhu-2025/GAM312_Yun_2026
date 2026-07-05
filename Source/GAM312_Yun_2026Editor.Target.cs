using UnrealBuildTool;
using System.Collections.Generic;

public class GAM312_Yun_2026EditorTarget : TargetRules
{
	public GAM312_Yun_2026EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("GAM312_Yun_2026");
	}
}
