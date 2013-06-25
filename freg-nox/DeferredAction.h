enum deferred_actions {
	DEFERRED_NOTHING,
	DEFERRED_MOVE,
	DEFERRED_JUMP,
	DEFERRED_BUILD,
	DEFERRED_DAMAGE,
	DEFERRED_THROW
}; //enum deferred_actions

class Block;
class Active;
class World;

class DeferredAction {
	private:
	int type;
	Active * const attachedBlock;
	ushort xTarg, yTarg, zTarg;
	Block * material;
	ushort srcSlot, destSlot;
	ushort num;
	World * const world;

	void Move() const;
	void Jump() const;
	void Build();
	void Damage() const;
	void Throw() const;

	public:
	void SetMove();
	void SetJump();
	void SetBuild(ushort x_targ, ushort y_targ, ushort z_targ,
			Block * material,
			ushort builder_slot);
	void SetDamage(ushort x_targ, ushort y_targ, ushort z_targ);
	void SetThrow(ushort src_slot, ushort dest_slot, ushort num);

	World * GetWorld() const;
	int  GetActionType() const;
	void MakeAction();

	DeferredAction(Active * attached, World *);
}; //class DeferredAction
