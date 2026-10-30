class OLAICmd_MoveAbility_Door extends OLAICmd_MoveAbility 
	within OLBot
	native(AI);

var OLDoor Door;
var vector LastDestination;

var int BashIter;

static event OLAICmd_MoveAbility_Door MoveThruDoor( OLBot Bot, OLDoor InDoor, vector InitialDestination, bool bInReversed )
{
	local OLAICmd_MoveAbility_Door Cmd;

	if ( Bot != None )
	{
		Cmd = new(Bot) default.class;
		if ( Cmd != None )
		{
			Cmd.Door = InDoor;
			Cmd.CurrentDestination = InitialDestination;
			Cmd.bReversed = bInReversed;
			Cmd.WaitPointComponent = InDoor.WaitPointComponent;

			return Cmd;
		}
	}

	return None;
}

function bool IsDoorOpen()
{
	return Door.GetOpenAngle() > 80.0f || Door.IsBroken();
}

function bool ModifyApproach(out vector NewDestination)
{
	if (IsDoorOpen())
	{
		return GetDoorApproachPoint(Door, NewDestination);
	}
	
	return false;
}

function Popped()
{
	Super.Popped();

	ActiveDoor = None;
	bCancelBash = FALSE;
}

function Cancel()
{
	`AILog("MoveAbility Cancelled - Door", 'AIState');

	bMoveCancelled = TRUE;

	Recalculate();

	Completed();
}

function bool IsObjectInUse()
{
	return Door.Bots.length > 1; // We are in this list so we want to know if there are others.
}

function RegisterWithObject()
{
	local bool bFound;
	local int Idx;

	bFound = false;
	for (Idx = 0; Idx < Door.Bots.length; ++Idx)
	{
		if (Door.Bots[Idx] == OLBot(GameAIOwner))
		{
			bFound = true;
		}
	}
	
	if (!bFound)
	{
		Door.Bots.AddItem(OLBot(GameAIOwner));
	}
}

function UnregisterWithObject()
{
	Door.Bots.RemoveItem(OLBot(GameAIOwner));

	if (Door.ActiveBot == OLBot(GameAIOwner))
	{
		Door.ActiveBot = None;
	}
}

function OLBot GetClosestToObject()
{
	local int Idx;
	local OLBot TempClosestBot;

	for (Idx = 0; Idx < Door.Bots.length; ++Idx)
	{
		if (TempClosestBot == None)
		{
			TempClosestBot = Door.Bots[Idx];
		}
		else
		{
			if (VSizeSq(TempClosestBot.EnemyPawn.Location - Door.ScriptGetCenterLocation())
				> VSizeSq(Door.Bots[Idx].EnemyPawn.Location - Door.ScriptGetCenterLocation()))
			{
				TempClosestBot = Door.Bots[Idx];
			}
		}
	}

	return TempClosestBot;
}

function OLBot GetActiveOnObject()
{
	return Door.ActiveBot;
}

function SetActiveOnObject(OLBot NewActiveBot)
{
	Door.ActiveBot = NewActiveBot;
}

function array<OLBot> GetBotsOnObject()
{
	return Door.Bots;
}

state Performing
{
	function BeginState(Name PreviousStateName)
	{
		ActiveDoor = Door;
	}

	function EndState(Name NextStateName)
	{
		ActiveDoor = None;
		bCancelBash = FALSE;
	}

Begin:
	`AILog("MoveAbility Performing - Door", 'AIState');

	if (!IsImprovedAI())
	{
		while (Door.DoorState == DS_Opening || Door.DoorState == DS_Closing)
		{
			Sleep(0.1f);
		}
	}

	if (Door.bBlocked || (Door.DoorUser != None && (!IsImprovedAI() || OLHero(Door.DoorUser) == None)))
	{
		Cancel();
	}

	if (!IsDoorOpen())
	{
		ClearDestination();

		//EnemyPawn.SetLocation(CurrSMDestination);
		LastDestination = CurrentDestination;
		CurrentDestination = bReversed ? Door.Edge1Dest : Door.Edge0Dest;

		EnemyPawn.TurnOnSpot(rotator(CurrentDestination - EnemyPawn.Location));
		WaitForSpecialMove();

		EnemyPawn.SetDesiredRotation(rotator(CurrentDestination - EnemyPawn.Location), true, , ,false);
		FinishRotation();

		if (Door.bBlocked || (Door.DoorUser != None && (!IsImprovedAI() || OLHero(Door.DoorUser) == None)))
		{
			EnemyPawn.LockDesiredRotation(false);
			Cancel();
		}

		StartDoorTraversal( bReversed );

		LastDestination.Z = EnemyPawn.Location.Z;
		
		if (EnemyPawn.IsA('OLEnemyNanoCloud'))
		{
			Door.bAITraversing = true;

			EnemyPawn.StartSpecialMove(SMT_NanoThroughDoor, LastDestination, Normal(CurrentDestination - LastDestination), APTT_TargetAtStart);
			WaitForSpecialMove();

			Door.bAITraversing = false;
		}
		else
		{
			bBreachingDoor = FALSE;

			if (Door.ShouldBreak(Outer))
			{
				bBreachingDoor = TRUE;

				Door.TriggerEvent(DET_StartedBashing, EnemyPawn);
			}

			if (EnemyPawn.bUsingWeapon && EnemyPawn.bHasWeaponEquipped)
			{
				PlayFullBodyAnim(EnemyPawn.AnimNameUnequipWeapon, 1.0f, 0.1f, 0.1f);
				WaitForFullBodyAnim();
			}

			if (bBreachingDoor && EnemyPawn.bUsesDoorBashLoop)
			{
				EnemyPawn.StartSpecialMove(SMT_BashDoorStart, LastDestination, Normal(CurrentDestination - LastDestination), APTT_TargetAtStart);

				WaitForSpecialMove(true);

				BashIter = 0;
				while (((!Door.bWaitForTriggerToBreak && BashIter < EnemyPawn.NumOfDoorBashLoops && !Door.bBlocked)
					|| (Door.bWaitForTriggerToBreak && !Door.bBreakTriggered))
					&& !Door.bInstantBreak && !bCancelBash)
				{
					EnemyPawn.StartSpecialMove(SMT_BashDoorLoop);

					WaitForSpecialMove(true);
					++BashIter;
				}

				if (!Door.bWaitForTriggerToBreak && Door.bBlocked && !bCancelBash)
				{
					`AILog("Playing Blocked Loop", 'Movement');

					BashIter = 0;
					while (BashIter < Door.NumBashesAfterBlocked)
					{
						EnemyPawn.StartSpecialMove(SMT_BashDoorLoop);
						WaitForSpecialMove(true);
						++BashIter;
					}
				}

				if (Door.bBlocked || bCancelBash)
				{
					EndDoorTraversal();
					EnemyPawn.StartSpecialMove(SMT_BashDoorFailed);
					WaitForSpecialMove();

					if (EnemyPawn.bUsingWeapon && !EnemyPawn.bHasWeaponEquipped)
					{
						PlayFullBodyAnim(EnemyPawn.AnimNameEquipWeapon, 1.0f, 0.1f, 0.1f);
						WaitForFullBodyAnim();
					}

					EnemyPawn.LockDesiredRotation(false);
					Cancel(); // NOTE: This returns from the state
				}
				else
				{
					Door.bAITraversing = true;
					
					EnemyPawn.StartSpecialMove(SMT_BashDoorFinish, Door.Location + Normal(LastDestination - Door.Location) * EnemyPawn.DoorBreakFinishDistance, Normal(CurrentDestination - LastDestination), APTT_TargetAtStart);
					WaitForSpecialMove();

					Door.bAITraversing = false;
				}
			}
			else
			{
				Door.bAITraversing = true;

				EnemyPawn.StartSpecialMove(SMT_EnterDoorInteraction, LastDestination, Normal(CurrentDestination - LastDestination), APTT_TargetAtStart);
				WaitForSpecialMove();

				Door.bAITraversing = false;
			}

			if (EnemyPawn.bUsingWeapon && !EnemyPawn.bHasWeaponEquipped)
			{
				PlayFullBodyAnim(EnemyPawn.AnimNameEquipWeapon, 1.0f, 0.1f, 0.1f);
				WaitForFullBodyAnim();
			}
		}
		
		EndDoorTraversal();

		EnemyPawn.LockDesiredRotation(false);
	}
	else
	{
		StartDoorTraversal( bReversed );

		Door.bAITraversing = true;

		CurrentDestination = bReversed ? Door.Edge1Dest : Door.Edge0Dest;
			
		MoveTo( CurrentDestination, None, -20.0f );

		Door.bAITraversing = false;

		EndDoorTraversal();
	}

	bFinishedDoor = true;
	Completed();
}