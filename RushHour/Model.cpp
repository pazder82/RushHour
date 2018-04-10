#include "stdafx.h"
#include "Model.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <memory>

using namespace std;
using namespace DirectX;

std::vector<VERTEX> Model::_objectVertices;
std::vector<UINT> Model::_objectIndices;

Model::Model(const char* pFile, ID3D11Device* dev) {
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
	_pScene = _imp.ReadFile(pFile, aiProcess_FlipUVs | aiProcess_FixInfacingNormals | aiProcess_MakeLeftHanded |
		aiProcess_GenSmoothNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
	if (!_pScene) {
	}else {
		if (_pScene->mNumMeshes < 1) {
		}
		else {
			// Process all meshes
			for (unsigned int m = 0; m < _pScene->mNumMeshes; m++) {
				_pMesh = _pScene->mMeshes[m];
				MeshEntry me;
				me._baseVertex = Model::_objectVertices.size();
				me._baseIndex = _objectIndices.size();
				if (_pMesh) {
					// Load vertices
					Model::_objectVertices.reserve(Model::_objectVertices.size() + _pMesh->mNumVertices);
					for (unsigned int i = 0; i < _pMesh->mNumVertices; i++) {
						const aiVector3D* pPos = &(_pMesh->mVertices[i]);
						const aiVector3D* pNormal = (_pMesh->mNormals != nullptr) ? &(_pMesh->mNormals[i]) : &Zero3D;
						const aiVector3D* pTexCoord = (_pMesh->HasTextureCoords(0)) ? &(_pMesh->mTextureCoords[0][i]) : &Zero3D;
						VERTEX v;
						v.pos.x = pPos->x; v.pos.y = pPos->y; v.pos.z = pPos->z;
						v.normal.x = pNormal->x; v.normal.y = pNormal->y; v.normal.z = pNormal->z;
						//v.normal.x = 1.0f; v.normal.y = 0.0f; v.normal.z = 0.0f;
						v.textCoord.x = pTexCoord->x; v.textCoord.y = pTexCoord->y;
						Model::_objectVertices.push_back(v);
					}

					// Load indices
					_objectIndices.reserve(_objectIndices.size() + (_pMesh->mNumFaces * 3));
					for (unsigned int i = 0; i < _pMesh->mNumFaces; i++) {
						if (_pMesh->mFaces[i].mNumIndices == 3) {
							_objectIndices.push_back(_pMesh->mFaces[i].mIndices[0]);
							_objectIndices.push_back(_pMesh->mFaces[i].mIndices[1]);
							_objectIndices.push_back(_pMesh->mFaces[i].mIndices[2]);
							me._numIndices += 3;
						}
					}

					// Process material of the mesh
					if (_pScene->HasMaterials()) {
						aiMaterial* material = _pScene->mMaterials[_pMesh->mMaterialIndex];
						aiString aiTextureFile;
						me._materialIndex = _pMesh->mMaterialIndex;
						// Load texture file
						if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
							material->GetTexture(aiTextureType_DIFFUSE, 0, &aiTextureFile);
							// Convert aiString to LPWSTR
							size_t size = strlen(aiTextureFile.C_Str()) + 1; // plus null
							auto wcTextureFile = std::make_unique<wchar_t[]>(size);
							size_t outSize;
							mbstowcs_s(&outSize, wcTextureFile.get(), size, aiTextureFile.C_Str(), size - 1);
							LPWSTR textureFile = wcTextureFile.get();

							CreateWICTextureFromFile(dev, nullptr, textureFile, nullptr, &(me._pTexture), 0);
							if (me._pTexture == nullptr) {
								CreateDDSTextureFromFile(dev, nullptr, textureFile, nullptr, &(me._pTexture), 0);
							}
						}
/*
						// Load property
						aiMaterialProperty* property;
						char* strp;
						float f;
						int i;
						for (unsigned int p = 0; p < material->mNumProperties; p++) {
							property = material->mProperties[p];
							if (property->mType == aiPTI_String) {
								strp = (char*)(property->mData);
								strp += 4;
							} else if (property->mType == aiPTI_Float) {
								f = *(float*)(property->mData);
							} else if (property->mType == aiPTI_Integer) {
								i = *(int*)(property->mData);
							}
						}
*/
					}
					// If no texture found, use the default one
					if (me._pTexture == nullptr) {
						CreateWICTextureFromFile(dev, nullptr, L"models/default.jpg", nullptr, &(me._pTexture), 0);
					}
					_entries.push_back(me);
				}
			}
		}
	}
}


Model::~Model() {
	for (auto i : _entries) {
		if (i._pTexture) {
			i._pTexture->Release();
		}
	}
}
