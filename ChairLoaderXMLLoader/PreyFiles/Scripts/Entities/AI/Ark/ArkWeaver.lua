Script.ReloadScript("SCRIPTS/Entities/AI/Ark/ArkWeaver_x.lua")
Script.ReloadScript("SCRIPTS/Entities/actor/BasicActor.lua")
Script.ReloadScript("SCRIPTS/Entities/AI/Shared/BasicAI.lua")
Script.ReloadScript("SCRIPTS/Entities/actor/ArkNpc.lua")
CreateActor(ArkWeaver_x)
CreateArkNpc(ArkWeaver_x)
ArkWeaver = CreateAI(ArkWeaver_x)
ArkWeaver:Expose()
