#pragma once

#include <vector>
#include <DirectXMath.h>
#include "RushHour.h"
#include "D3DSupplementary.h"
#include "assimp\Importer.hpp"
#include "assimp\mesh.h"
#include "assimp\types.h"
#include "assimp\vector3.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"


class Model {
private:
#define INVALID_MATERIAL 0xFFFFFFFF
	struct MeshEntry {
		MeshEntry() {
			_numIndices = 0;
			_baseVertex = 0;
			_baseIndex = 0;
			_materialIndex = INVALID_MATERIAL;
			_pTexture = nullptr;
		}

		unsigned int _numIndices;
		unsigned int _baseVertex;
		unsigned int _baseIndex;
		unsigned int _materialIndex;
		ID3D11ShaderResourceView* _pTexture;    // texture buffer
	};

	/* Common vertex and index buffer for all models */
	static std::vector<VERTEX> _objectVertices;
	static std::vector<UINT> _objectIndices;

	std::vector<MeshEntry> _entries; // metadata for model instance
	Assimp::Importer _imp;
	const aiScene* _pScene = nullptr;
	const aiMesh* _pMesh = nullptr;

public:
	Model(const char* pFile);
	Model(const Model&) = delete;
	Model(Model&&) = default;
	Model& operator= (const Model&) = delete;
	Model& operator= (Model&&) = default;
	~Model();

	const aiScene* GetScene() const { return _pScene; };
	const std::vector<MeshEntry> GetMeshEntries() const { return _entries; }

	static const std::vector<VERTEX> GetModelVertices() { return _objectVertices; }
	static const std::vector<UINT> GetModelIndices() { return _objectIndices; };

};

