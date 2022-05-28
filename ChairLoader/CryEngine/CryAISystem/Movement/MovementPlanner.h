// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#ifndef MovementPlanner_h
	#define MovementPlanner_h

	#include "MovementPlan.h"
	#include <CryAISystem/MovementRequest.h>

struct MovementActor;
struct MovementUpdateContext;

namespace Movement
{
// The planner takes a movement request and tries to come up with
// a plan to satisfy it. It will also execute the plan and report
// when it has been satisfied.
struct IPlanner
{
	class Status
	{
	public:
		Status()
			: m_requestSatisfied(false)
			, m_pathfinderFailed(false)
			, m_movingAlongPathFailed(false)
			, m_reachedMaxNumberOfReplansAllowed(false)
		{}

		void SetRequestSatisfied()                              { m_requestSatisfied = true; }
		void SetPathfinderFailed()                              { m_pathfinderFailed = true; }
		void SetMovingAlongPathFailed()                         { m_movingAlongPathFailed = true; }
		void SetReachedMaxAllowedReplans()                      { m_reachedMaxNumberOfReplansAllowed = true; }

		bool HasRequestBeenSatisfied() const                    { return m_requestSatisfied; }
		bool HasPathfinderFailed() const                        { return m_pathfinderFailed; }
		bool HasMovingAlongPathFailed() const                   { return m_movingAlongPathFailed; }
		bool HasReachedTheMaximumNumberOfReplansAllowed() const { return m_reachedMaxNumberOfReplansAllowed; }

	private:
		bool m_requestSatisfied;
		bool m_pathfinderFailed;
		bool m_movingAlongPathFailed;
		bool m_reachedMaxNumberOfReplansAllowed;
	};

	virtual ~IPlanner() {}

	// The controller will be updated and kept alive while this is true.
	virtual bool IsUpdateNeeded() const = 0;

	// Called when the movement system has some work for the planner.
	// Only called when 'IsReadyForNewRequest' returns true.
	virtual void StartWorkingOnRequest(const MovementRequest& request, const MovementUpdateContext& context) = 0;

	// The movement system calls this when the actor is no longer
	// interested in satisfying the request the planner is working on.
	// Note: This doesn't mean that the actor wants to stop! The planner
	// may continue executing the plan until a new request comes in.
	virtual void CancelCurrentRequest(MovementActor& actor) = 0;

	// Do some work and report back the 'Status' of the current request.
	virtual Status Update(const MovementUpdateContext& context) = 0;

	// Checked before 'StartWorkingOnRequest' is called. The planner might
	// be in the middle of something and mustn't be disturbed right now.
	virtual bool IsReadyForNewRequest() const = 0;

	// Fill in the 'status' object and explain what's currently
	// being worked on. It's very handy for debugging.
	virtual void GetStatus(MovementRequestStatus& status) const = 0;
};

// This generic movement planner will be able to these satisfy requests:
// - Move to position in open space
// - Move to cover
// - Move along path
// It will take care of leaving the cover before moving to a position.
class GenericPlanner : public IPlanner
{
public:
	explicit GenericPlanner(NavigationAgentTypeID navigationAgentTypeID);
	~GenericPlanner();
	virtual bool   IsUpdateNeeded() const override;
	virtual void   StartWorkingOnRequest(const MovementRequest& request, const MovementUpdateContext& context) override;
	virtual void   CancelCurrentRequest(MovementActor& actor) override;
	virtual Status Update(const MovementUpdateContext& context) override;
	virtual bool   IsReadyForNewRequest() const override;
	virtual void   GetStatus(MovementRequestStatus& status) const override;

private:
	void CheckOnPathfinder(const MovementUpdateContext& context, OUT Status& status);
	void ExecuteCurrentPlan(const MovementUpdateContext& context, OUT Status& status);
	void ProducePlan(const MovementUpdateContext& context);
	void ProduceMoveToPlan(const MovementUpdateContext& context);
	void ProduceStopPlan(const MovementUpdateContext& context);
	bool CanReplan(const MovementRequest& request) const;
	void StartWorkingOnRequest_Internal(const MovementRequest& request, const MovementUpdateContext& context);
	void OnNavigationMeshChanged(NavigationAgentTypeID navigationAgentTypeID, NavigationMeshID meshID, uint32 tileID);

private:
	/// Tiles that were affected by NavMesh changes
	struct MeshIDAndTileID
	{
		const NavigationMeshID meshID;
		const MNM::TileID      tileID;

		explicit MeshIDAndTileID(NavigationMeshID _meshID, const MNM::TileID _tileID) : meshID(_meshID), tileID(_tileID) {}
		bool operator==(const MeshIDAndTileID& rhs) const { return (this->tileID == rhs.tileID) && (this->meshID == rhs.meshID); }
	};

	const NavigationAgentTypeID  m_navigationAgentTypeID;
	std::vector<MeshIDAndTileID> m_queuedNavMeshChanges;
	bool                         m_pendingPathReplanningDueToPreviousNavMeshChanges; // dirty-flag to automatically re-path as soon as the possibly existing plan allows for it again
	Plan                         m_plan;
	MovementRequest              m_request;
	uint8                        m_amountOfFailedReplanning;
	bool                         m_pathfinderRequestQueued;
	const static uint8           s_maxAllowedReplanning = 3;
};
}

#endif // MovementPlanner_h
