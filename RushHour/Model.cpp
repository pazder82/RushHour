#include "stdafx.h"
#include "Model.h"

using namespace std;

std::vector<VERTEX> Model::_objectVertices;
std::vector<UINT> Model::_objectIndices;

Model::Model(const char* pFile) {
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
	_pScene = _imp.ReadFile(pFile, /*aiProcess_CalcTangentSpace |*/ aiProcess_FixInfacingNormals /*	| aiProcess_MakeLeftHanded */ | 
		aiProcess_GenSmoothNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
	if (!_pScene) {
	}else {
		if (_pScene->mNumMeshes < 1) {
		}
		else {
			for (unsigned int m = 0; m < _pScene->mNumMeshes; m++) {
				_pMesh = _pScene->mMeshes[m];
				MeshEntry me;
				me._baseVertex = Model::_objectVertices.size();
				me._baseIndex = _objectIndices.size();
				if (!_pMesh) {
				}
				else {
					Model::_objectVertices.reserve(Model::_objectVertices.size() + _pMesh->mNumVertices);
					for (unsigned int i = 0; i < _pMesh->mNumVertices; i++) {
						const aiVector3D* pPos = &(_pMesh->mVertices[i]);
						const aiVector3D* pNormal = (_pMesh->mNormals != nullptr) ? &(_pMesh->mNormals[i]) : &Zero3D;
						//const aiVector3D* pTexCoord = _pMesh->HasTextureCoords(0) ? &(_pMesh->mTextureCoords[0][i]) : &Zero3D;
						const aiVector3D* pTexCoord = &Zero3D;
						VERTEX v;
						v.pos.x = pPos->x; v.pos.y = pPos->y; v.pos.z = pPos->z;
						v.normal.x = pNormal->x; v.normal.y = pNormal->y; v.normal.z = pNormal->z;
						v.textCoord.x = pTexCoord->x; v.textCoord.y = pTexCoord->y;
						Model::_objectVertices.push_back(v);
					}
					_objectIndices.reserve(_objectIndices.size() + (_pMesh->mNumFaces * 3));
					for (unsigned int i = 0; i < _pMesh->mNumFaces; i++) {
						if (_pMesh->mFaces[i].mNumIndices == 3) {
							_objectIndices.push_back(_pMesh->mFaces[i].mIndices[0]);
							_objectIndices.push_back(_pMesh->mFaces[i].mIndices[1]);
							_objectIndices.push_back(_pMesh->mFaces[i].mIndices[2]);
							me._numIndices += 3;
						}
					}
					_entries.push_back(me);
				}
			}
		}
	}
}


Model::~Model() {
}
