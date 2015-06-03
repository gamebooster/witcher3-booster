import function ShowHud( show : Bool);  // flags: 4098 0000008D1AC6A110
import function HudConsoleMsg( msgText : String);  // flags: 258 0000008D1AC82380
import function spawn( nam : CName,  optional  quantity : Int32,  optional  distance : Float,  optional  isHostile : Bool);  // flags: 258 0000003B24D35F40
import function GetActorsInRange(center : CNode,  range : Float,  optional  maxResults : Int32,  optional  tag : CName,  optional  onlyAlive : Bool) : array<CActor>;  // flags: 2 0000003B24D079F0

import class CGameplayEntity extends CPeristentEntity {}

import class CActor extends CGameplayEntity {
  // import var actorGroups : EPathEngineAgentType; // 4001
  // import var aimOffset : Float; // 4001
  // import var barOffset : Float; // 4001
  // import var isAttackableByPlayer : Bool; // e009
  // import var losTestBoneIndex : Int32; // 4008
  // import var attackTarget : CActor; // 4008
  // import var attackTargetSetTime : EngineTime; // 4008
  // import var frontPushAnim : CName; // 4001
  // import var backPushAnim : CName; // 4001
  // import var isCollidable : Bool; // 4001
  // import var isVisibileFromFar : Bool; // 4001
  // import var voiceTag : CName; // 4003
  // import var voiceToRandomize : array<StringAnsi>; // 4001
  // import var behTreeMachine  : *CBehTreeMachine; // 4008
  // import var useHiResShadows : Bool; // 4009
  // import var pelvisBoneName : CName; // 4001
  // import var torsoBoneName : CName; // 4001
  // import var headBoneName : CName; // 4001
  // import var useAnimationEventFilter : Bool; // 4001
  // import var encounterGroupUsedToSpawn : Int32; // c000

  // public function GetAttitude( actor : CActor) : EAIAttitude;  // flags: 65561 0000003B16EBD680
}
import class CPlayer extends CActor {}
import class CNewNPC extends CActor {
  import public var aiEnabled : bool;
  import public var berserkTime : EngineTime;
  import public var npcGroupType : ENPCGroupType;
  
  public function Kill( optional  ignoreImmortalityMode : Bool,  optional  attacker : CGameplayEntity,  optional  source : CName);  // flags: 65536 0000003B23503AB0
}

import class CGame extends CObject {}
import class CCommonGame extends CGame {
  import public var player : CPlayer;
}
import class CR4Game extends CCommonGame {}

exec function showhud() {
  ShowHud(true);
  HudConsoleMsg("showhud");
}

exec function hidehud() {
  ShowHud(false);
  HudConsoleMsg("hidehud");
}

exec function spawnpigs() {
  var pig_name : name;
  var b : bool;
  
  // pig_name = "pig"; Compiler expects '' for CName
  pig_name = 'pig';
  spawn(pig_name, 5, 5, true);
  spawn(pig_name, 5);
}

exec function mykillall(optional range: Float) {
	var enemies : array<CActor>;
	var i : int;
	var enemiesSize : int;
	var npc : CNewNPC;
	if (range <= 0.000000) {
	  range = 20.000000;
  }
	enemies = GetActorsInRange(theGame.player, range,,,); // possible to use empty commas for optional arguments
  HudConsoleMsg("got enemies tst " + enemies.Size());
	enemiesSize = enemies.Size();
	for (i = 0; i < enemiesSize; i+=1) {
	  npc = (CNewNPC)enemies[i];
	  if (npc) {// && npc.GetAttitude(theGame.player) == AIA_Hostile) { // GetAttitude crashes because CActor is not complete without *CBehTreeMachine
	    npc.Kill(true,,);                                               // but not compilable without syntax for pointers
    }
  }
}