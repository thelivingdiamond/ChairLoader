// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#ifndef MovementPlan_h
	#define MovementPlan_h

struct IMovementActor;
struct MovementUpdateContext;

	#include <CryAISystem/MovementBlock.h>

namespace Movement
{
// This is a plan that will satisfy a movement request such as
// "reach this destination" or "stop immediately".
//
// It's comprised of one or more movement blocks that each take care
// of a certain part of plan. They are executed in order.
//
// As an example a plan to satisfy the request "move to this cover" could
// be comprised of the following movement blocks:
// [FollowPath] -> [UseSmartObject] -> [FollowPath] -> [InstallInCover]
class Plan
{
public:
	static const uint32 NoBlockIndex = ~0u;

	Plan()
		: m_current(NoBlockIndex)
	{
	}

	enum Status
	{
		Running,
		Finished,
		CantBeFinished,
	};

	template<typename BlockType>
	void AddBlock()
	{
		m_blocks.push_back(std::shared_ptr<Block>(new BlockType()));
	}

	void AddBlock(const std::shared_ptr<Block>& block)
	{
		m_blocks.push_back(block);
	}

	Status       Execute(const MovementUpdateContext& context);
	void         ChangeToIndex(const uint newIndex, IMovementActor& actor);
	bool         HasBlocks() const { return !m_blocks.empty(); }
	void         Clear(IMovementActor& actor);
	void         CutOffAfterCurrentBlock();
	bool         InterruptibleNow() const;
	uint32       GetCurrentBlockIndex() const { return m_current; }
	uint32       GetBlockCount() const        { return m_blocks.size(); }
	const Block* GetBlock(uint32 index) const;

private:
	typedef std::vector<std::shared_ptr<Block>> Blocks;
	Blocks m_blocks;
	uint32 m_current;
};
}

#endif // MovementPlan_h
