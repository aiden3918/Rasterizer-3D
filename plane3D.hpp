#ifndef plane3D_hpp
#define plane3D_hpp

#include "defintions.hpp"
#include "olcPixelGameEngine.h"

// draw actual planes in main b/c OLC_PGE_APPLICATION is defined there, all parts of code pertaining to drawing
// should take place in main

// transform graphs using world matrix
graph transformPlane(graph planeToTransform, matrix4x4 worldMatrix) {
	graph transformedPlane;
	for (auto l : planeToTransform.lines) {
		line lineTransformed;

		lineTransformed.point[0] = Matrix_MultiplyVector(worldMatrix, l.point[0]);
		lineTransformed.point[1] = Matrix_MultiplyVector(worldMatrix, l.point[1]);

		transformedPlane.lines.push_back(lineTransformed);
	}
	return transformedPlane;
}

graph projectPlane(graph planeToProject, matrix4x4 projectionMatrix, float fScreenWidth, float fScreenHeight) {
	graph projectedPlane;
	for (auto l : planeToProject.lines) {
		line projectedLine;

		projectedLine.point[0] = Matrix_MultiplyVector(projectionMatrix, l.point[0]);
		projectedLine.point[1] = Matrix_MultiplyVector(projectionMatrix, l.point[1]);

		projectedLine.point[0] = Vector_Div(projectedLine.point[0], l.point[0].z);
		projectedLine.point[1] = Vector_Div(projectedLine.point[1], l.point[1].z);

		vec3D vOffset = { 1.0f, 1.0f, 0.0f };
		projectedLine.point[0] = Vector_Add(projectedLine.point[0], vOffset);
		projectedLine.point[1] = Vector_Add(projectedLine.point[1], vOffset);

		projectedLine.point[0].x *= 0.5 * fScreenWidth; projectedLine.point[0].y *= 0.5 * fScreenHeight;
		projectedLine.point[1].x *= 0.5 * fScreenWidth; projectedLine.point[1].y *= 0.5 * fScreenHeight;


		projectedPlane.lines.push_back(projectedLine);
	}
	
	return projectedPlane;
}

graph postProjTransPlane(graph projPlane, vec3D translation) {
	graph postTransPlane;
	for (auto i : projPlane.lines) {
		postTransPlane.lines.push_back({ Vector_Add(i.point[0], translation),	Vector_Add(i.point[1], translation) });
	}
	return postTransPlane;
}

function transformPoints(function fnToTransform, matrix4x4 worldMatrix) {
	function transformedFn;
	for (auto vec : fnToTransform.points) {
		vec3D transformedPt;
		transformedPt = Matrix_MultiplyVector(worldMatrix, vec);

		transformedFn.points.push_back(transformedPt);
	}
	return transformedFn;
}

function projectPoints(function fnToProject, matrix4x4 projectionMatrix, float fScreenWidth, float fScreenHeight) {
	function projectedFn;
	for (auto vec : fnToProject.points) {
		vec3D projectedPt;
		projectedPt = Matrix_MultiplyVector(projectionMatrix, vec);

		projectedPt = Vector_Div(projectedPt, projectedPt.z);
		
		vec3D vOffset = { 1.0f, 1.0f, 0.0f };
		projectedPt = Vector_Add(projectedPt, vOffset);

		projectedPt.x *= 0.5 * fScreenWidth;
		projectedPt.y *= 0.5 * fScreenHeight;

		projectedFn.points.push_back(projectedPt);
	}
	return projectedFn;
}

function postProjTransFunction(function projFn, vec3D translation) {
	function postTransFn;
	for (auto i : projFn.points) {
		postTransFn.points.push_back(Vector_Add(i, translation));
	}
	return postTransFn;
}

#endif