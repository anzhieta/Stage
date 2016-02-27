#ifndef SPHERE_H
#define SPHERE_H

#include "stdafx.h"
#include <vector>
#include <glm\glm.hpp>

//Sisältää apufunktioita, joiden avulla voidaan muodostaa pallon piirtämiseen vaadittava 3D-malli
namespace stage{
	/** Pallon verteksit*/
	static std::vector<glm::vec3> sphere_vertices = {
		glm::vec3(0.000000, -1.000000, 0.000000),
		glm::vec3(0.723607, -0.447220, 0.525725),
		glm::vec3(-0.276388, -0.447220, 0.850649),
		glm::vec3(-0.894426, -0.447216, 0.000000),
		glm::vec3(-0.276388, -0.447220, -0.850649),
		glm::vec3(0.723607, -0.447220, -0.525725),
		glm::vec3(0.276388, 0.447220, 0.850649),
		glm::vec3(-0.723607, 0.447220, 0.525725),
		glm::vec3(-0.723607, 0.447220, -0.525725),
		glm::vec3(0.276388, 0.447220, -0.850649),
		glm::vec3(0.894426, 0.447216, 0.000000),
		glm::vec3(0.000000, 1.000000, 0.000000),
		glm::vec3(-0.162456, -0.850654, 0.499995),
		glm::vec3(0.425323, -0.850654, 0.309011),
		glm::vec3(0.262869, -0.525738, 0.809012),
		glm::vec3(0.850648, -0.525736, 0.000000),
		glm::vec3(0.425323, -0.850654, -0.309011),
		glm::vec3(-0.525730, -0.850652, 0.000000),
		glm::vec3(-0.688189, -0.525736, 0.499997),
		glm::vec3(-0.162456, -0.850654, -0.499995),
		glm::vec3(-0.688189, -0.525736, -0.499997),
		glm::vec3(0.262869, -0.525738, -0.809012),
		glm::vec3(0.951058, 0.000000, 0.309013),
		glm::vec3(0.951058, 0.000000, -0.309013),
		glm::vec3(0.000000, 0.000000, 1.000000),
		glm::vec3(0.587786, 0.000000, 0.809017),
		glm::vec3(-0.951058, 0.000000, 0.309013),
		glm::vec3(-0.587786, 0.000000, 0.809017),
		glm::vec3(-0.587786, 0.000000, -0.809017),
		glm::vec3(-0.951058, 0.000000, -0.309013),
		glm::vec3(0.587786, 0.000000, -0.809017),
		glm::vec3(0.000000, 0.000000, -1.000000),
		glm::vec3(0.688189, 0.525736, 0.499997),
		glm::vec3(-0.262869, 0.525738, 0.809012),
		glm::vec3(-0.850648, 0.525736, 0.000000),
		glm::vec3(-0.262869, 0.525738, -0.809012),
		glm::vec3(0.688189, 0.525736, -0.499997),
		glm::vec3(0.162456, 0.850654, 0.499995),
		glm::vec3(0.525730, 0.850652, 0.000000),
		glm::vec3(-0.425323, 0.850654, 0.309011),
		glm::vec3(-0.425323, 0.850654, -0.309011),
		glm::vec3(0.162456, 0.850654, -0.499995)
	};
	/** Pallon tahkot*/
	static std::vector<int> sphere_faces = {
		1, 14, 13,
		2, 14, 16,
		1, 13, 18,
		1, 18, 20,
		1, 20, 17,
		2, 16, 23,
		3, 15, 25,
		4, 19, 27,
		5, 21, 29,
		6, 22, 31,
		2, 23, 26,
		3, 25, 28,
		4, 27, 30,
		5, 29, 32,
		6, 31, 24,
		7, 33, 38,
		8, 34, 40,
		9, 35, 41,
		10, 36, 42,
		11, 37, 39,
		39, 42, 12,
		39, 37, 42,
		37, 10, 42,
		42, 41, 12,
		42, 36, 41,
		36, 9, 41,
		41, 40, 12,
		41, 35, 40,
		35, 8, 40,
		40, 38, 12,
		40, 34, 38,
		34, 7, 38,
		38, 39, 12,
		38, 33, 39,
		33, 11, 39,
		24, 37, 11,
		24, 31, 37,
		31, 10, 37,
		32, 36, 10,
		32, 29, 36,
		29, 9, 36,
		30, 35, 9,
		30, 27, 35,
		27, 8, 35,
		28, 34, 8,
		28, 25, 34,
		25, 7, 34,
		26, 33, 7,
		26, 23, 33,
		23, 11, 33,
		31, 32, 10,
		31, 22, 32,
		22, 5, 32,
		29, 30, 9,
		29, 21, 30,
		21, 4, 30,
		27, 28, 8,
		27, 19, 28,
		19, 3, 28,
		25, 26, 7,
		25, 15, 26,
		15, 2, 26,
		23, 24, 11,
		23, 16, 24,
		16, 6, 24,
		17, 22, 6,
		17, 20, 22,
		20, 5, 22,
		20, 21, 5,
		20, 18, 21,
		18, 4, 21,
		18, 19, 4,
		18, 13, 19,
		13, 3, 19,
		16, 17, 6,
		16, 14, 17,
		14, 1, 17,
		13, 15, 3,
		13, 14, 15,
		14, 2, 15
	};
	/** Yhdistää pallon verteksit ja tahkot yhtenäiseksi verteksilistaksi
	@returns	Lista pallon vektoreista
	*/
	static std::vector<glm::vec3> generate_sphere_vertices(){
		std::vector<glm::vec3> ret;
		for (unsigned int i = 0; i < sphere_faces.size(); i++){
			ret.push_back(sphere_vertices[sphere_faces[i] - 1]);
		}
		return ret;
	}
	/** Arpoo pallon vertekseille värit
	@returns	Lista värivektoreita
	*/
	static std::vector<glm::vec3> generate_sphere_colors(){
		std::vector<glm::vec3> ret;
		for (unsigned int i = 0; i < sphere_faces.size(); i++){
			ret.push_back(glm::vec3(static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
				static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
				static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
		}
		return ret;
	}
}
#endif