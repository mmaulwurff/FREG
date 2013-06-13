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

	void Move();
	void Jump();
	void Build();
	void Damage();
	void Throw();

	World * GetWorld() const;

	public:
	void SetMove();
	void SetJump();
	void SetBuild(ushort x_targ, ushort y_targ, ushort z_targ,
			Block * material,
			ushort builder_slot);
	void SetDamage(ushort x_targ, ushort y_targ, ushort z_targ);
	void SetThrow(ushort src_slot, ushort dest_slot, ushort num);

	int  GetActionType() const;
	void MakeAction();

	DeferredAction(Active * attached);
}; //class DeferredAction
