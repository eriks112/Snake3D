#include "Model.h"

std::vector<Texture> Model::loadedTextures;

bool Model::Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->cb_vs_vertexshader = &cb_vs_vertexshader;

	try
	{
		if (!LoadModel(filePath))
			return false;
	}
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}

	return true;
}

void Model::Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
	             ID3D11ShaderResourceView* shaderResource, ID3D11ShaderResourceView* shaderResource2)
{
	// Create matrixes for all the meshes in the model then draws it
	for (int i = 0; i < meshes.size(); i++)
	{
		//Update Constant buffer with WVP Matrix
		cb_vs_vertexshader->data.viewMatrix = viewMatrix; //Calculate World-View-Projection Matrix
		cb_vs_vertexshader->data.projectionMatrix = projectionMatrix; //Calculate World-View-Projection Matrix
		cb_vs_vertexshader->data.worldMatrix = meshes[i].GetTransformMatrix() * worldMatrix; //Calculate World
		cb_vs_vertexshader->ApplyChanges();

		meshes[i].Draw(shaderResource, shaderResource2, cb_vs_vertexshader);
	}
}

bool Model::LoadModel(const std::string& filePath)
{
	directory = StringHelper::GetDirectoryFromPath(filePath);

	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filePath,
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (pScene == nullptr)
		return false;

	ProcessNode(pScene->mRootNode, pScene, DirectX::XMMatrixIdentity());
	return true;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& parentTransformMatrix)
{
	XMMATRIX nodeTransformMatrix = XMMatrixTranspose(XMMATRIX(&node->mTransformation.a1)) * parentTransformMatrix;

	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene, nodeTransformMatrix));
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, nodeTransformMatrix);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix)
{
	// Data to fill
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;
	DirectX::XMFLOAT3 tangent, binormal, normal;
	
	//Get vertices
	for (UINT i = 0; i < mesh->mNumVertices; i+=3)
	{
		Vertex vertex1, vertex2, vertex3;
		XMFLOAT3 tangent, binormal;

		vertex1.pos.x = mesh->mVertices[i].x;
		vertex1.pos.y = mesh->mVertices[i].y;
		vertex1.pos.z = mesh->mVertices[i].z;
		vertex1.normal.x = mesh->mNormals[i].x;
		vertex1.normal.y = mesh->mNormals[i].y;
		vertex1.normal.z = mesh->mNormals[i].z;
		if (mesh->mTextureCoords[0])
		{
			vertex1.texCoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex1.texCoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertex2.pos.x = mesh->mVertices[i+1].x;
		vertex2.pos.y = mesh->mVertices[i+1].y;
		vertex2.pos.z = mesh->mVertices[i+1].z;
		vertex2.normal.x = mesh->mNormals[i+1].x;
		vertex2.normal.y = mesh->mNormals[i+1].y;
		vertex2.normal.z = mesh->mNormals[i+1].z;
		if (mesh->mTextureCoords[0])
		{
			vertex2.texCoord.x = (float)mesh->mTextureCoords[0][i+1].x;
			vertex2.texCoord.y = (float)mesh->mTextureCoords[0][i+1].y;
		}

		vertex3.pos.x = mesh->mVertices[i+2].x;
		vertex3.pos.y = mesh->mVertices[i+2].y;
		vertex3.pos.z = mesh->mVertices[i+2].z;
		vertex3.normal.x = mesh->mNormals[i+2].x;
		vertex3.normal.y = mesh->mNormals[i+2].y;
		vertex3.normal.z = mesh->mNormals[i+2].z;
		if (mesh->mTextureCoords[0])
		{
			vertex3.texCoord.x = (float)mesh->mTextureCoords[0][i+2].x;
			vertex3.texCoord.y = (float)mesh->mTextureCoords[0][i+2].y;
		}

		// Calculate the tangent and binormal of that face.
		CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

		// Calculate the new normal using the tangent and binormal.
		CalculateNormal(tangent, binormal, normal);

		vertex1.normal.x = normal.x;
		vertex1.normal.y = normal.y;
		vertex1.normal.z = normal.z;
		vertex1.tangent.x = tangent.x;
		vertex1.tangent.y = tangent.y;
		vertex1.tangent.z = tangent.z;
		vertex1.biNormal.x = binormal.x;
		vertex1.biNormal.y = binormal.y;
		vertex1.biNormal.z = binormal.z;

		vertex2.normal.x = normal.x;
		vertex2.normal.y = normal.y;
		vertex2.normal.z = normal.z;
		vertex2.tangent.x = tangent.x;
		vertex2.tangent.y = tangent.y;
		vertex2.tangent.z = tangent.z;
		vertex2.biNormal.x = binormal.x;
		vertex2.biNormal.y = binormal.y;
		vertex2.biNormal.z = binormal.z;

		vertex3.normal.x = normal.x;
		vertex3.normal.y = normal.y;
		vertex3.normal.z = normal.z;
		vertex3.tangent.x = tangent.x;
		vertex3.tangent.y = tangent.y;
		vertex3.tangent.z = tangent.z;
		vertex3.biNormal.x = binormal.x;
		vertex3.biNormal.y = binormal.y;
		vertex3.biNormal.z = binormal.z;

		vertices.push_back(vertex1);
		vertices.push_back(vertex2);
		vertices.push_back(vertex3);
	}

	//Get indices
	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// Load textures for model
	std::vector<Texture> textures;
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	std::vector<Texture> diffuseTextures = LoadMaterialTextures(material, aiTextureType::aiTextureType_DIFFUSE, scene);
	textures.insert(textures.end(), diffuseTextures.begin(), diffuseTextures.end());
	std::vector<Texture> normalTexture = LoadMaterialTextures(material, aiTextureType::aiTextureType_NORMALS, scene);
	textures.insert(textures.end(), normalTexture.begin(), normalTexture.end());
	std::vector<Texture> specularTexture = LoadMaterialTextures(material, aiTextureType::aiTextureType_SHININESS, scene);
	textures.insert(textures.end(), specularTexture.begin(), specularTexture.end());

	return Mesh(device, deviceContext, vertices, indices, textures, transformMatrix);
}

void Model::CalculateTangentBinormal(Vertex vertex1, Vertex vertex2, Vertex vertex3,
	XMFLOAT3& tangent, XMFLOAT3& binormal)
{
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;
	float length;


	// Calculate the two vectors for this face.
	vector1[0] = vertex2.pos.x - vertex1.pos.x;
	vector1[1] = vertex2.pos.y - vertex1.pos.y;
	vector1[2] = vertex2.pos.z - vertex1.pos.z;

	vector2[0] = vertex3.pos.x - vertex1.pos.x;
	vector2[1] = vertex3.pos.y - vertex1.pos.y;
	vector2[2] = vertex3.pos.z - vertex1.pos.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertex2.texCoord.x - vertex1.texCoord.x;
	tvVector[0] = vertex2.texCoord.y - vertex1.texCoord.y;

	tuVector[1] = vertex3.texCoord.x - vertex1.texCoord.x;
	tvVector[1] = vertex3.texCoord.y - vertex1.texCoord.y;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of this normal.
	length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize the normal and then store it
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of this normal.
	length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

	// Normalize the normal and then store it
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;

	return;
}

void Model::CalculateNormal(XMFLOAT3 tangent, XMFLOAT3 binormal, XMFLOAT3& normal)
{
	float length;

	// Calculate the cross product of the tangent and binormal which will give the normal vector.
	normal.x = (tangent.y * binormal.z) - (tangent.z * binormal.y);
	normal.y = (tangent.z * binormal.x) - (tangent.x * binormal.z);
	normal.z = (tangent.x * binormal.y) - (tangent.y * binormal.x);

	// Calculate the length of the normal.
	length = sqrt((normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z));

	// Normalize the normal.
	normal.x = normal.x / length;
	normal.y = normal.y / length;
	normal.z = normal.z / length;

	return;
}

TextureStorageType Model::DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType)
{
	if (pMat->GetTextureCount(textureType) == 0)
		return TextureStorageType::None;

	aiString path;
	pMat->GetTexture(textureType, index, &path);
	std::string texturePath = path.C_Str();
	//Check if texture is an embedded indexed texture by seeing if the file path is an index #
	if (texturePath[0] == '*')
	{
		if (pScene->mTextures[0]->mHeight == 0)
		{
			return TextureStorageType::EmbeddedIndexCompressed;
		}
		else
		{
			assert("SUPPORT DOES NOT EXIST YET FOR INDEXED NON COMPRESSED TEXTURES!" && 0);
			return TextureStorageType::EmbeddedIndexNonCompressed;
		}
	}
	//Check if texture is an embedded texture but not indexed (path will be the texture's name instead of #)
	if (auto pTex = pScene->GetEmbeddedTexture(texturePath.c_str()))
	{
		if (pTex->mHeight == 0)
		{
			return TextureStorageType::EmbeddedCompressed;
		}
		else
		{
			assert("SUPPORT DOES NOT EXIST YET FOR EMBEDDED NON COMPRESSED TEXTURES!" && 0);
			return TextureStorageType::EmbeddedNonCompressed;
		}
	}
	//Lastly check if texture is a filepath by checking for period before extension name
	if (texturePath.find('.') != std::string::npos)
	{
		return TextureStorageType::Disk;
	}

	return TextureStorageType::None; // No texture exists
}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene)
{
	std::vector<Texture> materialTextures;
	TextureStorageType storetype = TextureStorageType::Invalid;
	unsigned int textureCount = pMaterial->GetTextureCount(textureType);

	if (textureCount == 0) //If there are no textures
	{
		storetype = TextureStorageType::None;
		aiColor3D aiColor(0.0f, 0.0f, 0.0f);

		switch (textureType)
		{
		case aiTextureType_DIFFUSE:
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
			if (aiColor.IsBlack()) //If color = black, just use grey
			{
				materialTextures.push_back(Texture(device, Colors::UnloadedTextureColor, textureType));
				return materialTextures;
			}
			materialTextures.push_back(Texture(device, Color(aiColor.r * 255, aiColor.g * 255, aiColor.b * 255), textureType));
			return materialTextures;
		}
	}
	else
	{
		for (UINT i = 0; i < textureCount; i++)
		{		
			// Check if texture already is loaded into memory
			bool textureExists = false;
			for (size_t i = 0; i < loadedTextures.size(); i++)
			{
				if (pMaterial->GetName() == loadedTextures.at(i).GetName() && textureType == loadedTextures.at(i).GetType()) // Texture already exists
				{
					materialTextures.push_back(loadedTextures.at(i));
					textureExists = true;
					break;
				}
			}

			// If current texture was found no need to process further
			if (textureExists)
				break;

			aiString path;
			pMaterial->GetTexture(textureType, i, &path);
			TextureStorageType storetype = DetermineTextureStorageType(pScene, pMaterial, i, textureType);
			switch (storetype)
			{
			case TextureStorageType::EmbeddedIndexCompressed:
			{
				int index = GetTextureIndex(&path);
				Texture embeddedIndexedTexture(device,
					reinterpret_cast<uint8_t*>(pScene->mTextures[index]->pcData),
					pScene->mTextures[index]->mWidth,
					textureType);
				embeddedIndexedTexture.SetName(pMaterial->GetName());
				materialTextures.push_back(embeddedIndexedTexture);
				loadedTextures.push_back(embeddedIndexedTexture); // Stores the texture in the static collection
				break;
			}
			case TextureStorageType::EmbeddedCompressed: // This is the texture in FBX files from blender
			{
				// Adds the textures for the model
				const aiTexture* pTexture = pScene->GetEmbeddedTexture(path.C_Str());
				Texture embeddedTexture(device,
					reinterpret_cast<uint8_t*>(pTexture->pcData),
					pTexture->mWidth,
					textureType);
				embeddedTexture.SetName(pMaterial->GetName());

				materialTextures.push_back(embeddedTexture);
				loadedTextures.push_back(embeddedTexture); // Stores the texture in the static collection

				break;
			}
			case TextureStorageType::Disk:
			{
				std::string filename = directory + '\\' + path.C_Str();
				Texture diskTexture(device, filename, textureType);
				diskTexture.SetName(pMaterial->GetName());
				materialTextures.push_back(diskTexture);
				loadedTextures.push_back(diskTexture); // Stores the texture in the static collection
				break;
			}
				break;
			}
		}
	}

	if (materialTextures.size() == 0)
	{
		materialTextures.push_back(Texture(device, Colors::UnhandledTextureColor, aiTextureType::aiTextureType_DIFFUSE));
	}

	return materialTextures;
}

int Model::GetTextureIndex(aiString* pStr)
{
	assert(pStr->length >= 2);
	return atoi(&pStr->C_Str()[1]);
}