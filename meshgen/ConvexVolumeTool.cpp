﻿//
// ConvexVolumeTool.cpp
//
#include "ConvexVolumeTool.h"
#include "InputGeom.h"
#include "NavMeshTool.h"
#include "common/Utilities.h"

#include <Recast.h>
#include <RecastDebugDraw.h>
#include <DetourDebugDraw.h>

#include <imgui/imgui.h>
#include <imgui/imgui_custom/imgui_user.h>
#include <glm/gtc/type_ptr.hpp>

// Quick and dirty convex hull.

// Calculates convex hull on xz-plane of points on 'pts',
// stores the indices of the resulting hull in 'out' and
// returns number of points on hull.
static void convexhull(const std::vector<glm::vec3>& pts, std::vector<uint32_t>& out)
{
	// Find lower-leftmost point.
	uint32_t hull = 0;
	for (uint32_t i = 1; i < static_cast<uint32_t>(pts.size()); ++i)
		if (cmppt(pts[i], pts[hull]))
			hull = i;
	// Gift wrap hull.
	uint32_t endpt = 0;
	out.clear();
	do
	{
		out.push_back(hull);
		endpt = 0;
		for (uint32_t j = 1; j < static_cast<uint32_t>(pts.size()); ++j)
			if (hull == endpt || left(pts[hull], pts[endpt], pts[j]))
				endpt = j;
		hull = endpt;
	} while (endpt != out[0]);
}

static int pointInPoly(const std::vector<glm::vec3>& verts, const glm::vec3& p)
{
	bool c = false;

	for (size_t i = 0, j = verts.size() - 1; i < verts.size(); j = i++)
	{
		const glm::vec3& vi = verts[i];
		const glm::vec3& vj = verts[j];

		if (((vi[2] > p[2]) != (vj[2] > p[2])) &&
			(p[0] < (vj[0] - vi[0]) * (p[2] - vi[2]) / (vj[2] - vi[2]) + vi[0]))
		{
			c = !c;
		}
	}
	return c;
}

//----------------------------------------------------------------------------

ConvexVolumeTool::ConvexVolumeTool()
{
}

ConvexVolumeTool::~ConvexVolumeTool()
{
}

void ConvexVolumeTool::init(NavMeshTool* meshTool)
{
	m_meshTool = meshTool;

	m_state = (ConvexVolumeToolState*)m_meshTool->getToolState(type());
	if (!m_state)
	{
		m_state = new ConvexVolumeToolState();
		m_meshTool->setToolState(type(), m_state);
	}

	m_state->init(m_meshTool);
}

void ConvexVolumeTool::reset()
{
}

void ConvexVolumeTool::handleMenu()
{
	ImGui::SliderFloat("Shape Height", &m_state->m_boxHeight, 0.1f, 100.0f);
	ImGui::SliderFloat("Shape Descent", &m_state->m_boxDescent, -100.f, 100.f);
	ImGui::SliderFloat("Poly Offset", &m_state->m_polyOffset, 0.0f, 10.0f);

	ImGui::Separator();

	ImGui::Text("Area Type");

	ImGui::Indent();

	if (ImGui::RadioButton("Ground", m_state->m_areaType == PolyArea::Ground))
		m_state->m_areaType = PolyArea::Ground;
	if (ImGui::RadioButton("Unwalkable", m_state->m_areaType == PolyArea::Unwalkable))
		m_state->m_areaType = PolyArea::Unwalkable;
#if 0
	if (ImGui::RadioButton("Grass", m_areaType == PolyArea::Grass))
		m_areaType = PolyArea::Grass;
	if (ImGui::RadioButton("Road", m_areaType == PolyArea::Road))
		m_areaType = PolyArea::Road;
	if (ImGui::RadioButton("Water", m_areaType == PolyArea::Water))
		m_areaType = PolyArea::Water;
	if (ImGui::RadioButton("Door", m_areaType == PolyArea::Door))
		m_areaType = PolyArea::Door;
#endif
	ImGui::Unindent();

	ImGui::Separator();

	if (ImGui::Button("Clear Shape"))
	{
		reset();
	}
}

void ConvexVolumeTool::handleClick(const glm::vec3& /*s*/, const glm::vec3& p, bool shift)
{
	if (!m_meshTool) return;
	auto navMesh = m_meshTool->GetNavMesh();

	std::vector<dtTileRef> modifiedTiles = m_state->handleVolumeClick(p, shift);

	if (!modifiedTiles.empty())
	{
		m_meshTool->RebuildTiles(modifiedTiles);
	}
}

void ConvexVolumeTool::handleRender()
{
}

void ConvexVolumeTool::handleRenderOverlay(const glm::mat4& /*proj*/,
	const glm::mat4& /*model*/, const glm::ivec4& view)
{
	// Tool help
	if (m_state->m_pts.empty())
	{
		ImGui::RenderTextRight(-330, -(view[3] - 40), ImVec4(255, 255, 255, 192),
			"LMB: Create new shape.  SHIFT+LMB: Delete existing shape (click inside a shape).");
	}
	else
	{
		ImGui::RenderTextRight(-330, -(view[3] - 40), ImVec4(255, 255, 255, 192),
			"Click LMB to add new points. Alt+Click to finish the shape.");
		ImGui::RenderTextRight(-330, -(view[3] - 60), ImVec4(255, 255, 255, 192),
			"The shape will be convex hull of all added points.");
	}
}

//----------------------------------------------------------------------------

void ConvexVolumeToolState::init(NavMeshTool* meshTool)
{
	m_meshTool = meshTool;
}

void ConvexVolumeToolState::reset()
{
	m_pts.clear();
	m_hull.clear();
}

void ConvexVolumeToolState::handleRender()
{
	DebugDrawGL dd;

	// Find height extents of the shape.
	float minh = FLT_MAX, maxh = 0;
	for (size_t i = 0; i < m_pts.size(); ++i)
		minh = glm::min(minh, m_pts[i].y);
	minh -= m_boxDescent;
	maxh = minh + m_boxHeight;

	dd.begin(DU_DRAW_POINTS, 4.0f);
	for (size_t i = 0; i < m_pts.size(); ++i)
	{
		unsigned int col = duRGBA(255, 255, 255, 255);
		if (i == m_pts.size() - 1)
			col = duRGBA(240, 32, 16, 255);
		dd.vertex(m_pts[i].x, m_pts[i].y + 0.1f, m_pts[i].z, col);
	}
	dd.end();

	dd.begin(DU_DRAW_LINES, 2.0f);
	for (size_t i = 0, j = m_hull.size() - 1; i < m_hull.size(); j = i++)
	{
		const glm::vec3& vi = m_pts[m_hull[j]];
		const glm::vec3& vj = m_pts[m_hull[i]];
		dd.vertex(vj.x, minh, vj.z, duRGBA(255, 255, 255, 64));
		dd.vertex(vi.x, minh, vi.z, duRGBA(255, 255, 255, 64));
		dd.vertex(vj.x, maxh, vj.z, duRGBA(255, 255, 255, 64));
		dd.vertex(vi.x, maxh, vi.z, duRGBA(255, 255, 255, 64));
		dd.vertex(vj.x, minh, vj.z, duRGBA(255, 255, 255, 64));
		dd.vertex(vj.x, maxh, vj.z, duRGBA(255, 255, 255, 64));
	}
	dd.end();
}

void ConvexVolumeToolState::handleRenderOverlay(const glm::mat4& proj,
	const glm::mat4& model, const glm::ivec4& view)
{
}

std::vector<dtTileRef> ConvexVolumeToolState::handleVolumeClick(const glm::vec3& p, bool shift)
{
	auto navMesh = m_meshTool->GetNavMesh();

	std::vector<dtTileRef> modifiedTiles;

	if (shift)
	{
		// Delete
		size_t nearestIndex = -1;
		const auto& volumes = navMesh->GetConvexVolumes();
		size_t i = 0;
		for (const auto& vol : volumes)
		{
			if (pointInPoly(vol->verts, p) && p.y >= vol->hmin && p.y <= vol->hmax)
			{
				nearestIndex = i;
				break;
			}

			i++;
		}

		// If end point close enough, delete it.
		if (nearestIndex != -1)
		{
			const ConvexVolume* volume = navMesh->GetConvexVolume(nearestIndex);
			modifiedTiles = navMesh->GetTilesIntersectingConvexVolume(volume);

			m_meshTool->GetNavMesh()->DeleteConvexVolume(nearestIndex);
		}
	}
	else
	{
		// Create
		bool alt = (SDL_GetModState() & KMOD_ALT) != 0;

		// If clicked on that last pt, create the shape.
		if (m_pts.size() > 0 && (alt || distSqr(p, m_pts[m_pts.size() - 1]) < rcSqr(0.2f)))
		{
			if (m_hull.size() > 2)
			{
				std::vector<glm::vec3> verts(m_hull.size());

				// Create shape.
				for (size_t i = 0; i < m_hull.size(); ++i)
					verts[i] = m_pts[m_hull[i]];

				float minh = FLT_MAX, maxh = 0;
				for (size_t i = 0; i < m_hull.size(); ++i)
					minh = glm::min(minh, verts[i].y);

				minh -= m_boxDescent;
				maxh = minh + m_boxHeight;

				if (m_polyOffset > 0.01f)
				{
					std::vector<glm::vec3> offset(m_hull.size() * 2 + 1);

					int noffset = rcOffsetPoly(glm::value_ptr(verts[0]), static_cast<int>(verts.size()),
						m_polyOffset, glm::value_ptr(offset[0]), static_cast<int>(verts.size() * 2 + 1));
					if (noffset > 0)
					{
						offset.resize(noffset);

						ConvexVolume* volume = navMesh->AddConvexVolume(offset, minh, maxh, m_areaType);
						modifiedTiles = navMesh->GetTilesIntersectingConvexVolume(volume);
					}
				}
				else
				{
					ConvexVolume* volume = navMesh->AddConvexVolume(verts, minh, maxh, m_areaType);
					modifiedTiles = navMesh->GetTilesIntersectingConvexVolume(volume);
				}
			}

			reset();
		}
		else
		{
			// Add new point
			m_pts.push_back(p);

			// Update hull.
			if (m_pts.size() >= 2)
				convexhull(m_pts, m_hull);
			else
				m_hull.clear();
		}
	}

	return modifiedTiles;
}