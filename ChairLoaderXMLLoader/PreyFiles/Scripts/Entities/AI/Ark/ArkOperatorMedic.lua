Script.ReloadScript("SCRIPTS/Entities/AI/Ark/ArkOperatorMedic_x.lua")
Script.ReloadScript("SCRIPTS/Entities/actor/BasicActor.lua")
Script.ReloadScript("SCRIPTS/Entities/AI/Shared/BasicAI.lua")
Script.ReloadScript("SCRIPTS/Entities/actor/ArkNpc.lua")
Script.ReloadScript("SCRIPTS/Entities/AI/Ark/ArkOperator.lua")
CreateActorForAISubclass(ArkOperatorMedic_x)
CreateArkNpc(ArkOperator_x)
ArkOperatorMedic = CreateAISubclass(ArkOperatorMedic_x, ArkOperator_x)
InitActorForAISubclass(ArkOperatorMedic_x)
ArkOperatorMedic:Expose()