﻿//
// ConvexVolumeTool.h
//

#pragma once

#include "NavMeshTool.h"
#include "common/NavMeshData.h"

#include <glm/glm.hpp>
#include <vector>

class ConvexVolumeToolState;

class ConvexVolumeTool : public Tool
{
public:
	ConvexVolumeTool();
	virtual ~ConvexVolumeTool();

	virtual ToolType type() const override { return ToolType::CONVEX_VOLUME; }
	virtual void init(NavMeshTool* meshTool) override;
	virtual void reset() override;
	virtual void handleMenu() override;
	virtual void handleClick(const glm::vec3& s, const glm::vec3& p, bool shift) override;
	virtual void handleToggle() override {}
	virtual void handleStep() override {}
	virtual void handleUpdate(float dt) override {}
	virtual void handleRender() override;
	virtual void handleRenderOverlay(const glm::mat4& proj,
		const glm::mat4& model, const glm::ivec4& view) override;

private:
	NavMeshTool* m_meshTool = nullptr;
	ConvexVolumeToolState* m_state = nullptr;

};

class ConvexVolumeToolState : public ToolState
{
	friend class ConvexVolumeTool;

public:
	virtual void init(NavMeshTool* meshTool) override;
	virtual void reset() override;
	virtual void handleRender() override;
	virtual void handleRenderOverlay(const glm::mat4& proj,
		const glm::mat4& model, const glm::ivec4& view) override;
	virtual void handleUpdate(const float dt) override {}

	std::vector<dtTileRef> handleVolumeClick(const glm::vec3& p, bool shift);

private:
	NavMeshTool* m_meshTool = nullptr;

	PolyArea m_areaType = PolyArea::Ground;
	float m_polyOffset = 0.0f;
	float m_boxHeight = 6.0f;
	float m_boxDescent = 1.0f;

	std::vector<glm::vec3> m_pts;
	std::vector<uint32_t> m_hull;
};