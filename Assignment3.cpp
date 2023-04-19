/** @file Week7-2-TreeBillboardsApp.cpp
 *  @brief Tree Billboarding Demo
 *   Adding Billboarding to our previous Hills, Mountain, Crate, and Wave Demo
 * 
 *   Controls:
 *   Hold the left mouse button down and move the mouse to rotate.
 *   Hold the right mouse button down and move the mouse to zoom in and out.
 *
 *  @author Hooman Salamat
 */

#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
#include "FrameResource.h"
#include "Waves.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

const int gNumFrameResources = 3;


struct RenderItem
{
	RenderItem() = default;

    // World matrix of the shape that describes the object's local space
    // relative to the world space, which defines the position, orientation,
    // and scale of the object in the world.
    XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;

	// Collision Bounds
	DirectX::BoundingBox Bounds;
};


enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	AlphaTestedTreeSprites,
	Count
};

class Assignment3 : public D3DApp
{
public:
    Assignment3(HINSTANCE hInstance);
    Assignment3(const Assignment3& rhs) = delete;
    Assignment3& operator=(const Assignment3& rhs) = delete;
    ~Assignment3();

    virtual bool Initialize()override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdateWaves(const GameTimer& gt); 

	void LoadTextures();
    void BuildRootSignature();
	void BuildDescriptorHeaps();
    void BuildShadersAndInputLayouts();
    void BuildLandGeometry();
    void BuildWavesGeometry();
	void BuildBoxGeometry();

	void BuildGateWalkWayGeometry();
	void BuildBackWallWalkGeometry();
	void BuildRightWallWalkGeometry();
	void BuildLeftWallWalkGeometry();

	void BuildGateLedgeGeometry();
	void BuildBackLedgeGeometry();
	void BuildRightLedgeGeometry();
	void BuildLeftLedgeGeometry();

	void BuildAvenueGeometry();

	void BuildDrawBridgeGeometry();

	void BuildCylinderGeometry();
	void BuildConeGeometry();
	void BuildTorusGeometry();
	void BuildWedgeGeometry();
	void BuildPyramidGeometry();
	void BuildDiamondGeometry();
	void BuildTriPrismGeometry();
	void BuildSphereGeometry();
	void BuildTreeSpritesGeometry();
    void BuildPSOs();
    void BuildFrameResources();
    void BuildMaterials();
    void BuildRenderItems();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	bool CheckCollision(const XMFLOAT3& playerPos, float boxSize);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

    float GetHillsHeight(float x, float z)const;
    XMFLOAT3 GetHillsNormal(float x, float z)const;
private:

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    UINT mCbvSrvDescriptorSize = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mStdInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mTreeSpriteInputLayout;

    RenderItem* mWavesRitem = nullptr;

	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;

	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

	std::unique_ptr<Waves> mWaves;

    PassConstants mMainPassCB;

	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 mTarget = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();
	XMFLOAT3 mPlayerPos;
	XMFLOAT4 mPlayerOrientation;
	BoundingSphere mCameraBounds;

    float mTheta = 1.5f*XM_PI;
    float mPhi = XM_PIDIV2 - 0.1f;
    float mRadius = 70.0f;

    POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        Assignment3 theApp(hInstance);
        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

Assignment3::Assignment3(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
}

Assignment3::~Assignment3()
{
    if(md3dDevice != nullptr)
        FlushCommandQueue();
}

bool Assignment3::Initialize()
{
    if(!D3DApp::Initialize())
        return false;

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    // Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
    mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    mWaves = std::make_unique<Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);
	mPlayerPos = XMFLOAT3(60.0f, 0.0f, -220.0f);
	mPlayerOrientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

 
	LoadTextures();
    BuildRootSignature();
	BuildDescriptorHeaps();
    BuildShadersAndInputLayouts();
    BuildLandGeometry();
    BuildWavesGeometry();
	BuildBoxGeometry();
	BuildGateWalkWayGeometry();
	BuildBackWallWalkGeometry();
	BuildRightWallWalkGeometry();
	BuildLeftWallWalkGeometry();
	BuildGateLedgeGeometry();
	BuildBackLedgeGeometry();
	BuildRightLedgeGeometry();
	BuildLeftLedgeGeometry();
	BuildAvenueGeometry();
	BuildDrawBridgeGeometry();
	BuildCylinderGeometry();
	BuildConeGeometry();
	BuildTorusGeometry();
	BuildWedgeGeometry();
	BuildPyramidGeometry();
	BuildDiamondGeometry();
	BuildTriPrismGeometry();
	BuildSphereGeometry();
	BuildTreeSpritesGeometry();
	BuildMaterials();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSOs();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

    return true;
}
 
void Assignment3::OnResize()
{
    D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection matrix.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void Assignment3::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);
	UpdateCamera(gt);

    // Cycle through the circular frame resource array.
    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
    mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if(mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

	AnimateMaterials(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
    UpdateWaves(gt);


}
void Assignment3::Draw(const GameTimer& gt)
{
    auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    ThrowIfFailed(cmdListAlloc->Reset());

    // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::CornflowerBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

    DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(mPSOs["alphaTested"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AlphaTested]);

	mCommandList->SetPipelineState(mPSOs["treeSprites"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AlphaTestedTreeSprites]);

	mCommandList->SetPipelineState(mPSOs["transparent"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Transparent]);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
    ThrowIfFailed(mCommandList->Close());

    // Add the command list to the queue for execution.
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Swap the back and front buffers
    ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    // Advance the fence value to mark commands up to this fence point.
    mCurrFrameResource->Fence = ++mCurrentFence;

    // Add an instruction to the command queue to set a new fence point. 
    // Because we are on the GPU timeline, the new fence point won't be 
    // set until the GPU finishes processing all the commands prior to this Signal().
    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void Assignment3::OnMouseDown(WPARAM btnState, int x, int y)
{
}
void Assignment3::OnMouseUp(WPARAM btnState, int x, int y)
{
}
void Assignment3::OnMouseMove(WPARAM btnState, int x, int y)
{

}
void Assignment3::OnKeyboardInput(const GameTimer& gt)
{
	float dt = (gt.DeltaTime()) * 10;

	// Set up the movement and rotation speeds.
	float dPosition = 5.0f;
	float dTheta = 0.3f;
	float dPhi = 0.3f;

	// Calculate the camera's forward and right vectors based on mTheta and mPhi.
	XMVECTOR forward = XMVectorSet(
		sinf(mPhi) * sinf(mTheta),
		cosf(mPhi),
		sinf(mPhi) * cosf(mTheta),
		0.0f);
	forward = XMVector3Normalize(forward);

	XMVECTOR right = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), forward));

	// Move the player based on the keys pressed.
	if (GetAsyncKeyState('W') & 0x8000)
	{
		XMVECTOR playerPos = XMLoadFloat3(&mPlayerPos);
		XMVECTOR newPos = playerPos + (dPosition * dt * forward);
		newPos = XMVectorSetY(newPos, playerPos.m128_f32[1]);
		XMStoreFloat3(&mPlayerPos, newPos);
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		XMVECTOR playerPos = XMLoadFloat3(&mPlayerPos);
		XMVECTOR newPos = playerPos - (dPosition * dt * forward);
		newPos = XMVectorSetY(newPos, playerPos.m128_f32[1]);
		XMStoreFloat3(&mPlayerPos, newPos);
	}
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		XMVECTOR playerPos = XMLoadFloat3(&mPlayerPos);
		XMVECTOR newPos = playerPos - (dPosition * dt * right);
		newPos = XMVectorSetY(newPos, playerPos.m128_f32[1]);
		XMStoreFloat3(&mPlayerPos, newPos);
	}
	if (GetAsyncKeyState('E') & 0x8000)
	{
		XMVECTOR playerPos = XMLoadFloat3(&mPlayerPos);
		XMVECTOR newPos = playerPos + (dPosition * dt * right);
		newPos = XMVectorSetY(newPos, playerPos.m128_f32[1]);
		XMStoreFloat3(&mPlayerPos, newPos);
	}

	// Adjust the camera angles based on the keys pressed.
	if (GetAsyncKeyState('A') & 0x8000)
	{
		mTheta -= dTheta * dt;
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		mTheta += dTheta * dt;
	}

	// Update the camera position and target.
	UpdateCamera(gt);
}
void Assignment3::UpdateCamera(const GameTimer& gt)
{
	// Calculate the camera's position and orientation based on the player's position and orientation.
	XMVECTOR playerPos = XMLoadFloat3(&mPlayerPos);
	XMVECTOR playerOrientation = XMLoadFloat4(&mPlayerOrientation);
	XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(playerOrientation);
	XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotationMatrix);
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), forward));
	XMVECTOR up = XMVector3Cross(forward, right);
	XMVECTOR eyePos = playerPos + XMVectorSet(0.0f, 5.0f, 0.0f, 0.0f);

	// Calculate the camera's theta value.
	float theta = mTheta;

	XMMATRIX view = XMMatrixLookToLH(eyePos, XMVectorSet(
		sinf(mPhi) * sinf(theta),
		cosf(mPhi),
		sinf(mPhi) * cosf(theta),
		0.0f), up);

	XMStoreFloat4x4(&mView, view);
	mCameraBounds.Center = mPlayerPos;
}

void Assignment3::AnimateMaterials(const GameTimer& gt)
{
	// Scroll the water material texture coordinates.
	auto waterMat = mMaterials["water"].get();

	float& tu = waterMat->MatTransform(3, 0);
	float& tv = waterMat->MatTransform(3, 1);

	tu += 0.1f * gt.DeltaTime();
	tv += 0.02f * gt.DeltaTime();

	if(tu >= 1.0f)
		tu -= 1.0f;

	if(tv >= 1.0f)
		tv -= 1.0f;

	waterMat->MatTransform(3, 0) = tu;
	waterMat->MatTransform(3, 1) = tv;

	// Material has changed, so need to update cbuffer.
	waterMat->NumFramesDirty = gNumFrameResources;
}
void Assignment3::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for(auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if(e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}
void Assignment3::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
	for(auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if(mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}
// Adding Lighting to scene 
void Assignment3::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mEyePos;
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.05f, 0.05f, 0.15f, 0.5f };
	float time = gt.TotalTime();
	float rotationSpeed = 0.3f; // Set the rotation speed to 0.5 radians per second
	float radius = 10.0f; // Set the radius to 10 units

	float x = radius * cosf(rotationSpeed * time);
	float y = 0.0f; // Keep the light on the xz-plane
	float z = radius * sinf(rotationSpeed * time);

	// Set the sun's light direction and color
	mMainPassCB.Lights[0].Direction = { x,y,z };
	mMainPassCB.Lights[0].Strength = { 0.1f, 0.095f, 0.07f }; // Adjust the strength of the sun's light separately

	// Set the moon's light direction and color
	float moonRotationSpeed = 0.2f;
	float moonRadius = 15.0f;
	float mx = moonRadius * cosf(moonRotationSpeed * time);
	float my = 0.0f;
	float mz = moonRadius * sinf(moonRotationSpeed * time);
	mMainPassCB.Lights[1].Direction = { mx, my + XM_PI, mz };
	mMainPassCB.Lights[1].Strength = { 0.02f, 0.035f, 0.055f };

	// Set the second moon's light direction and color
	float moonRotationSpeed2 = 0.2f;
	float moonRadius2 = 5.0f;
	float mx2 = moonRadius2 * cosf(moonRotationSpeed2 * time);
	float my2 = 0.0f;
	float mz2 = moonRadius2 * sinf(moonRotationSpeed2 * time);
	float moonFinalX = mx + mx2;
	float moonFinalY = my2;
	float moonFinalZ = mz + mz2;
	mMainPassCB.Lights[2].Direction = { moonFinalX, moonFinalY, moonFinalZ };
	mMainPassCB.Lights[2].Strength = { 0.02f, 0.02f, 0.05f };

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void Assignment3::UpdateWaves(const GameTimer& gt)
{
	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if((mTimer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = MathHelper::Rand(4, mWaves->RowCount() - 5);
		int j = MathHelper::Rand(4, mWaves->ColumnCount() - 5);

		float r = MathHelper::RandF(0.2f, 0.5f);

		mWaves->Disturb(i, j, r);
	}

	// Update the wave simulation.
	mWaves->Update(gt.DeltaTime());

	// Update the wave vertex buffer with the new solution.
	auto currWavesVB = mCurrFrameResource->WavesVB.get();
	for(int i = 0; i < mWaves->VertexCount(); ++i)
	{
		Vertex v;

		v.Pos = mWaves->Position(i);
		v.Normal = mWaves->Normal(i);
		
		// Derive tex-coords from position by 
		// mapping [-w/2,w/2] --> [0,1]
		v.TexC.x = 0.5f + v.Pos.x / mWaves->Width();
		v.TexC.y = 0.5f - v.Pos.z / mWaves->Depth();

		currWavesVB->CopyData(i, v);
	}

	// Set the dynamic VB of the wave renderitem to the current frame VB.
	mWavesRitem->Geo->VertexBufferGPU = currWavesVB->Resource();
}

// Textures Step1
void Assignment3::LoadTextures()
{
	auto grassTex = std::make_unique<Texture>();
	grassTex->Name = "grassTex";
	grassTex->Filename = L"../../Textures/grass.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), grassTex->Filename.c_str(),
		grassTex->Resource, grassTex->UploadHeap));

	auto waterTex = std::make_unique<Texture>();
	waterTex->Name = "waterTex";
	waterTex->Filename = L"../../Textures/water1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), waterTex->Filename.c_str(),
		waterTex->Resource, waterTex->UploadHeap));

	auto fenceTex = std::make_unique<Texture>();
	fenceTex->Name = "fenceTex";
	fenceTex->Filename = L"../../Textures/WireFence.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), fenceTex->Filename.c_str(),
		fenceTex->Resource, fenceTex->UploadHeap));

	auto crateTex = std::make_unique<Texture>();
	crateTex->Name = "crateTex";
	crateTex->Filename = L"../../Textures/WoodCrate01.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), crateTex->Filename.c_str(),
		crateTex->Resource, crateTex->UploadHeap));

	auto wallTex = std::make_unique<Texture>();
	wallTex->Name = "wallTex";
	wallTex->Filename = L"../../Textures/bricks.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), wallTex->Filename.c_str(),
		wallTex->Resource, wallTex->UploadHeap));

	auto wall2Tex = std::make_unique<Texture>();
	wall2Tex->Name = "wall2Tex";
	wall2Tex->Filename = L"../../Textures/bricks2.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), wall2Tex->Filename.c_str(),
		wall2Tex->Resource, wall2Tex->UploadHeap));

	auto roof1Tex = std::make_unique<Texture>();
	roof1Tex->Name = "roof1Tex";
	roof1Tex->Filename = L"../../Textures/roof1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), roof1Tex->Filename.c_str(),
		roof1Tex->Resource, roof1Tex->UploadHeap));

	auto wood1Tex = std::make_unique<Texture>();
	wood1Tex->Name = "wood1Tex";
	wood1Tex->Filename = L"../../Textures/wood2.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), wood1Tex->Filename.c_str(),
		wood1Tex->Resource, wood1Tex->UploadHeap));

	auto stone1Tex = std::make_unique<Texture>();
	stone1Tex->Name = "stone1Tex";
	stone1Tex->Filename = L"../../Textures/stone.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), stone1Tex->Filename.c_str(),
		stone1Tex->Resource, stone1Tex->UploadHeap));

	auto wood2Tex = std::make_unique<Texture>();
	wood2Tex->Name = "wood2Tex";
	wood2Tex->Filename = L"../../Textures/wood3.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), wood2Tex->Filename.c_str(),
		wood2Tex->Resource, wood2Tex->UploadHeap));

	auto rMetalTex = std::make_unique<Texture>();
	rMetalTex->Name = "rMetalTex";
	rMetalTex->Filename = L"../../Textures/rustyMetal.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), rMetalTex->Filename.c_str(),
		rMetalTex->Resource,rMetalTex->UploadHeap));

	auto dirtGrassTex = std::make_unique<Texture>();
	dirtGrassTex->Name = "dirtGrassTex";
	dirtGrassTex->Filename = L"../../Textures/dirtGrass.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), dirtGrassTex->Filename.c_str(),
		dirtGrassTex->Resource, dirtGrassTex->UploadHeap));

	auto hedgeTex = std::make_unique<Texture>();
	hedgeTex->Name = "hedgeTex";
	hedgeTex->Filename = L"../../Textures/hedge1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), hedgeTex->Filename.c_str(),
		hedgeTex->Resource, hedgeTex->UploadHeap));
	
	auto treeArrayTex = std::make_unique<Texture>();
	treeArrayTex->Name = "treeArrayTex";
	treeArrayTex->Filename = L"../../Textures/treeArray.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), treeArrayTex->Filename.c_str(),
		treeArrayTex->Resource, treeArrayTex->UploadHeap));

	mTextures[grassTex->Name] = std::move(grassTex);
	mTextures[waterTex->Name] = std::move(waterTex);
	mTextures[fenceTex->Name] = std::move(fenceTex);
	mTextures[crateTex->Name] = std::move(crateTex);
	mTextures[wallTex->Name] = std::move(wallTex);
	mTextures[roof1Tex->Name] = std::move(roof1Tex);
	mTextures[wood1Tex->Name] = std::move(wood1Tex);
	mTextures[stone1Tex->Name] = std::move(stone1Tex);
	mTextures[wood2Tex->Name] = std::move(wood2Tex);
	mTextures[rMetalTex->Name] = std::move(rMetalTex);
	mTextures[dirtGrassTex->Name] = std::move(dirtGrassTex);
	mTextures[hedgeTex->Name] = std::move(hedgeTex);
	mTextures[treeArrayTex->Name] = std::move(treeArrayTex);
}
void Assignment3::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[1].InitAsConstantBufferView(0);
    slotRootParameter[2].InitAsConstantBufferView(1);
    slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

    // A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if(errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

// Textures Step2
void Assignment3::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 14;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto grassTex = mTextures["grassTex"]->Resource;
	auto waterTex = mTextures["waterTex"]->Resource;
	auto fenceTex = mTextures["fenceTex"]->Resource;
	auto crateTex = mTextures["crateTex"]->Resource;
	auto wallTex = mTextures["wallTex"]->Resource;
	auto roof1Tex = mTextures["roof1Tex"]->Resource;
	auto wood1Tex = mTextures["wood1Tex"]->Resource;
	auto stone1Tex = mTextures["stone1Tex"]->Resource;
	auto wood2Tex = mTextures["wood2Tex"]->Resource;
	auto rMetalTex = mTextures["rMetalTex"]->Resource;
	auto dirtGrassTex = mTextures["dirtGrassTex"]->Resource;
	auto hedgeTex = mTextures["hedgeTex"]->Resource;
	auto treeArrayTex = mTextures["treeArrayTex"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	md3dDevice->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	// Adding Water Texture
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = waterTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(waterTex.Get(), &srvDesc, hDescriptor);

	// Adding Fence Texture
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = fenceTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(fenceTex.Get(), &srvDesc, hDescriptor);

	// Adding Crate Texture
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = crateTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(crateTex.Get(), &srvDesc, hDescriptor);

	// Adding Wall Texture
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = wallTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(wallTex.Get(), &srvDesc, hDescriptor);

	// Adding Roof1 Texture
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = roof1Tex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(roof1Tex.Get(), &srvDesc, hDescriptor);

	// Adding wood1 Texture
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = wood1Tex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(wood1Tex.Get(), &srvDesc, hDescriptor);

	// Adding Stone1 Texture
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = stone1Tex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(stone1Tex.Get(), &srvDesc, hDescriptor);

	// Adding wood2 Texture
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = wood2Tex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(wood2Tex.Get(), &srvDesc, hDescriptor);

	// Adding Rusty Metal Texture
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = rMetalTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(rMetalTex.Get(), &srvDesc, hDescriptor);	
	
	// Adding dirt Grass Texture
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = dirtGrassTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(dirtGrassTex.Get(), &srvDesc, hDescriptor);

	// Adding Hedge Texture
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = hedgeTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(hedgeTex.Get(), &srvDesc, hDescriptor);

	// Adding Tree Arrey Textures
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	auto desc = treeArrayTex->GetDesc();
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Format = treeArrayTex->GetDesc().Format;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = treeArrayTex->GetDesc().DepthOrArraySize;
	md3dDevice->CreateShaderResourceView(treeArrayTex.Get(), &srvDesc, hDescriptor);
}
void Assignment3::BuildShadersAndInputLayouts()
{
	const D3D_SHADER_MACRO defines[] =
	{
		//"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		//"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", defines, "PS", "ps_5_1");
	mShaders["alphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_1");
	
	mShaders["treeSpriteVS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["treeSpriteGS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "GS", "gs_5_1");
	mShaders["treeSpritePS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", alphaTestDefines, "PS", "ps_5_1");

    mStdInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

	mTreeSpriteInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

// Build Geometry
// Build Grid
void Assignment3::BuildLandGeometry()
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 320.0f, 10, 20);

    std::vector<Vertex> vertices(grid.Vertices.size());
    for(size_t i = 0; i < grid.Vertices.size(); ++i)
    {
        auto& p = grid.Vertices[i].Position;
        vertices[i].Pos = p;
        vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
        vertices[i].Normal = GetHillsNormal(p.x, p.z);
		vertices[i].TexC = grid.Vertices[i].TexC;
    }

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

    std::vector<std::uint16_t> indices = grid.GetIndices16();
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["landGeo"] = std::move(geo);
}
// Build Water
void Assignment3::BuildWavesGeometry()
{
    std::vector<std::uint16_t> indices(12 * mWaves->TriangleCount()); // 3 indices per face
	assert(mWaves->VertexCount() < 0x0000ffff);

    // Iterate over each quad.
    int m = mWaves->RowCount();
    int n = mWaves->ColumnCount();
    int k = 0;
    for(int i = 0; i < m - 1; ++i)
    {
        for(int j = 0; j < n - 1; ++j)
        {
            indices[k] = i*n + j;
            indices[k + 1] = i*n + j + 1;
            indices[k + 2] = (i + 1)*n + j;

            indices[k + 3] = (i + 1)*n + j;
            indices[k + 4] = i*n + j + 1;
            indices[k + 5] = (i + 1)*n + j + 1;

            k += 6; // next quad
        }
    }

	UINT vbByteSize = mWaves->VertexCount()*sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size()*sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "waterGeo";

	// Set dynamically.
	geo->VertexBufferCPU = nullptr;
	geo->VertexBufferGPU = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["waterGeo"] = std::move(geo);
}
// Build Box
void Assignment3::BuildBoxGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(8.0f, 8.0f, 8.0f, 3, 1,1,1);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["box"] = submesh;

	mGeometries["boxGeo"] = std::move(geo);
}
// Build WalkWays
void Assignment3::BuildGateWalkWayGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(27.95f, 0.1f, 10.0f, 3, 1,3,1);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "walk1Geo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["walk1"] = submesh;

	mGeometries["walk1Geo"] = std::move(geo);
}
void Assignment3::BuildBackWallWalkGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(95.0f, 0.1f, 4.0f, 3, 2, 12 ,1);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "walk2Geo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["walk2"] = submesh;

	mGeometries["walk2Geo"] = std::move(geo);
}
void Assignment3::BuildRightWallWalkGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(4.0f, 0.1f, 95.0f, 3, 1, 0.5, 17);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "walk3Geo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["walk3"] = submesh;

	mGeometries["walk3Geo"] = std::move(geo);
}
void Assignment3::BuildLeftWallWalkGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(4.0f, 0.1f, 95.0f, 3, 1, 0.5, 17);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "walk4Geo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["walk4"] = submesh;

	mGeometries["walk4Geo"] = std::move(geo);
}

//Build Ledges
void Assignment3::BuildGateLedgeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(8.0f, 8.0f, 8.0f, 3, 1, 1, 1);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "gateLedgeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["gateLedge"] = submesh;

	mGeometries["gateLedgeGeo"] = std::move(geo);
}
void Assignment3::BuildBackLedgeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(92.0f, 1.0f, 1.0f, 3, 1, 30, 1);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "backLedgeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["backLedge"] = submesh;

	mGeometries["backLedgeGeo"] = std::move(geo);
}
void Assignment3::BuildRightLedgeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 92.0f, 3, 1.0 , 20.0, 1.0);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "rightLedgeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["rightLedge"] = submesh;

	mGeometries["rightLedgeGeo"] = std::move(geo);
}
void Assignment3::BuildLeftLedgeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 92.0f, 3, 1.0, 20.0, 1.0);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "leftLedgeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["leftLedge"] = submesh;

	mGeometries["leftLedgeGeo"] = std::move(geo);
}

void Assignment3::BuildAvenueGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(92.0f, 1.0f, 10.0f, 3, 1, 10, 1);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "avenueGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["avenue"] = submesh;

	mGeometries["avenueGeo"] = std::move(geo);
}

// Build DrawBridge
void Assignment3::BuildDrawBridgeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(12.0f, 16.0f, 0.5f, 3, 1, 1, 1);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "drawBridgeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["drawBridge"] = submesh;

	mGeometries["drawBridgeGeo"] = std::move(geo);
}

// Build Cylinder
void Assignment3::BuildCylinderGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(5.0f, 5.0f, 10.0f, 20, 20);

	std::vector<Vertex> vertices(cylinder.Vertices.size());
	for (size_t i = 0; i < cylinder.Vertices.size(); ++i)
	{
		auto& p = cylinder.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = cylinder.Vertices[i].Normal;
		vertices[i].TexC = cylinder.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = cylinder.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "cylinderGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["cylinder"] = submesh;

	mGeometries["cylinderGeo"] = std::move(geo);
}
// Build Cone
void Assignment3::BuildConeGeometry()
{
	GeometryGenerator geoGen;
	float bottomRadius = 5.0f;
	float topRadius = 1.0f;
	float height = 10.0f;
	uint32_t sliceCount = 20;
	uint32_t stackCount = 20;

	GeometryGenerator::MeshData cone = geoGen.CreateCylinder(bottomRadius, topRadius, height, sliceCount, stackCount);

	std::vector<Vertex> vertices(cone.Vertices.size());
	for (size_t i = 0; i < cone.Vertices.size(); ++i)
	{
		auto& p = cone.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = cone.Vertices[i].Normal;
		vertices[i].TexC = cone.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = cone.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "coneGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["cone"] = submesh;

	mGeometries["coneGeo"] = std::move(geo);
}
// Build Torus
void Assignment3::BuildTorusGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData torus = geoGen.CreateTorus(0.2f, 0.75f, 6, 6);

	std::vector<Vertex> vertices(torus.Vertices.size());
	for (size_t i = 0; i < torus.Vertices.size(); ++i)
	{
		auto& p = torus.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = torus.Vertices[i].Normal;
		vertices[i].TexC = torus.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = torus.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "torusGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["torus"] = submesh;

	mGeometries["torusGeo"] = std::move(geo);
}
// Build Wedge
void Assignment3::BuildWedgeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData wedge = geoGen.CreateWedge(1.0f, 1.0f, 1.0f, 3);

	std::vector<Vertex> vertices(wedge.Vertices.size());
	for (size_t i = 0; i < wedge.Vertices.size(); ++i)
	{
		auto& p = wedge.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = wedge.Vertices[i].Normal;
		vertices[i].TexC = wedge.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = wedge.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "wedgeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["wedge"] = submesh;

	mGeometries["wedgeGeo"] = std::move(geo);
}
// Build Pyramid
void Assignment3::BuildPyramidGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData pyramid = geoGen.CreatePyramid(8.0f, 8.0f, 8.0f, 3);

	std::vector<Vertex> vertices(pyramid.Vertices.size());
	for (size_t i = 0; i < pyramid.Vertices.size(); ++i)
	{
		auto& p = pyramid.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = pyramid.Vertices[i].Normal;
		vertices[i].TexC = pyramid.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = pyramid.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "pyramidGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["pyramid"] = submesh;

	mGeometries["pyramidGeo"] = std::move(geo);
}
// Build Diamond
void Assignment3::BuildDiamondGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData diamond = geoGen.CreateDiamond(8.0f, 8.0f, 8.0f, 3);

	std::vector<Vertex> vertices(diamond.Vertices.size());
	for (size_t i = 0; i < diamond.Vertices.size(); ++i)
	{
		auto& p = diamond.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = diamond.Vertices[i].Normal;
		vertices[i].TexC = diamond.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = diamond.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "diamondGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["diamond"] = submesh;

	mGeometries["diamondGeo"] = std::move(geo);
}
// Build TriPrismZ
void Assignment3::BuildTriPrismGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData triPrism = geoGen.CreateTriPrism(8.0f, 8.0f, 8.0f, 3);

	std::vector<Vertex> vertices(triPrism.Vertices.size());
	for (size_t i = 0; i < triPrism.Vertices.size(); ++i)
	{
		auto& p = triPrism.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = triPrism.Vertices[i].Normal;
		vertices[i].TexC = triPrism.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = triPrism.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "triPrismGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["triPrism"] = submesh;

	mGeometries["triPrismGeo"] = std::move(geo);
}
// Adding Sphere
void Assignment3::BuildSphereGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(5.0f, 20, 20);

	std::vector<Vertex> vertices(sphere.Vertices.size());
	for (size_t i = 0; i < sphere.Vertices.size(); ++i)
	{
		auto& p = sphere.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = sphere.Vertices[i].Normal;
		vertices[i].TexC = sphere.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = sphere.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "sphereGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["sphere"] = submesh;

	mGeometries["sphereGeo"] = std::move(geo);
}

// Adding Trees
void Assignment3::BuildTreeSpritesGeometry()
{
	//step5
	struct TreeSpriteVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};

	static const int treeCount = 50;
	std::array<TreeSpriteVertex, 50> vertices;

	float gridSizeX = 90.0f;
	float gridSizeZ = 90.0f;
	float borderSize = 10.0f; // Space between the castle and the trees

	for (UINT i = 0; i < treeCount; ++i)
	{
		float x, z;

		// Choose a random side: 0 = left, 1 = right, 2 = top, 3 = bottom
		int side = rand() % 3;

		switch (side)
		{
		case 0:
			x = MathHelper::RandF(-65.0f - borderSize, -65.0f);
			z = MathHelper::RandF(-65.0f, 65.0f);
			break;
		case 1:
			x = MathHelper::RandF(65.0f, 65.0f + borderSize);
			z = MathHelper::RandF(-65.0f, 65.0f);
			break;
		case 2:
			x = MathHelper::RandF(-65.0f, 65.0f);
			z = MathHelper::RandF(65.0f, 65.0f + borderSize);
			break;
		}

		float y = GetHillsHeight(x, z);

		// Move tree slightly above land height.
		y += 8.0f;

		vertices[i].Pos = XMFLOAT3(x, y, z);
		vertices[i].Size = XMFLOAT2(20.0f, 20.0f);

	}

	std::array<std::uint16_t, 50> indices =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 28, 30, 31, 32, 33, 34, 35, 36, 37 ,38 ,39 , 40,
		41, 42, 43, 44, 45, 46, 47 ,48 ,49
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(TreeSpriteVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "treeSpritesGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(TreeSpriteVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["points"] = submesh;

	mGeometries["treeSpritesGeo"] = std::move(geo);
}

void Assignment3::BuildPSOs()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mStdInputLayout.data(), (UINT)mStdInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()), 
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;

	//there is abug with F2 key that is supposed to turn on the multisampling!
//Set4xMsaaState(true);
	//m4xMsaaState = true;

	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

	//
	// PSO for transparent objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//transparentPsoDesc.BlendState.AlphaToCoverageEnable = true;

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&mPSOs["transparent"])));

	//
	// PSO for alpha tested objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = opaquePsoDesc;
	alphaTestedPsoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["alphaTestedPS"]->GetBufferPointer()),
		mShaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&mPSOs["alphaTested"])));

	//
	// PSO for tree sprites
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpritePsoDesc = opaquePsoDesc;
	treeSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteVS"]->GetBufferPointer()),
		mShaders["treeSpriteVS"]->GetBufferSize()
	};
	treeSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteGS"]->GetBufferPointer()),
		mShaders["treeSpriteGS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpritePS"]->GetBufferPointer()),
		mShaders["treeSpritePS"]->GetBufferSize()
	};
	//step1
	treeSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	treeSpritePsoDesc.InputLayout = { mTreeSpriteInputLayout.data(), (UINT)mTreeSpriteInputLayout.size() };
	treeSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&treeSpritePsoDesc, IID_PPV_ARGS(&mPSOs["treeSprites"])));
}
void Assignment3::BuildFrameResources()
{
    for(int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            1, (UINT)mAllRitems.size(), (UINT)mMaterials.size(), mWaves->VertexCount()));
    }
}

// Texture Step3
void Assignment3::BuildMaterials()
{
	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->MatCBIndex = 0;
	grass->DiffuseSrvHeapIndex = 0;
	grass->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->Roughness = 0.125f;

	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->MatCBIndex = 1;
	water->DiffuseSrvHeapIndex = 1;
	water->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	water->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->Roughness = 0.0f;

	auto wirefence = std::make_unique<Material>();
	wirefence->Name = "wirefence";
	wirefence->MatCBIndex = 2;
	wirefence->DiffuseSrvHeapIndex = 2;
	wirefence->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wirefence->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	wirefence->Roughness = 0.25f;

	auto crate = std::make_unique<Material>();
	crate->Name = "crate";
	crate->MatCBIndex = 3;
	crate->DiffuseSrvHeapIndex = 3;
	crate->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	crate->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	crate->Roughness = 0.25f;

	auto wall1 = std::make_unique<Material>();
	wall1->Name = "wall1";
	wall1->MatCBIndex = 4;
	wall1->DiffuseSrvHeapIndex = 4;
	wall1->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wall1->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	wall1->Roughness = 0.25f;

	auto roof1 = std::make_unique<Material>();
	roof1->Name = "roof1";
	roof1->MatCBIndex = 5;
	roof1->DiffuseSrvHeapIndex = 5;
	roof1->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	roof1->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	roof1->Roughness = 0.25f;

	auto wood1 = std::make_unique<Material>();
	wood1->Name = "wood1";
	wood1->MatCBIndex = 6;
	wood1->DiffuseSrvHeapIndex = 6;
	wood1->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wood1->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	wood1->Roughness = 0.25f;

	auto stone1 = std::make_unique<Material>();
	stone1->Name = "stone1";
	stone1->MatCBIndex = 7;
	stone1->DiffuseSrvHeapIndex = 7;
	stone1->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	stone1->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	stone1->Roughness = 0.25f;

	auto wood2 = std::make_unique<Material>();
	wood2->Name = "wood2";
	wood2->MatCBIndex = 8;
	wood2->DiffuseSrvHeapIndex = 8;
	wood2->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wood2->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	wood2->Roughness = 0.25f;

	auto rMetal = std::make_unique<Material>();
	rMetal->Name = "rMetal";
	rMetal->MatCBIndex = 9;
	rMetal->DiffuseSrvHeapIndex = 9;
	rMetal->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	rMetal->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	rMetal->Roughness = 0.25f;	
	
	auto dirtGrass = std::make_unique<Material>();
	dirtGrass->Name = "dirtGrass";
	dirtGrass->MatCBIndex = 10;
	dirtGrass->DiffuseSrvHeapIndex = 10;
	dirtGrass->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dirtGrass->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	dirtGrass->Roughness = 0.25f;

	auto hedge = std::make_unique<Material>();
	hedge->Name = "hedge";
	hedge->MatCBIndex = 11;
	hedge->DiffuseSrvHeapIndex = 11;
	hedge->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	hedge->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	hedge->Roughness = 0.25f;

	auto treeSprites = std::make_unique<Material>();
	treeSprites->Name = "treeSprites";
	treeSprites->MatCBIndex = 12;
	treeSprites->DiffuseSrvHeapIndex = 12;
	treeSprites->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	treeSprites->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	treeSprites->Roughness = 0.125f;

	mMaterials["grass"] = std::move(grass);
	mMaterials["water"] = std::move(water);
	mMaterials["wirefence"] = std::move(wirefence);
	mMaterials["crate"] = std::move(crate);
	mMaterials["wall1"] = std::move(wall1);
	mMaterials["roof1"] = std::move(roof1);
	mMaterials["wood1"] = std::move(wood1);
	mMaterials["stone1"] = std::move(stone1);
	mMaterials["wood2"] = std::move(wood2);
	mMaterials["rMetal"] = std::move(rMetal);
	mMaterials["dirtGrass"] = std::move(dirtGrass);
	mMaterials["hedge"] = std::move(hedge);
	mMaterials["treeSprites"] = std::move(treeSprites);
}

// Adding Shapes with Textures
void Assignment3::BuildRenderItems()
{
	// Create a bounding sphere around the camera's initial position
	mCameraBounds = BoundingSphere(mPlayerPos, 1.0f);

    auto wavesRitem = std::make_unique<RenderItem>();
    wavesRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&wavesRitem->World, XMMatrixScaling(1.25f, 1.0f, 0.08f) * XMMatrixTranslation(0.0f, -1.0f, -73.0f));
	wavesRitem->ObjCBIndex = 0;
	wavesRitem->Mat = mMaterials["water"].get();
	wavesRitem->Geo = mGeometries["waterGeo"].get();
	wavesRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem->IndexCount = wavesRitem->Geo->DrawArgs["grid"].IndexCount;
	wavesRitem->StartIndexLocation = wavesRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	wavesRitem->BaseVertexLocation = wavesRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
    mWavesRitem = wavesRitem.get();
	mRitemLayer[(int)RenderLayer::Transparent].push_back(wavesRitem.get());

    auto gridRitem = std::make_unique<RenderItem>();
    gridRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&gridRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) + XMMatrixTranslation(0.0f, 0.0f, -160.0f));
	XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(10.0f, 25.0f, 1.0f));
	gridRitem->ObjCBIndex = 1;
	gridRitem->Mat = mMaterials["grass"].get();
	gridRitem->Geo = mGeometries["landGeo"].get();
	gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
    gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
    gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());

	auto Gate = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&Gate->World, XMMatrixScaling(2.0f, 2.0f, -1.0f) + XMMatrixTranslation(0.0f, 11.0f, -110.0f));
	Gate->Bounds = DirectX::BoundingBox(XMFLOAT3(43.0f, 5.0f, 105.0f), XMFLOAT3(53.0f, 15.0f, 115.0f));
	Gate->ObjCBIndex = 2;
	Gate->Mat = mMaterials["wirefence"].get();
	Gate->Geo = mGeometries["boxGeo"].get();
	Gate->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Gate->IndexCount = Gate->Geo->DrawArgs["box"].IndexCount;
	Gate->StartIndexLocation = Gate->Geo->DrawArgs["box"].StartIndexLocation;
	Gate->BaseVertexLocation = Gate->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(Gate.get());


	auto crate1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&crate1->World, XMMatrixTranslation(85.0f, 5.0f, 100.0f) + XMMatrixScaling( 0.1f, 0.1f, 0.1f));
	crate1->ObjCBIndex = 3;
	crate1->Mat = mMaterials["crate"].get();
	crate1->Geo = mGeometries["boxGeo"].get();
	crate1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	crate1->IndexCount = crate1->Geo->DrawArgs["box"].IndexCount;
	crate1->StartIndexLocation = crate1->Geo->DrawArgs["box"].StartIndexLocation;
	crate1->BaseVertexLocation = crate1->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(crate1.get());


	// Walls

	// Back Walls
	auto bWall1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bWall1->World, XMMatrixTranslation(0.0f, 10.0f, 110.0f) + XMMatrixScaling(5.0f, 2.0f, 0.0f));
	bWall1->ObjCBIndex = 4;
	bWall1->Mat = mMaterials["wall1"].get();
	bWall1->Geo = mGeometries["boxGeo"].get();
	bWall1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bWall1->IndexCount = bWall1->Geo->DrawArgs["box"].IndexCount;
	bWall1->StartIndexLocation = bWall1->Geo->DrawArgs["box"].StartIndexLocation;
	bWall1->BaseVertexLocation = bWall1->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bWall1.get());

	auto bWall2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bWall2->World, XMMatrixTranslation(48.0f, 10.0f, 110.0f) + XMMatrixScaling(5.0f, 2.0f, 0.0f));
	bWall2->ObjCBIndex = 5;
	bWall2->Mat = mMaterials["wall1"].get();
	bWall2->Geo = mGeometries["boxGeo"].get();
	bWall2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bWall2->IndexCount = bWall2->Geo->DrawArgs["box"].IndexCount;
	bWall2->StartIndexLocation = bWall2->Geo->DrawArgs["box"].StartIndexLocation;
	bWall2->BaseVertexLocation = bWall2->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bWall2.get());

	auto bWall3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bWall3->World, XMMatrixTranslation(-48.0f, 10.0f, 110.0f) + XMMatrixScaling(5.0f, 2.0f, 0.0f));
	bWall3->ObjCBIndex = 6;
	bWall3->Mat = mMaterials["wall1"].get();
	bWall3->Geo = mGeometries["boxGeo"].get();
	bWall3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bWall3->IndexCount = bWall3->Geo->DrawArgs["box"].IndexCount;
	bWall3->StartIndexLocation = bWall3->Geo->DrawArgs["box"].StartIndexLocation;
	bWall3->BaseVertexLocation = bWall3->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bWall3.get());

	auto bWall4 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bWall4->World, XMMatrixTranslation(-96.0f, 10.0f, 110.0f) + XMMatrixScaling(5.0f, 2.0f, 0.0f));
	bWall4->ObjCBIndex = 7;
	bWall4->Mat = mMaterials["wall1"].get();
	bWall4->Geo = mGeometries["boxGeo"].get();
	bWall4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bWall4->IndexCount = bWall4->Geo->DrawArgs["box"].IndexCount;
	bWall4->StartIndexLocation = bWall4->Geo->DrawArgs["box"].StartIndexLocation;
	bWall4->BaseVertexLocation = bWall4->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bWall4.get());

	auto bWall5 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bWall5->World, XMMatrixTranslation(96.0f, 10.0f, 110.0f) + XMMatrixScaling(5.0f, 2.0f, 0.0f));
	bWall5->ObjCBIndex = 8;
	bWall5->Mat = mMaterials["wall1"].get();
	bWall5->Geo = mGeometries["boxGeo"].get();
	bWall5->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bWall5->IndexCount = bWall5->Geo->DrawArgs["box"].IndexCount;
	bWall5->StartIndexLocation = bWall5->Geo->DrawArgs["box"].StartIndexLocation;
	bWall5->BaseVertexLocation = bWall5->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bWall5.get());

	// Front Walls
	auto fWall1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fWall1->World, XMMatrixTranslation(48.0f, 10.0f, -110.0f) + XMMatrixScaling(5.0f, 2.0f, 0.0f));
	fWall1->ObjCBIndex = 9;
	fWall1->Mat = mMaterials["wall1"].get();
	fWall1->Geo = mGeometries["boxGeo"].get();
	fWall1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fWall1->IndexCount = fWall1->Geo->DrawArgs["box"].IndexCount;
	fWall1->StartIndexLocation = fWall1->Geo->DrawArgs["box"].StartIndexLocation;
	fWall1->BaseVertexLocation = fWall1->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fWall1.get());

	auto fWall2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fWall2->World, XMMatrixTranslation(-48.0f, 10.0f, -110.0f) + XMMatrixScaling(5.0f, 2.0f, 0.0f));
	fWall2->ObjCBIndex = 10;
	fWall2->Mat = mMaterials["wall1"].get();
	fWall2->Geo = mGeometries["boxGeo"].get();
	fWall2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fWall2->IndexCount = fWall2->Geo->DrawArgs["box"].IndexCount;
	fWall2->StartIndexLocation = fWall2->Geo->DrawArgs["box"].StartIndexLocation;
	fWall2->BaseVertexLocation = fWall2->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fWall2.get());

	auto fWall3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fWall3->World, XMMatrixTranslation(-96.0f, 10.0f, -110.0f) + XMMatrixScaling(5.0f, 2.0f, 0.0f));
	fWall3->ObjCBIndex = 11;
	fWall3->Mat = mMaterials["wall1"].get();
	fWall3->Geo = mGeometries["boxGeo"].get();
	fWall3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fWall3->IndexCount = fWall3->Geo->DrawArgs["box"].IndexCount;
	fWall3->StartIndexLocation = fWall3->Geo->DrawArgs["box"].StartIndexLocation;
	fWall3->BaseVertexLocation = fWall3->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fWall3.get());

	auto fWall4 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fWall4->World, XMMatrixTranslation(96.0f, 10.0f, -110.0f) + XMMatrixScaling(5.0f, 2.0f, 0.0f));
	fWall4->ObjCBIndex = 12;
	fWall4->Mat = mMaterials["wall1"].get();
	fWall4->Geo = mGeometries["boxGeo"].get();
	fWall4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fWall4->IndexCount = fWall4->Geo->DrawArgs["box"].IndexCount;
	fWall4->StartIndexLocation = fWall4->Geo->DrawArgs["box"].StartIndexLocation;
	fWall4->BaseVertexLocation = fWall4->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fWall4.get());


	auto fWallLedge1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fWallLedge1->World, XMMatrixTranslation(58.5f, 23.0f, -113.0f) + XMMatrixScaling(7.0f, -0.7f, -0.7f));
	fWallLedge1->ObjCBIndex = 13;
	fWallLedge1->Mat = mMaterials["wall1"].get();
	fWallLedge1->Geo = mGeometries["boxGeo"].get();
	fWallLedge1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fWallLedge1->IndexCount = fWallLedge1->Geo->DrawArgs["box"].IndexCount;
	fWallLedge1->StartIndexLocation = fWallLedge1->Geo->DrawArgs["box"].StartIndexLocation;
	fWallLedge1->BaseVertexLocation = fWallLedge1->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fWallLedge1.get());

	auto fWallLedge2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fWallLedge2->World, XMMatrixTranslation(-58.5f, 23.0f, -113.0f) + XMMatrixScaling(7.0f, -0.7f, -0.7f));
	fWallLedge2->ObjCBIndex = 14;
	fWallLedge2->Mat = mMaterials["wall1"].get();
	fWallLedge2->Geo = mGeometries["boxGeo"].get();
	fWallLedge2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fWallLedge2->IndexCount = fWallLedge2->Geo->DrawArgs["box"].IndexCount;
	fWallLedge2->StartIndexLocation = fWallLedge2->Geo->DrawArgs["box"].StartIndexLocation;
	fWallLedge2->BaseVertexLocation = fWallLedge2->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fWallLedge2.get());

	// Right Walls
	auto rWall1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&rWall1->World, XMMatrixTranslation(110.0f, 10.0f, 0.0f) + XMMatrixScaling(0.0f, 2.0f, 05.0f));
	rWall1->ObjCBIndex = 15;
	rWall1->Mat = mMaterials["wall1"].get();
	rWall1->Geo = mGeometries["boxGeo"].get();
	rWall1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	rWall1->IndexCount = rWall1->Geo->DrawArgs["box"].IndexCount;
	rWall1->StartIndexLocation = rWall1->Geo->DrawArgs["box"].StartIndexLocation;
	rWall1->BaseVertexLocation = rWall1->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(rWall1.get());

	auto rWall2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&rWall2->World, XMMatrixTranslation(110.0f, 10.0f, 48.0f) + XMMatrixScaling(0.0f, 2.0f, 5.0f));
	rWall2->ObjCBIndex = 16;
	rWall2->Mat = mMaterials["wall1"].get();
	rWall2->Geo = mGeometries["boxGeo"].get();
	rWall2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	rWall2->IndexCount = rWall2->Geo->DrawArgs["box"].IndexCount;
	rWall2->StartIndexLocation = rWall2->Geo->DrawArgs["box"].StartIndexLocation;
	rWall2->BaseVertexLocation = rWall2->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(rWall2.get());

	auto rWall3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&rWall3->World, XMMatrixTranslation(110.0f, 10.0f, -48.0f) + XMMatrixScaling(0.0f, 2.0f, 5.0f));
	rWall3->ObjCBIndex = 17;
	rWall3->Mat = mMaterials["wall1"].get();
	rWall3->Geo = mGeometries["boxGeo"].get();
	rWall3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	rWall3->IndexCount = rWall3->Geo->DrawArgs["box"].IndexCount;
	rWall3->StartIndexLocation = rWall3->Geo->DrawArgs["box"].StartIndexLocation;
	rWall3->BaseVertexLocation = rWall3->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(rWall3.get());

	auto rWall4 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&rWall4->World, XMMatrixTranslation(110.0f, 10.0f, 96.0f) + XMMatrixScaling(0.0f, 2.0f, 5.0f));
	rWall4->ObjCBIndex = 18;
	rWall4->Mat = mMaterials["wall1"].get();
	rWall4->Geo = mGeometries["boxGeo"].get();
	rWall4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	rWall4->IndexCount = rWall4->Geo->DrawArgs["box"].IndexCount;
	rWall4->StartIndexLocation = rWall4->Geo->DrawArgs["box"].StartIndexLocation;
	rWall4->BaseVertexLocation = rWall4->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(rWall4.get());

	auto rWall5 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&rWall5->World, XMMatrixTranslation(110.0f, 10.0f, -96.0f) + XMMatrixScaling(0.0f, 2.0f, 5.0f));
	rWall5->ObjCBIndex = 19;
	rWall5->Mat = mMaterials["wall1"].get();
	rWall5->Geo = mGeometries["boxGeo"].get();
	rWall5->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	rWall5->IndexCount = rWall5->Geo->DrawArgs["box"].IndexCount;
	rWall5->StartIndexLocation = rWall5->Geo->DrawArgs["box"].StartIndexLocation;
	rWall5->BaseVertexLocation = rWall5->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(rWall5.get());

	// Left Walls
	auto lWall1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lWall1->World, XMMatrixTranslation(-110.0f, 10.0f, 0.0f) + XMMatrixScaling(0.0f, 2.0f, 05.0f));
	lWall1->ObjCBIndex = 20;
	lWall1->Mat = mMaterials["wall1"].get();
	lWall1->Geo = mGeometries["boxGeo"].get();
	lWall1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lWall1->IndexCount = lWall1->Geo->DrawArgs["box"].IndexCount;
	lWall1->StartIndexLocation = lWall1->Geo->DrawArgs["box"].StartIndexLocation;
	lWall1->BaseVertexLocation = lWall1->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(lWall1.get());

	auto lWall2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lWall2->World, XMMatrixTranslation(-110.0f, 10.0f, 48.0f) + XMMatrixScaling(0.0f, 2.0f, 5.0f));
	lWall2->ObjCBIndex = 21;
	lWall2->Mat = mMaterials["wall1"].get();
	lWall2->Geo = mGeometries["boxGeo"].get();
	lWall2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lWall2->IndexCount = lWall2->Geo->DrawArgs["box"].IndexCount;
	lWall2->StartIndexLocation = lWall2->Geo->DrawArgs["box"].StartIndexLocation;
	lWall2->BaseVertexLocation = lWall2->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(lWall2.get());

	auto lWall3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lWall3->World, XMMatrixTranslation(-110.0f, 10.0f, -48.0f) + XMMatrixScaling(0.0f, 2.0f, 5.0f));
	lWall3->ObjCBIndex = 22;
	lWall3->Mat = mMaterials["wall1"].get();
	lWall3->Geo = mGeometries["boxGeo"].get();
	lWall3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lWall3->IndexCount = lWall3->Geo->DrawArgs["box"].IndexCount;
	lWall3->StartIndexLocation = lWall3->Geo->DrawArgs["box"].StartIndexLocation;
	lWall3->BaseVertexLocation = lWall3->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(lWall3.get());

	auto lWall4 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lWall4->World, XMMatrixTranslation(-110.0f, 10.0f, 96.0f) + XMMatrixScaling(0.0f, 2.0f, 5.0f));
	lWall4->ObjCBIndex = 23;
	lWall4->Mat = mMaterials["wall1"].get();
	lWall4->Geo = mGeometries["boxGeo"].get();
	lWall4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lWall4->IndexCount = lWall4->Geo->DrawArgs["box"].IndexCount;
	lWall4->StartIndexLocation = lWall4->Geo->DrawArgs["box"].StartIndexLocation;
	lWall4->BaseVertexLocation = lWall4->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(lWall4.get());

	auto lWall5 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lWall5->World, XMMatrixTranslation(-110.0f, 10.0f, -96.0f) + XMMatrixScaling(0.0f, 2.0f, 5.0f));
	lWall5->ObjCBIndex = 24;
	lWall5->Mat = mMaterials["wall1"].get();
	lWall5->Geo = mGeometries["boxGeo"].get();
	lWall5->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lWall5->IndexCount = lWall5->Geo->DrawArgs["box"].IndexCount;
	lWall5->StartIndexLocation = lWall5->Geo->DrawArgs["box"].StartIndexLocation;
	lWall5->BaseVertexLocation = lWall5->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(lWall5.get());

	auto fRTower = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fRTower->World, XMMatrixTranslation(110.0f, 22.0f, -110.0f) + XMMatrixScaling(3.0f, 4.0f, 3.0f));
	fRTower->ObjCBIndex = 25;
	fRTower->Mat = mMaterials["wall1"].get();
	fRTower->Geo = mGeometries["cylinderGeo"].get();
	fRTower->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fRTower->IndexCount = fRTower->Geo->DrawArgs["cylinder"].IndexCount;
	fRTower->StartIndexLocation = fRTower->Geo->DrawArgs["cylinder"].StartIndexLocation;
	fRTower->BaseVertexLocation = fRTower->Geo->DrawArgs["cylinder"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fRTower.get());

	auto fLTower = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fLTower->World, XMMatrixTranslation(-110.0f, 22.0f, -110.0f) + XMMatrixScaling(3.0f, 4.0f, 3.0f));
	fLTower->ObjCBIndex = 26;
	fLTower->Mat = mMaterials["wall1"].get();
	fLTower->Geo = mGeometries["cylinderGeo"].get();
	fLTower->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fLTower->IndexCount = fLTower->Geo->DrawArgs["cylinder"].IndexCount;
	fLTower->StartIndexLocation = fLTower->Geo->DrawArgs["cylinder"].StartIndexLocation;
	fLTower->BaseVertexLocation = fLTower->Geo->DrawArgs["cylinder"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fLTower.get());

	auto bRTower = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bRTower->World, XMMatrixTranslation(-110.0f, 22.0f, 110.0f) + XMMatrixScaling(3.0f, 4.0f, 3.0f));
	bRTower->ObjCBIndex = 27;
	bRTower->Mat = mMaterials["wall1"].get();
	bRTower->Geo = mGeometries["cylinderGeo"].get();
	bRTower->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bRTower->IndexCount = bRTower->Geo->DrawArgs["cylinder"].IndexCount;
	bRTower->StartIndexLocation = bRTower->Geo->DrawArgs["cylinder"].StartIndexLocation;
	bRTower->BaseVertexLocation = bRTower->Geo->DrawArgs["cylinder"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bRTower.get());

	auto bLTower = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bLTower->World, XMMatrixTranslation(110.0f, 22.0f, 110.0f) + XMMatrixScaling(3.0f, 4.0f, 3.0f));
	bLTower->ObjCBIndex = 28;
	bLTower->Mat = mMaterials["wall1"].get();
	bLTower->Geo = mGeometries["cylinderGeo"].get();
	bLTower->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bLTower->IndexCount = bLTower->Geo->DrawArgs["cylinder"].IndexCount;
	bLTower->StartIndexLocation = bLTower->Geo->DrawArgs["cylinder"].StartIndexLocation;
	bLTower->BaseVertexLocation = bLTower->Geo->DrawArgs["cylinder"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bLTower.get());

	auto fRCone = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fRCone->World, XMMatrixTranslation(110.0f, 60.0f, -110.0f) + XMMatrixScaling(4.0f, 2.0f, 4.0f));;
	fRCone->ObjCBIndex = 29;
	fRCone->Mat = mMaterials["roof1"].get();
	fRCone->Geo = mGeometries["coneGeo"].get();
	fRCone->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fRCone->IndexCount = fRCone->Geo->DrawArgs["cone"].IndexCount;
	fRCone->StartIndexLocation = fRCone->Geo->DrawArgs["cone"].StartIndexLocation;
	fRCone->BaseVertexLocation = fRCone->Geo->DrawArgs["cone"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fRCone.get());

	auto fLCone = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fLCone->World, XMMatrixTranslation(-110.0f, 60.0f, -110.0f) + XMMatrixScaling(4.0f, 2.0f, 4.0f));
	fLCone->ObjCBIndex = 30;
	fLCone->Mat = mMaterials["roof1"].get();
	fLCone->Geo = mGeometries["coneGeo"].get();
	fLCone->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fLCone->IndexCount = fLCone->Geo->DrawArgs["cone"].IndexCount;
	fLCone->StartIndexLocation = fLCone->Geo->DrawArgs["cone"].StartIndexLocation;
	fLCone->BaseVertexLocation = fLCone->Geo->DrawArgs["cone"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fLCone.get());

	auto bRCone = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bRCone->World, XMMatrixTranslation(-110.0f, 60.0f, 110.0f) + XMMatrixScaling(4.0f, 2.0f, 4.0f));
	bRCone->ObjCBIndex = 31;
	bRCone->Mat = mMaterials["roof1"].get();
	bRCone->Geo = mGeometries["coneGeo"].get();
	bRCone->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bRCone->IndexCount = bRCone->Geo->DrawArgs["cone"].IndexCount;
	bRCone->StartIndexLocation = bRCone->Geo->DrawArgs["cone"].StartIndexLocation;
	bRCone->BaseVertexLocation = bRCone->Geo->DrawArgs["cone"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bRCone.get());

	auto bLCone = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bLCone->World, XMMatrixTranslation(110.0f, 60.0f, 110.0f) + XMMatrixScaling(4.0f, 2.0f, 4.0f));
	bLCone->ObjCBIndex = 32;
	bLCone->Mat = mMaterials["roof1"].get();
	bLCone->Geo = mGeometries["coneGeo"].get();
	bLCone->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bLCone->IndexCount = bLCone->Geo->DrawArgs["cone"].IndexCount;
	bLCone->StartIndexLocation = bLCone->Geo->DrawArgs["cone"].StartIndexLocation;
	bLCone->BaseVertexLocation = bLCone->Geo->DrawArgs["cone"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bLCone.get());

	// Front Gates
	auto lGateWall = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lGateWall->World, XMMatrixTranslation(-20.0f, 15.0f, -110.0f) + XMMatrixScaling(1.0f, 3.0f, 2.0f));
	lGateWall->ObjCBIndex = 33;
	lGateWall->Mat = mMaterials["wall1"].get();
	lGateWall->Geo = mGeometries["boxGeo"].get();
	lGateWall->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lGateWall->IndexCount = lGateWall->Geo->DrawArgs["box"].IndexCount;
	lGateWall->StartIndexLocation = lGateWall->Geo->DrawArgs["box"].StartIndexLocation;
	lGateWall->BaseVertexLocation = lGateWall->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(lGateWall.get());

	auto rGateWall = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&rGateWall->World, XMMatrixTranslation(20.0f, 15.0f, -110.0f) + XMMatrixScaling(1.0f, 3.0f, 2.0f));
	rGateWall->ObjCBIndex = 34;
	rGateWall->Mat = mMaterials["wall1"].get();
	rGateWall->Geo = mGeometries["boxGeo"].get();
	rGateWall->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	rGateWall->IndexCount = rGateWall->Geo->DrawArgs["box"].IndexCount;
	rGateWall->StartIndexLocation = rGateWall->Geo->DrawArgs["box"].StartIndexLocation;
	rGateWall->BaseVertexLocation = rGateWall->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(rGateWall.get());

	auto gateHouseRoof = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&gateHouseRoof->World, XMMatrixTranslation(0.0f, 27.0f, -110.0f) + XMMatrixScaling(2.0f, 0.0f, 2.0f));
	gateHouseRoof->ObjCBIndex = 35;
	gateHouseRoof->Mat = mMaterials["wall1"].get();
	gateHouseRoof->Geo = mGeometries["boxGeo"].get();
	gateHouseRoof->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gateHouseRoof->IndexCount = gateHouseRoof->Geo->DrawArgs["box"].IndexCount;
	gateHouseRoof->StartIndexLocation = gateHouseRoof->Geo->DrawArgs["box"].StartIndexLocation;
	gateHouseRoof->BaseVertexLocation = gateHouseRoof->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(gateHouseRoof.get());

	auto gateHouseFloor = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&gateHouseFloor->World, XMMatrixTranslation(0.0f, 0.5f, -55.0f));
	gateHouseFloor->ObjCBIndex = 36;
	gateHouseFloor->Mat = mMaterials["wood1"].get();
	gateHouseFloor->Geo = mGeometries["walk1Geo"].get();
	gateHouseFloor->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gateHouseFloor->IndexCount = gateHouseFloor->Geo->DrawArgs["walk1"].IndexCount;
	gateHouseFloor->StartIndexLocation = gateHouseFloor->Geo->DrawArgs["walk1"].StartIndexLocation;
	gateHouseFloor->BaseVertexLocation = gateHouseFloor->Geo->DrawArgs["walk1"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(gateHouseFloor.get());


	auto gateHouseWalkWay = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&gateHouseWalkWay->World, XMMatrixTranslation(0.0f, 15.5f, -55.0f));
	gateHouseWalkWay->ObjCBIndex = 37;
	gateHouseWalkWay->Mat = mMaterials["wood1"].get();
	gateHouseWalkWay->Geo = mGeometries["walk1Geo"].get();
	gateHouseWalkWay->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gateHouseWalkWay->IndexCount = gateHouseWalkWay->Geo->DrawArgs["walk1"].IndexCount;
	gateHouseWalkWay->StartIndexLocation = gateHouseWalkWay->Geo->DrawArgs["walk1"].StartIndexLocation;
	gateHouseWalkWay->BaseVertexLocation = gateHouseWalkWay->Geo->DrawArgs["walk1"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(gateHouseWalkWay.get());

	auto fgateHouseLedgeMid = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fgateHouseLedgeMid->World, XMMatrixTranslation(0.0f, 32.2f, -120.8f) + XMMatrixScaling(2.0f, -0.7f, -0.7f));
	fgateHouseLedgeMid->ObjCBIndex = 38;
	fgateHouseLedgeMid->Mat = mMaterials["wall1"].get();
	fgateHouseLedgeMid->Geo = mGeometries["boxGeo"].get();
	fgateHouseLedgeMid->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fgateHouseLedgeMid->IndexCount = fgateHouseLedgeMid->Geo->DrawArgs["box"].IndexCount;
	fgateHouseLedgeMid->StartIndexLocation = fgateHouseLedgeMid->Geo->DrawArgs["box"].StartIndexLocation;
	fgateHouseLedgeMid->BaseVertexLocation = fgateHouseLedgeMid->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fgateHouseLedgeMid.get());

	auto fgateHouseLedgeLeft = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fgateHouseLedgeLeft->World, XMMatrixTranslation(20.0f, 32.2f, -120.8f) + XMMatrixScaling(1.0f, -0.7f, -0.7f));
	fgateHouseLedgeLeft->ObjCBIndex = 39;
	fgateHouseLedgeLeft->Mat = mMaterials["wall1"].get();
	fgateHouseLedgeLeft->Geo = mGeometries["boxGeo"].get();
	fgateHouseLedgeLeft->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fgateHouseLedgeLeft->IndexCount = fgateHouseLedgeLeft->Geo->DrawArgs["box"].IndexCount;
	fgateHouseLedgeLeft->StartIndexLocation = fgateHouseLedgeLeft->Geo->DrawArgs["box"].StartIndexLocation;
	fgateHouseLedgeLeft->BaseVertexLocation = fgateHouseLedgeLeft->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fgateHouseLedgeLeft.get());

	auto fgateHouseLedgeRight = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fgateHouseLedgeRight->World, XMMatrixTranslation(-20.0f, 32.2f, -120.8f) + XMMatrixScaling(1.0f, -0.7f, -0.7f));
	fgateHouseLedgeRight->ObjCBIndex = 40;
	fgateHouseLedgeRight->Mat = mMaterials["wall1"].get();
	fgateHouseLedgeRight->Geo = mGeometries["boxGeo"].get();
	fgateHouseLedgeRight->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fgateHouseLedgeRight->IndexCount = fgateHouseLedgeRight->Geo->DrawArgs["box"].IndexCount;
	fgateHouseLedgeRight->StartIndexLocation = fgateHouseLedgeRight->Geo->DrawArgs["box"].StartIndexLocation;
	fgateHouseLedgeRight->BaseVertexLocation = fgateHouseLedgeRight->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fgateHouseLedgeRight.get());

	auto fgateHouseLedge2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fgateHouseLedge2->World, XMMatrixTranslation(26.8f, 32.2f, -115.0f) + XMMatrixScaling(-0.7f, -0.7f, 0.2f));
	fgateHouseLedge2->ObjCBIndex = 41;
	fgateHouseLedge2->Mat = mMaterials["wall1"].get();
	fgateHouseLedge2->Geo = mGeometries["boxGeo"].get();
	fgateHouseLedge2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fgateHouseLedge2->IndexCount = fgateHouseLedge2->Geo->DrawArgs["box"].IndexCount;
	fgateHouseLedge2->StartIndexLocation = fgateHouseLedge2->Geo->DrawArgs["box"].StartIndexLocation;
	fgateHouseLedge2->BaseVertexLocation = fgateHouseLedge2->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fgateHouseLedge2.get());

	auto fgateHouseLedge3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fgateHouseLedge3->World, XMMatrixTranslation(-26.8f, 32.2f, -115.0f) + XMMatrixScaling(-0.7f, -0.7f, 0.2f));
	fgateHouseLedge3->ObjCBIndex = 42;
	fgateHouseLedge3->Mat = mMaterials["wall1"].get();
	fgateHouseLedge3->Geo = mGeometries["boxGeo"].get();
	fgateHouseLedge3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fgateHouseLedge3->IndexCount = fgateHouseLedge3->Geo->DrawArgs["box"].IndexCount;
	fgateHouseLedge3->StartIndexLocation = fgateHouseLedge3->Geo->DrawArgs["box"].StartIndexLocation;
	fgateHouseLedge3->BaseVertexLocation = fgateHouseLedge3->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fgateHouseLedge3.get());

	auto bgateHouseLedgeMid = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bgateHouseLedgeMid->World, XMMatrixTranslation(0.0f, 32.2f, -98.8f) + XMMatrixScaling(2.0f, -0.7f, -0.7f));
	bgateHouseLedgeMid->ObjCBIndex = 43;
	bgateHouseLedgeMid->Mat = mMaterials["wall1"].get();
	bgateHouseLedgeMid->Geo = mGeometries["boxGeo"].get();
	bgateHouseLedgeMid->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bgateHouseLedgeMid->IndexCount = bgateHouseLedgeMid->Geo->DrawArgs["box"].IndexCount;
	bgateHouseLedgeMid->StartIndexLocation = bgateHouseLedgeMid->Geo->DrawArgs["box"].StartIndexLocation;
	bgateHouseLedgeMid->BaseVertexLocation = bgateHouseLedgeMid->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bgateHouseLedgeMid.get());

	auto bgateHouseLedgeLeft = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bgateHouseLedgeLeft->World, XMMatrixTranslation(20.0f, 32.2f, -98.8f) + XMMatrixScaling(1.0f, -0.7f, -0.7f));
	bgateHouseLedgeLeft->ObjCBIndex = 44;
	bgateHouseLedgeLeft->Mat = mMaterials["wall1"].get();
	bgateHouseLedgeLeft->Geo = mGeometries["boxGeo"].get();
	bgateHouseLedgeLeft->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bgateHouseLedgeLeft->IndexCount = bgateHouseLedgeLeft->Geo->DrawArgs["box"].IndexCount;
	bgateHouseLedgeLeft->StartIndexLocation = bgateHouseLedgeLeft->Geo->DrawArgs["box"].StartIndexLocation;
	bgateHouseLedgeLeft->BaseVertexLocation = bgateHouseLedgeLeft->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bgateHouseLedgeLeft.get());

	auto bgateHouseLedgeRight = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bgateHouseLedgeRight->World, XMMatrixTranslation(-20.0f, 32.2f, -98.8f) + XMMatrixScaling(1.0f, -0.7f, -0.7f));
	bgateHouseLedgeRight->ObjCBIndex = 45;
	bgateHouseLedgeRight->Mat = mMaterials["wall1"].get();
	bgateHouseLedgeRight->Geo = mGeometries["boxGeo"].get();
	bgateHouseLedgeRight->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bgateHouseLedgeRight->IndexCount = bgateHouseLedgeRight->Geo->DrawArgs["box"].IndexCount;
	bgateHouseLedgeRight->StartIndexLocation = bgateHouseLedgeRight->Geo->DrawArgs["box"].StartIndexLocation;
	bgateHouseLedgeRight->BaseVertexLocation = bgateHouseLedgeRight->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bgateHouseLedgeRight.get());

	// GATEHOUSE Parafets
	// Front
	std::vector<std::unique_ptr<RenderItem>> wedgeItems(13);
	int objCBIndexOffset = 46;
	float wedgeXOffset = 4.0f;
	for (int i = 0; i < 13; i++) {
		wedgeItems[i] = std::make_unique<RenderItem>();
		float xPos = (i % 2 == 0 ? 1 : -1) * wedgeXOffset * ((i + 1) / 2);
		XMStoreFloat4x4(&wedgeItems[i]->World, XMMatrixTranslation(xPos, 34.4f, -121.0f) + XMMatrixScaling(1.0f, 1.0f, 1.0f));
		wedgeItems[i]->ObjCBIndex = objCBIndexOffset + i;
		wedgeItems[i]->Mat = mMaterials["wall1"].get();
		wedgeItems[i]->Geo = mGeometries["wedgeGeo"].get();
		wedgeItems[i]->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		wedgeItems[i]->IndexCount = wedgeItems[i]->Geo->DrawArgs["wedge"].IndexCount;
		wedgeItems[i]->StartIndexLocation = wedgeItems[i]->Geo->DrawArgs["wedge"].StartIndexLocation;
		wedgeItems[i]->BaseVertexLocation = wedgeItems[i]->Geo->DrawArgs["wedge"].BaseVertexLocation;
		mRitemLayer[(int)RenderLayer::AlphaTested].push_back(wedgeItems[i].get());
		mAllRitems.push_back(std::move(wedgeItems[i]));
	}

	auto fRWallWalk = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fRWallWalk->World, XMMatrixScaling(0.2f, 1.0f, 0.02f) + XMMatrixTranslation(42.0f, 22.1f, -107.0f));
	fRWallWalk->ObjCBIndex = 59;
	fRWallWalk->Mat = mMaterials["wood1"].get();
	fRWallWalk->Geo = mGeometries["walk1Geo"].get();
	fRWallWalk->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fRWallWalk->IndexCount = fRWallWalk->Geo->DrawArgs["walk1"].IndexCount;
	fRWallWalk->StartIndexLocation = fRWallWalk->Geo->DrawArgs["walk1"].StartIndexLocation;
	fRWallWalk->BaseVertexLocation = fRWallWalk->Geo->DrawArgs["walk1"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fRWallWalk.get());

	auto fRWallWalk2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fRWallWalk2->World, XMMatrixScaling(0.2f, 1.0f, 0.02f) + XMMatrixTranslation(75.0f, 22.1f, -107.0f));
	fRWallWalk2->ObjCBIndex = 60;
	fRWallWalk2->Mat = mMaterials["wood1"].get();
	fRWallWalk2->Geo = mGeometries["walk1Geo"].get();
	fRWallWalk2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fRWallWalk2->IndexCount = fRWallWalk2->Geo->DrawArgs["walk1"].IndexCount;
	fRWallWalk2->StartIndexLocation = fRWallWalk2->Geo->DrawArgs["walk1"].StartIndexLocation;
	fRWallWalk2->BaseVertexLocation = fRWallWalk2->Geo->DrawArgs["walk1"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fRWallWalk2.get());

	auto fLWallWalk = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fLWallWalk->World, XMMatrixScaling(0.2f, 1.0f, 0.02f) + XMMatrixTranslation(-75.0f, 22.0f, -107.0f));
	fLWallWalk->ObjCBIndex = 61;
	fLWallWalk->Mat = mMaterials["wood1"].get();
	fLWallWalk->Geo = mGeometries["walk1Geo"].get();
	fLWallWalk->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fLWallWalk->IndexCount = fLWallWalk->Geo->DrawArgs["walk1"].IndexCount;
	fLWallWalk->StartIndexLocation = fLWallWalk->Geo->DrawArgs["walk1"].StartIndexLocation;
	fLWallWalk->BaseVertexLocation = fLWallWalk->Geo->DrawArgs["walk1"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fLWallWalk.get());

	auto fLWallWalk2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&fLWallWalk2->World, XMMatrixScaling(0.2f, 1.0f, 0.02f) + XMMatrixTranslation(-42.0f, 22.0f, -107.0f));
	fLWallWalk2->ObjCBIndex = 62;
	fLWallWalk2->Mat = mMaterials["wood1"].get();
	fLWallWalk2->Geo = mGeometries["walk1Geo"].get();
	fLWallWalk2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	fLWallWalk2->IndexCount = fLWallWalk2->Geo->DrawArgs["walk1"].IndexCount;
	fLWallWalk2->StartIndexLocation = fLWallWalk2->Geo->DrawArgs["walk1"].StartIndexLocation;
	fLWallWalk2->BaseVertexLocation = fLWallWalk2->Geo->DrawArgs["walk1"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(fLWallWalk2.get());

	auto lWallWalkWay = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lWallWalkWay->World,XMMatrixTranslation(-53.0f, 11.0f, 0.0f));
	lWallWalkWay->ObjCBIndex = 63;
	lWallWalkWay->Mat = mMaterials["wood1"].get();
	lWallWalkWay->Geo = mGeometries["walk4Geo"].get();
	lWallWalkWay->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lWallWalkWay->IndexCount = lWallWalkWay->Geo->DrawArgs["walk4"].IndexCount;
	lWallWalkWay->StartIndexLocation = lWallWalkWay->Geo->DrawArgs["walk4"].StartIndexLocation;
	lWallWalkWay->BaseVertexLocation = lWallWalkWay->Geo->DrawArgs["walk4"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(lWallWalkWay.get());

	auto rWallWalkWay = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&rWallWalkWay->World, XMMatrixTranslation(53.0f, 11.0f, 0.0f));
	rWallWalkWay->ObjCBIndex = 64;
	rWallWalkWay->Mat = mMaterials["wood1"].get();
	rWallWalkWay->Geo = mGeometries["walk3Geo"].get();
	rWallWalkWay->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	rWallWalkWay->IndexCount = rWallWalkWay->Geo->DrawArgs["walk3"].IndexCount;
	rWallWalkWay->StartIndexLocation = rWallWalkWay->Geo->DrawArgs["walk3"].StartIndexLocation;
	rWallWalkWay->BaseVertexLocation = rWallWalkWay->Geo->DrawArgs["walk3"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(rWallWalkWay.get());

	auto bWallWalkWay = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&bWallWalkWay->World, XMMatrixTranslation(0.0f, 11.0f, 54.0f));
	bWallWalkWay->ObjCBIndex =65;
	bWallWalkWay->Mat = mMaterials["wood1"].get();
	bWallWalkWay->Geo = mGeometries["walk2Geo"].get();
	bWallWalkWay->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bWallWalkWay->IndexCount = bWallWalkWay->Geo->DrawArgs["walk2"].IndexCount;
	bWallWalkWay->StartIndexLocation = bWallWalkWay->Geo->DrawArgs["walk2"].StartIndexLocation;
	bWallWalkWay->BaseVertexLocation = bWallWalkWay->Geo->DrawArgs["walk2"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(bWallWalkWay.get());

	auto crate2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&crate2->World, XMMatrixTranslation(85.0f, 5.0f, 80.0f) + XMMatrixScaling(0.1f, 0.1f, 0.1f));
	crate2->ObjCBIndex = 66;
	crate2->Mat = mMaterials["crate"].get();
	crate2->Geo = mGeometries["boxGeo"].get();
	crate2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	crate2->IndexCount = crate2->Geo->DrawArgs["box"].IndexCount;
	crate2->StartIndexLocation = crate2->Geo->DrawArgs["box"].StartIndexLocation;
	crate2->BaseVertexLocation = crate2->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(crate2.get());

	auto backWallLedge = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&backWallLedge->World, XMMatrixTranslation(0.0f, 11.2f, 56.48f));
	backWallLedge->ObjCBIndex = 67;
	backWallLedge->Mat = mMaterials["stone1"].get();
	backWallLedge->Geo = mGeometries["backLedgeGeo"].get();
	backWallLedge->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	backWallLedge->IndexCount = backWallLedge->Geo->DrawArgs["backLedge"].IndexCount;
	backWallLedge->StartIndexLocation = backWallLedge->Geo->DrawArgs["backLedge"].StartIndexLocation;
	backWallLedge->BaseVertexLocation = backWallLedge->Geo->DrawArgs["backLedge"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(backWallLedge.get());

	auto rightWallLedge = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&rightWallLedge->World, XMMatrixTranslation(56.48f, 11.2f, 0.0f));
	rightWallLedge->ObjCBIndex = 68;
	rightWallLedge->Mat = mMaterials["stone1"].get();
	rightWallLedge->Geo = mGeometries["rightLedgeGeo"].get();
	rightWallLedge->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	rightWallLedge->IndexCount = rightWallLedge->Geo->DrawArgs["rightLedge"].IndexCount;
	rightWallLedge->StartIndexLocation = rightWallLedge->Geo->DrawArgs["rightLedge"].StartIndexLocation;
	rightWallLedge->BaseVertexLocation = rightWallLedge->Geo->DrawArgs["rightLedge"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(rightWallLedge.get());

	auto leftWallLedge = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&leftWallLedge->World, XMMatrixTranslation(-56.48f, 11.2f, 0.0f));
	leftWallLedge->ObjCBIndex = 69;
	leftWallLedge->Mat = mMaterials["stone1"].get();
	leftWallLedge->Geo = mGeometries["leftLedgeGeo"].get();
	leftWallLedge->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	leftWallLedge->IndexCount = leftWallLedge->Geo->DrawArgs["leftLedge"].IndexCount;
	leftWallLedge->StartIndexLocation = leftWallLedge->Geo->DrawArgs["leftLedge"].StartIndexLocation;
	leftWallLedge->BaseVertexLocation = leftWallLedge->Geo->DrawArgs["leftLedge"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(leftWallLedge.get());

	// DRAW BRIDGE STUFF
	auto drawBridge = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&drawBridge->World, XMMatrixRotationX(-1) + XMMatrixTranslation(0.0f, 12.0f, -130.0f));
	drawBridge->ObjCBIndex = 70;
	drawBridge->Mat = mMaterials["wood2"].get();
	drawBridge->Geo = mGeometries["drawBridgeGeo"].get();
	drawBridge->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	drawBridge->IndexCount = drawBridge->Geo->DrawArgs["drawBridge"].IndexCount;
	drawBridge->StartIndexLocation = drawBridge->Geo->DrawArgs["drawBridge"].StartIndexLocation;
	drawBridge->BaseVertexLocation = drawBridge->Geo->DrawArgs["drawBridge"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(drawBridge.get());

	auto rDrawBridgePeg = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&rDrawBridgePeg->World, XMMatrixRotationX(0.8) * XMMatrixScaling(0.1f, 0.3f, 0.1f) * XMMatrixTranslation(5.0f, 10.0f, -66.5f));
	rDrawBridgePeg->ObjCBIndex = 71;
	rDrawBridgePeg->Mat = mMaterials["stone1"].get();
	rDrawBridgePeg->Geo = mGeometries["boxGeo"].get();
	rDrawBridgePeg->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	rDrawBridgePeg->IndexCount = rDrawBridgePeg->Geo->DrawArgs["box"].IndexCount;
	rDrawBridgePeg->StartIndexLocation = rDrawBridgePeg->Geo->DrawArgs["box"].StartIndexLocation;
	rDrawBridgePeg->BaseVertexLocation = rDrawBridgePeg->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(rDrawBridgePeg.get());

	auto lDrawBridgePeg = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lDrawBridgePeg->World, XMMatrixRotationX(0.8)* XMMatrixScaling(0.1f, 0.3f, 0.1f)* XMMatrixTranslation(-5.0f, 10.0f, -66.5f));
	lDrawBridgePeg->ObjCBIndex = 72;
	lDrawBridgePeg->Mat = mMaterials["stone1"].get();
	lDrawBridgePeg->Geo = mGeometries["boxGeo"].get();
	lDrawBridgePeg->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lDrawBridgePeg->IndexCount = lDrawBridgePeg->Geo->DrawArgs["box"].IndexCount;
	lDrawBridgePeg->StartIndexLocation = lDrawBridgePeg->Geo->DrawArgs["box"].StartIndexLocation;
	lDrawBridgePeg->BaseVertexLocation = lDrawBridgePeg->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(lDrawBridgePeg.get());

	auto riverWall1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&riverWall1->World, XMMatrixTranslation(0.0f, -0.51f, -67.0f) * XMMatrixScaling(1.75f, 4.0f, 1.0f));
	riverWall1->ObjCBIndex = 73;
	riverWall1->Mat = mMaterials["dirtGrass"].get();
	riverWall1->Geo = mGeometries["backLedgeGeo"].get();
	riverWall1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	riverWall1->IndexCount = riverWall1->Geo->DrawArgs["backLedge"].IndexCount;
	riverWall1->StartIndexLocation = riverWall1->Geo->DrawArgs["backLedge"].StartIndexLocation;
	riverWall1->BaseVertexLocation = riverWall1->Geo->DrawArgs["backLedge"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(riverWall1.get());

	auto riverWall2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&riverWall2->World, XMMatrixTranslation(0.0f, -0.51f, -77.0f)* XMMatrixScaling(1.75f, 4.0f, 1.0f));
	riverWall2->ObjCBIndex = 74;
	riverWall2->Mat = mMaterials["dirtGrass"].get();
	riverWall2->Geo = mGeometries["backLedgeGeo"].get();
	riverWall2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	riverWall2->IndexCount = riverWall2->Geo->DrawArgs["backLedge"].IndexCount;
	riverWall2->StartIndexLocation = riverWall2->Geo->DrawArgs["backLedge"].StartIndexLocation;
	riverWall2->BaseVertexLocation = riverWall2->Geo->DrawArgs["backLedge"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(riverWall2.get());

	auto avenue = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&avenue->World, XMMatrixTranslation(0.0f, -0.50f, -158.0f) + XMMatrixScaling(2.5f, 0.0f, 0.0f));
	avenue->ObjCBIndex = 75;
	avenue->Mat = mMaterials["grass"].get();
	avenue->Geo = mGeometries["avenueGeo"].get();
	avenue->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	avenue->IndexCount = avenue->Geo->DrawArgs["avenue"].IndexCount;
	avenue->StartIndexLocation = avenue->Geo->DrawArgs["avenue"].StartIndexLocation;
	avenue->BaseVertexLocation = avenue->Geo->DrawArgs["avenue"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(avenue.get());

	float linkLength = 8.0f;
	float linkSpacing = 4.0f;
	int numberOfLinks = 15;

	// Right Gate Chain
	for (int i = 0; i < numberOfLinks; ++i)
	{
		auto torus = std::make_unique<RenderItem>();

		XMMATRIX rotation;
		if (i % 2 == 0)
		{
			rotation = XMMatrixRotationX(XM_PI / 1.0);
		}
		else
		{
			rotation = XMMatrixRotationZ(XM_PI / 1.5);
		}

		XMStoreFloat4x4(&torus->World, rotation + XMMatrixTranslation(100.0f, 215.0f, -1325 + (i * (linkLength + linkSpacing))) * XMMatrixScaling(0.1f, 0.1f, 0.1f));
		torus->ObjCBIndex = 76 + i;
		torus->Mat = mMaterials["rMetal"].get();
		torus->Geo = mGeometries["torusGeo"].get();
		torus->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		torus->IndexCount = torus->Geo->DrawArgs["torus"].IndexCount;
		torus->StartIndexLocation = torus->Geo->DrawArgs["torus"].StartIndexLocation;
		torus->BaseVertexLocation = torus->Geo->DrawArgs["torus"].BaseVertexLocation;
		mRitemLayer[(int)RenderLayer::AlphaTested].push_back(torus.get());
		mAllRitems.push_back(std::move(torus));
	}
	// Left Gate Chain
	for (int i = 0; i < numberOfLinks; ++i)
	{
		auto torus = std::make_unique<RenderItem>();

		XMMATRIX rotation;
		if (i % 2 == 0)
		{
			rotation = XMMatrixRotationX(XM_PI / 1.0);
		}
		else
		{
			rotation = XMMatrixRotationZ(XM_PI / 1.5);
		}

		XMStoreFloat4x4(&torus->World, rotation + XMMatrixTranslation(-100.0f, 215.0f, -1325 + (i * (linkLength + linkSpacing))) * XMMatrixScaling(0.1f, 0.1f, 0.1f));
		torus->ObjCBIndex = 91 + i;
		torus->Mat = mMaterials["rMetal"].get();
		torus->Geo = mGeometries["torusGeo"].get();
		torus->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		torus->IndexCount = torus->Geo->DrawArgs["torus"].IndexCount;
		torus->StartIndexLocation = torus->Geo->DrawArgs["torus"].StartIndexLocation;
		torus->BaseVertexLocation = torus->Geo->DrawArgs["torus"].BaseVertexLocation;
		mRitemLayer[(int)RenderLayer::AlphaTested].push_back(torus.get());
		mAllRitems.push_back(std::move(torus));
	}

	// MAZE

	// Outer Walls
	auto mOuterWall1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mOuterWall1->World, XMMatrixScaling(8.0f, 1.6f, 0.5f) * XMMatrixTranslation(35.0f, 6.0f, -90.0f));
	XMStoreFloat4x4(&mOuterWall1->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mOuterWall1->ObjCBIndex = 106;
	mOuterWall1->Mat = mMaterials["hedge"].get();
	mOuterWall1->Geo = mGeometries["boxGeo"].get();
	mOuterWall1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mOuterWall1->IndexCount = mOuterWall1->Geo->DrawArgs["box"].IndexCount;
	mOuterWall1->StartIndexLocation = mOuterWall1->Geo->DrawArgs["box"].StartIndexLocation;
	mOuterWall1->BaseVertexLocation = mOuterWall1->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mOuterWall1.get());

	auto mOuterWall2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mOuterWall2->World, XMMatrixScaling(8.0f, 1.6f, 0.5f) * XMMatrixTranslation(-35.0f, 6.0f, -90.0f));
	XMStoreFloat4x4(&mOuterWall2->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mOuterWall2->ObjCBIndex = 107;
	mOuterWall2->Mat = mMaterials["hedge"].get();
	mOuterWall2->Geo = mGeometries["boxGeo"].get();
	mOuterWall2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mOuterWall2->IndexCount = mOuterWall2->Geo->DrawArgs["box"].IndexCount;
	mOuterWall2->StartIndexLocation = mOuterWall2->Geo->DrawArgs["box"].StartIndexLocation;
	mOuterWall2->BaseVertexLocation = mOuterWall2->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mOuterWall2.get());

	auto mOuterWall3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mOuterWall3->World, XMMatrixScaling(0.5f, 1.6f, 8.0f) * XMMatrixTranslation(-69.0f, 6.0f, -120.0f));
	XMStoreFloat4x4(&mOuterWall3->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mOuterWall3->ObjCBIndex = 108;
	mOuterWall3->Mat = mMaterials["hedge"].get();
	mOuterWall3->Geo = mGeometries["boxGeo"].get();
	mOuterWall3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mOuterWall3->IndexCount = mOuterWall3->Geo->DrawArgs["box"].IndexCount;
	mOuterWall3->StartIndexLocation = mOuterWall3->Geo->DrawArgs["box"].StartIndexLocation;
	mOuterWall3->BaseVertexLocation = mOuterWall3->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mOuterWall3.get());

	auto mOuterWall4 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mOuterWall4->World, XMMatrixScaling(0.5f, 1.6f, 10.0f) * XMMatrixTranslation(-69.0f, 6.0f, -192.0f));
	XMStoreFloat4x4(&mOuterWall4->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mOuterWall4->ObjCBIndex = 109;
	mOuterWall4->Mat = mMaterials["hedge"].get();
	mOuterWall4->Geo = mGeometries["boxGeo"].get();
	mOuterWall4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mOuterWall4->IndexCount = mOuterWall4->Geo->DrawArgs["box"].IndexCount;
	mOuterWall4->StartIndexLocation = mOuterWall4->Geo->DrawArgs["box"].StartIndexLocation;
	mOuterWall4->BaseVertexLocation = mOuterWall4->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mOuterWall4.get());

	auto mOuterWall5 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mOuterWall5->World, XMMatrixScaling(0.5f, 1.6f, 8.0f) * XMMatrixTranslation(69.0f, 6.0f, -120.0f));
	XMStoreFloat4x4(&mOuterWall5->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mOuterWall5->ObjCBIndex = 110;
	mOuterWall5->Mat = mMaterials["hedge"].get();
	mOuterWall5->Geo = mGeometries["boxGeo"].get();
	mOuterWall5->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mOuterWall5->IndexCount = mOuterWall5->Geo->DrawArgs["box"].IndexCount;
	mOuterWall5->StartIndexLocation = mOuterWall5->Geo->DrawArgs["box"].StartIndexLocation;
	mOuterWall5->BaseVertexLocation = mOuterWall5->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mOuterWall5.get());

	auto mOuterWall6 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mOuterWall6->World, XMMatrixScaling(0.5f, 1.6f, 10.0f) * XMMatrixTranslation(69.0f, 6.0f, -192.0f));
	XMStoreFloat4x4(&mOuterWall6->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mOuterWall6->ObjCBIndex = 111;
	mOuterWall6->Mat = mMaterials["hedge"].get();
	mOuterWall6->Geo = mGeometries["boxGeo"].get();
	mOuterWall6->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mOuterWall6->IndexCount = mOuterWall6->Geo->DrawArgs["box"].IndexCount;
	mOuterWall6->StartIndexLocation = mOuterWall6->Geo->DrawArgs["box"].StartIndexLocation;
	mOuterWall6->BaseVertexLocation = mOuterWall6->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mOuterWall6.get());

	auto mOuterWall7 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mOuterWall7->World, XMMatrixScaling(8.3f, 1.6f, 0.5f)* XMMatrixTranslation(34.0f, 6.0f, -230.0f));
	XMStoreFloat4x4(&mOuterWall7->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mOuterWall7->ObjCBIndex = 112;
	mOuterWall7->Mat = mMaterials["hedge"].get();
	mOuterWall7->Geo = mGeometries["boxGeo"].get();
	mOuterWall7->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mOuterWall7->IndexCount = mOuterWall7->Geo->DrawArgs["box"].IndexCount;
	mOuterWall7->StartIndexLocation = mOuterWall7->Geo->DrawArgs["box"].StartIndexLocation;
	mOuterWall7->BaseVertexLocation = mOuterWall7->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mOuterWall7.get());

	auto mOuterWall8 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mOuterWall8->World, XMMatrixScaling(8.3f, 1.6f, 0.5f)* XMMatrixTranslation(-34.0f, 6.0f, -230.0f));
	XMStoreFloat4x4(&mOuterWall8->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mOuterWall8->ObjCBIndex = 113;
	mOuterWall8->Mat = mMaterials["hedge"].get();
	mOuterWall8->Geo = mGeometries["boxGeo"].get();
	mOuterWall8->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mOuterWall8->IndexCount = mOuterWall8->Geo->DrawArgs["box"].IndexCount;
	mOuterWall8->StartIndexLocation = mOuterWall8->Geo->DrawArgs["box"].StartIndexLocation;
	mOuterWall8->BaseVertexLocation = mOuterWall8->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mOuterWall8.get());

	//Inner Walls

	// Section 1
	auto mInnerWall1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall1->World, XMMatrixScaling(0.5f, 1.6f, 3.0f)* XMMatrixTranslation(40.0f, 6.0f, -209.0f));
	XMStoreFloat4x4(&mInnerWall1->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall1->ObjCBIndex = 114;
	mInnerWall1->Mat = mMaterials["hedge"].get();
	mInnerWall1->Geo = mGeometries["boxGeo"].get();
	mInnerWall1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall1->IndexCount = mInnerWall1->Geo->DrawArgs["box"].IndexCount;
	mInnerWall1->StartIndexLocation = mInnerWall1->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall1->BaseVertexLocation = mInnerWall1->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall1.get());

	auto mInnerWall2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall2->World, XMMatrixScaling(0.5f, 1.6f, 3.0f)* XMMatrixTranslation(28.0f, 6.0f, -216.0f));
	XMStoreFloat4x4(&mInnerWall2->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall2->ObjCBIndex = 115;
	mInnerWall2->Mat = mMaterials["hedge"].get();
	mInnerWall2->Geo = mGeometries["boxGeo"].get();
	mInnerWall2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall2->IndexCount = mInnerWall2->Geo->DrawArgs["box"].IndexCount;
	mInnerWall2->StartIndexLocation = mInnerWall2->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall2->BaseVertexLocation = mInnerWall2->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall2.get());

	auto mInnerWall3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall3->World, XMMatrixScaling(2.0f, 1.6f, 0.5f)* XMMatrixTranslation(50.0f, 6.0f, -210.0f));
	XMStoreFloat4x4(&mInnerWall3->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall3->ObjCBIndex = 116;
	mInnerWall3->Mat = mMaterials["hedge"].get();
	mInnerWall3->Geo = mGeometries["boxGeo"].get();
	mInnerWall3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall3->IndexCount = mInnerWall3->Geo->DrawArgs["box"].IndexCount;
	mInnerWall3->StartIndexLocation = mInnerWall3->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall3->BaseVertexLocation = mInnerWall3->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall3.get());

	auto mInnerWall4 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall4->World, XMMatrixScaling(3.0f, 1.6f, 0.5f)* XMMatrixTranslation(-55.0f, 6.0f, -210.0f));
	XMStoreFloat4x4(&mInnerWall4->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall4->ObjCBIndex = 117;
	mInnerWall4->Mat = mMaterials["hedge"].get();
	mInnerWall4->Geo = mGeometries["boxGeo"].get();
	mInnerWall4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall4->IndexCount = mInnerWall4->Geo->DrawArgs["box"].IndexCount;
	mInnerWall4->StartIndexLocation = mInnerWall4->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall4->BaseVertexLocation = mInnerWall4->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall4.get());

	auto mInnerWall5 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall5->World, XMMatrixScaling(0.5f, 1.6f, 3.0f)* XMMatrixTranslation(-28.0f, 6.0f, -216.0f));
	XMStoreFloat4x4(&mInnerWall5->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall5->ObjCBIndex = 118;
	mInnerWall5->Mat = mMaterials["hedge"].get();
	mInnerWall5->Geo = mGeometries["boxGeo"].get();
	mInnerWall5->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall5->IndexCount = mInnerWall5->Geo->DrawArgs["box"].IndexCount;
	mInnerWall5->StartIndexLocation = mInnerWall5->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall5->BaseVertexLocation = mInnerWall5->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall5.get());

	auto mInnerWall6 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall6->World, XMMatrixScaling(0.5f, 1.6f, 3.0f)* XMMatrixTranslation(-10.0f, 6.0f, -209.0f));
	XMStoreFloat4x4(&mInnerWall6->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall6->ObjCBIndex = 119;
	mInnerWall6->Mat = mMaterials["hedge"].get();
	mInnerWall6->Geo = mGeometries["boxGeo"].get();
	mInnerWall6->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall6->IndexCount = mInnerWall6->Geo->DrawArgs["box"].IndexCount;
	mInnerWall6->StartIndexLocation = mInnerWall6->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall6->BaseVertexLocation = mInnerWall6->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall6.get());	

	// Sectin 1 End


	// Section 2 Start

	auto mInnerWall7 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall7->World, XMMatrixScaling(0.5f, 1.6f, 3.0f)* XMMatrixTranslation(40.0f, 6.0f, -175.0f));
	XMStoreFloat4x4(&mInnerWall7->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall7->ObjCBIndex = 120;
	mInnerWall7->Mat = mMaterials["hedge"].get();
	mInnerWall7->Geo = mGeometries["boxGeo"].get();
	mInnerWall7->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall7->IndexCount = mInnerWall7->Geo->DrawArgs["box"].IndexCount;
	mInnerWall7->StartIndexLocation = mInnerWall7->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall7->BaseVertexLocation = mInnerWall7->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall7.get());

	auto mInnerWall8 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall8->World, XMMatrixScaling(0.5f, 1.6f, 3.0f)* XMMatrixTranslation(28.0f, 6.0f, -180.0f));
	XMStoreFloat4x4(&mInnerWall8->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall8->ObjCBIndex = 121;
	mInnerWall8->Mat = mMaterials["hedge"].get();
	mInnerWall8->Geo = mGeometries["boxGeo"].get();
	mInnerWall8->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall8->IndexCount = mInnerWall8->Geo->DrawArgs["box"].IndexCount;
	mInnerWall8->StartIndexLocation = mInnerWall8->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall8->BaseVertexLocation = mInnerWall8->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall8.get());

	auto mInnerWall9 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall9->World, XMMatrixScaling(2.0f, 1.6f, 0.5f)* XMMatrixTranslation(50.0f, 6.0f, -175.0f));
	XMStoreFloat4x4(&mInnerWall9->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall9->ObjCBIndex = 122;
	mInnerWall9->Mat = mMaterials["hedge"].get();
	mInnerWall9->Geo = mGeometries["boxGeo"].get();
	mInnerWall9->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall9->IndexCount = mInnerWall9->Geo->DrawArgs["box"].IndexCount;
	mInnerWall9->StartIndexLocation = mInnerWall9->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall9->BaseVertexLocation = mInnerWall9->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall9.get());

	auto mInnerWall10 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall10->World, XMMatrixScaling(3.0f, 1.6f, 0.5f)* XMMatrixTranslation(-55.0f, 6.0f, -175.0f));
	XMStoreFloat4x4(&mInnerWall10->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall10->ObjCBIndex = 123;
	mInnerWall10->Mat = mMaterials["hedge"].get();
	mInnerWall10->Geo = mGeometries["boxGeo"].get();
	mInnerWall10->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall10->IndexCount = mInnerWall10->Geo->DrawArgs["box"].IndexCount;
	mInnerWall10->StartIndexLocation = mInnerWall10->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall10->BaseVertexLocation = mInnerWall10->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall10.get());

	auto mInnerWall11 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall11->World, XMMatrixScaling(0.5f, 1.6f, 3.0f)* XMMatrixTranslation(-28.0f, 6.0f, -180.0f));
	XMStoreFloat4x4(&mInnerWall11->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall11->ObjCBIndex = 124;
	mInnerWall11->Mat = mMaterials["hedge"].get();
	mInnerWall11->Geo = mGeometries["boxGeo"].get();
	mInnerWall11->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall11->IndexCount = mInnerWall11->Geo->DrawArgs["box"].IndexCount;
	mInnerWall11->StartIndexLocation = mInnerWall11->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall11->BaseVertexLocation = mInnerWall11->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall11.get());

	auto mInnerWall12 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall12->World, XMMatrixScaling(0.5f, 1.6f, 3.0f)* XMMatrixTranslation(-10.0f, 6.0f, -175.0f));
	XMStoreFloat4x4(&mInnerWall12->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall12->ObjCBIndex = 125;
	mInnerWall12->Mat = mMaterials["hedge"].get();
	mInnerWall12->Geo = mGeometries["boxGeo"].get();
	mInnerWall12->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall12->IndexCount = mInnerWall12->Geo->DrawArgs["box"].IndexCount;
	mInnerWall12->StartIndexLocation = mInnerWall12->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall12->BaseVertexLocation = mInnerWall12->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall12.get());

	// Section 2 End

	// Section 3 Start

	// EMPTY

	// Section 3 End

	// Section 4 Start

	auto mInnerWall13 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall13->World, XMMatrixScaling(8.0f, 1.6f, .25f)* XMMatrixTranslation(30.0f, 6.0f, -110.0f));
	XMStoreFloat4x4(&mInnerWall13->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall13->ObjCBIndex = 126;
	mInnerWall13->Mat = mMaterials["hedge"].get();
	mInnerWall13->Geo = mGeometries["boxGeo"].get();
	mInnerWall13->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall13->IndexCount = mInnerWall13->Geo->DrawArgs["box"].IndexCount;
	mInnerWall13->StartIndexLocation = mInnerWall13->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall13->BaseVertexLocation = mInnerWall13->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall13.get());

	auto mInnerWall14 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall14->World, XMMatrixScaling(8.0f, 1.6f, .25f)* XMMatrixTranslation(-30.0f, 6.0f, -100.0f));
	XMStoreFloat4x4(&mInnerWall14->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall14->ObjCBIndex = 127;
	mInnerWall14->Mat = mMaterials["hedge"].get();
	mInnerWall14->Geo = mGeometries["boxGeo"].get();
	mInnerWall14->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall14->IndexCount = mInnerWall14->Geo->DrawArgs["box"].IndexCount;
	mInnerWall14->StartIndexLocation = mInnerWall14->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall14->BaseVertexLocation = mInnerWall14->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall14.get());

	auto mInnerWall15 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall15->World, XMMatrixScaling(0.25f, 1.6f, 1.0f)* XMMatrixTranslation(4.0f, 6.0f, -95.0f));
	XMStoreFloat4x4(&mInnerWall15->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall15->ObjCBIndex = 128;
	mInnerWall15->Mat = mMaterials["hedge"].get();
	mInnerWall15->Geo = mGeometries["boxGeo"].get();
	mInnerWall15->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall15->IndexCount = mInnerWall15->Geo->DrawArgs["box"].IndexCount;
	mInnerWall15->StartIndexLocation = mInnerWall15->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall15->BaseVertexLocation = mInnerWall15->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall15.get());

	auto mInnerWall16 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerWall16->World, XMMatrixScaling(0.25f, 1.6f, 1.0f)* XMMatrixTranslation(-4.0f, 6.0f, -115.0f));
	XMStoreFloat4x4(&mInnerWall16->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerWall16->ObjCBIndex = 129;
	mInnerWall16->Mat = mMaterials["hedge"].get();
	mInnerWall16->Geo = mGeometries["boxGeo"].get();
	mInnerWall16->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerWall16->IndexCount = mInnerWall16->Geo->DrawArgs["box"].IndexCount;
	mInnerWall16->StartIndexLocation = mInnerWall16->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerWall16->BaseVertexLocation = mInnerWall16->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerWall16.get());


	// Section 4 End

	// Dividers

	// First/Second Section Divider
	auto mInnerDivider1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerDivider1->World, XMMatrixScaling(16.0f, 1.6f, 0.5f)* XMMatrixTranslation(3.0f, 6.0f, -195.0f));
	XMStoreFloat4x4(&mInnerDivider1->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerDivider1->ObjCBIndex = 130;
	mInnerDivider1->Mat = mMaterials["hedge"].get();
	mInnerDivider1->Geo = mGeometries["boxGeo"].get();
	mInnerDivider1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerDivider1->IndexCount = mInnerDivider1->Geo->DrawArgs["box"].IndexCount;
	mInnerDivider1->StartIndexLocation = mInnerDivider1->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerDivider1->BaseVertexLocation = mInnerDivider1->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerDivider1.get());

	// Second/Third Section Divider

	auto mInnerDivider2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerDivider2->World, XMMatrixScaling(16.0f, 1.6f, 0.5f)* XMMatrixTranslation(-3.0f, 6.0f, -160.0f));
	XMStoreFloat4x4(&mInnerDivider2->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerDivider2->ObjCBIndex = 131;
	mInnerDivider2->Mat = mMaterials["hedge"].get();
	mInnerDivider2->Geo = mGeometries["boxGeo"].get();
	mInnerDivider2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerDivider2->IndexCount = mInnerDivider2->Geo->DrawArgs["box"].IndexCount;
	mInnerDivider2->StartIndexLocation = mInnerDivider2->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerDivider2->BaseVertexLocation = mInnerDivider2->Geo->DrawArgs["box"].BaseVertexLocation;
	XMStoreFloat4x4(&mInnerDivider1->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerDivider2.get());

	// 3.5 Section Divider

	auto mInnerDivider3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerDivider3->World, XMMatrixScaling(16.0f, 1.6f, 0.5f)* XMMatrixTranslation(3.0f, 6.0f, -140.0f));
	XMStoreFloat4x4(&mInnerDivider3->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerDivider3->ObjCBIndex = 132;
	mInnerDivider3->Mat = mMaterials["hedge"].get();
	mInnerDivider3->Geo = mGeometries["boxGeo"].get();
	mInnerDivider3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerDivider3->IndexCount = mInnerDivider3->Geo->DrawArgs["box"].IndexCount;
	mInnerDivider3->StartIndexLocation = mInnerDivider3->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerDivider3->BaseVertexLocation = mInnerDivider3->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerDivider3.get());

	// Secion 4 Dividers

	auto mInnerDivider4 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerDivider4->World, XMMatrixScaling(8.0f, 1.6f, 0.5f)* XMMatrixTranslation(35.0f, 6.0f, -120.0f));
	XMStoreFloat4x4(&mInnerDivider4->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerDivider4->ObjCBIndex = 133;
	mInnerDivider4->Mat = mMaterials["hedge"].get();
	mInnerDivider4->Geo = mGeometries["boxGeo"].get();
	mInnerDivider4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerDivider4->IndexCount = mInnerDivider4->Geo->DrawArgs["box"].IndexCount;
	mInnerDivider4->StartIndexLocation = mInnerDivider4->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerDivider4->BaseVertexLocation = mInnerDivider4->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerDivider4.get());

	auto mInnerDivider5 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&mInnerDivider5->World, XMMatrixScaling(8.0f, 1.6f, 0.5f)* XMMatrixTranslation(-35.0f, 6.0f, -120.0f));
	XMStoreFloat4x4(&mInnerDivider5->TexTransform, XMMatrixScaling(15.0f, 11.0f, 1.0f));
	mInnerDivider5->ObjCBIndex = 134;
	mInnerDivider5->Mat = mMaterials["hedge"].get();
	mInnerDivider5->Geo = mGeometries["boxGeo"].get();
	mInnerDivider5->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mInnerDivider5->IndexCount = mInnerDivider5->Geo->DrawArgs["box"].IndexCount;
	mInnerDivider5->StartIndexLocation = mInnerDivider5->Geo->DrawArgs["box"].StartIndexLocation;
	mInnerDivider5->BaseVertexLocation = mInnerDivider5->Geo->DrawArgs["box"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(mInnerDivider5.get());

	// Trees
	auto treeSpritesRitem = std::make_unique<RenderItem>();
	treeSpritesRitem->World = MathHelper::Identity4x4();
	treeSpritesRitem->ObjCBIndex = 135;
	treeSpritesRitem->Mat = mMaterials["treeSprites"].get();
	treeSpritesRitem->Geo = mGeometries["treeSpritesGeo"].get();
	//step2
	treeSpritesRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	treeSpritesRitem->IndexCount = treeSpritesRitem->Geo->DrawArgs["points"].IndexCount;
	treeSpritesRitem->StartIndexLocation = treeSpritesRitem->Geo->DrawArgs["points"].StartIndexLocation;
	treeSpritesRitem->BaseVertexLocation = treeSpritesRitem->Geo->DrawArgs["points"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::AlphaTestedTreeSprites].push_back(treeSpritesRitem.get());

    mAllRitems.push_back(std::move(wavesRitem));
    mAllRitems.push_back(std::move(gridRitem));
	mAllRitems.push_back(std::move(Gate));
	mAllRitems.push_back(std::move(crate1));
	mAllRitems.push_back(std::move(bWall1));
	mAllRitems.push_back(std::move(bWall2));
	mAllRitems.push_back(std::move(bWall3));
	mAllRitems.push_back(std::move(bWall4));
	mAllRitems.push_back(std::move(bWall5));
	mAllRitems.push_back(std::move(fWall1));
	mAllRitems.push_back(std::move(fWall2));
	mAllRitems.push_back(std::move(fWall3));
	mAllRitems.push_back(std::move(fWall4));
	mAllRitems.push_back(std::move(fWallLedge1));
	mAllRitems.push_back(std::move(fWallLedge2));
	mAllRitems.push_back(std::move(rWall1));
	mAllRitems.push_back(std::move(rWall2));
	mAllRitems.push_back(std::move(rWall3));
	mAllRitems.push_back(std::move(rWall4));
	mAllRitems.push_back(std::move(rWall5));
	mAllRitems.push_back(std::move(lWall1));
	mAllRitems.push_back(std::move(lWall2));
	mAllRitems.push_back(std::move(lWall3));
	mAllRitems.push_back(std::move(lWall4));
	mAllRitems.push_back(std::move(lWall5));
	mAllRitems.push_back(std::move(fRTower));
	mAllRitems.push_back(std::move(fLTower));
	mAllRitems.push_back(std::move(bRTower));
	mAllRitems.push_back(std::move(bLTower));
	mAllRitems.push_back(std::move(fRCone));
	mAllRitems.push_back(std::move(fLCone));
	mAllRitems.push_back(std::move(bRCone));
	mAllRitems.push_back(std::move(bLCone));
	mAllRitems.push_back(std::move(lGateWall));
	mAllRitems.push_back(std::move(rGateWall));
	mAllRitems.push_back(std::move(gateHouseRoof));
	mAllRitems.push_back(std::move(gateHouseFloor));
	mAllRitems.push_back(std::move(gateHouseWalkWay));
	mAllRitems.push_back(std::move(fgateHouseLedgeMid));
	mAllRitems.push_back(std::move(fgateHouseLedgeLeft));
	mAllRitems.push_back(std::move(fgateHouseLedgeRight));
	mAllRitems.push_back(std::move(fgateHouseLedge2));
	mAllRitems.push_back(std::move(fgateHouseLedge3));
	mAllRitems.push_back(std::move(bgateHouseLedgeMid));
	mAllRitems.push_back(std::move(bgateHouseLedgeLeft));
	mAllRitems.push_back(std::move(bgateHouseLedgeRight));
	mAllRitems.push_back(std::move(fRWallWalk));
	mAllRitems.push_back(std::move(fRWallWalk2));
	mAllRitems.push_back(std::move(fLWallWalk));
	mAllRitems.push_back(std::move(fLWallWalk2));
	mAllRitems.push_back(std::move(lWallWalkWay));
	mAllRitems.push_back(std::move(rWallWalkWay));
	mAllRitems.push_back(std::move(bWallWalkWay));
	mAllRitems.push_back(std::move(crate2));
	mAllRitems.push_back(std::move(backWallLedge));
	mAllRitems.push_back(std::move(rightWallLedge));
	mAllRitems.push_back(std::move(leftWallLedge));
	mAllRitems.push_back(std::move(drawBridge));
	mAllRitems.push_back(std::move(rDrawBridgePeg));
	mAllRitems.push_back(std::move(lDrawBridgePeg));
	mAllRitems.push_back(std::move(riverWall1));
	mAllRitems.push_back(std::move(riverWall2));
	mAllRitems.push_back(std::move(avenue));
	mAllRitems.push_back(std::move(mOuterWall1));
	mAllRitems.push_back(std::move(mOuterWall2));
	mAllRitems.push_back(std::move(mOuterWall3));
	mAllRitems.push_back(std::move(mOuterWall4));	
	mAllRitems.push_back(std::move(mOuterWall5));
	mAllRitems.push_back(std::move(mOuterWall6));
	mAllRitems.push_back(std::move(mOuterWall7));
	mAllRitems.push_back(std::move(mOuterWall8));
	mAllRitems.push_back(std::move(mInnerWall1));
	mAllRitems.push_back(std::move(mInnerWall2));
	mAllRitems.push_back(std::move(mInnerWall3));
	mAllRitems.push_back(std::move(mInnerWall4));
	mAllRitems.push_back(std::move(mInnerWall5));
	mAllRitems.push_back(std::move(mInnerWall6));
	mAllRitems.push_back(std::move(mInnerWall7));
	mAllRitems.push_back(std::move(mInnerWall8));
	mAllRitems.push_back(std::move(mInnerWall9));
	mAllRitems.push_back(std::move(mInnerWall10));
	mAllRitems.push_back(std::move(mInnerWall11));
	mAllRitems.push_back(std::move(mInnerWall12));
	mAllRitems.push_back(std::move(mInnerWall13));
	mAllRitems.push_back(std::move(mInnerWall14));
	mAllRitems.push_back(std::move(mInnerWall15));
	mAllRitems.push_back(std::move(mInnerWall16));
	mAllRitems.push_back(std::move(mInnerDivider1));
	mAllRitems.push_back(std::move(mInnerDivider2));
	mAllRitems.push_back(std::move(mInnerDivider3));
	mAllRitems.push_back(std::move(mInnerDivider4));
	mAllRitems.push_back(std::move(mInnerDivider5));
	mAllRitems.push_back(std::move(treeSpritesRitem));
}

void Assignment3::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		if (ri->Geo == nullptr)
		{
			assert(false && "Nullptr found in render item geometry!");
			continue;
		}

		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Assignment3::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return { 
		pointWrap, pointClamp,
		linearWrap, linearClamp, 
		anisotropicWrap, anisotropicClamp };
}

float Assignment3::GetHillsHeight(float x, float z) const
{
	float h = 0.0f;
	if (z < -50.0f)
	{
		// Check if (x, z) is within the mote boundaries
		if (x > -90.0f && x < 90.0f && z > -57.0f && z < -43.0f)
		{
			// Check if (x, z) is within the flat section of the mote
			if (x > -90.0f && x < 90.0f && z > -44.0f && z < -43.0f)
			{
				h = -5.0f;
			}
			else // In the sloping sections of the mote
			{
				if (z > -57.0f && z < -54.0f) // Sloping down
				{
					h = -5.0f * (1.0f - (z + 54.0f) / 3.0f);
				}
				else if (z > -44.0f && z < -43.0f) // Sloping up
				{
					h = -5.0f * (1.0f - (46.0f - z) / 3.0f);
				}
			}
		}
	}
	else
	{
		h = 0.0f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
	}
	return h;
}



XMFLOAT3 Assignment3::GetHillsNormal(float x, float z)const
{
    // n = (-df/dx, 1, -df/dz)
    XMFLOAT3 n(
        -0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
        1.0f,
        -0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

    XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
    XMStoreFloat3(&n, unitNormal);

    return n;
}

bool Assignment3::CheckCollision(const XMFLOAT3& playerPos, float boxSize)
{
	bool isColliding = false;

	// Check for collision with each box shape in the scene
	for (const auto& item1 : mAllRitems)
	{
		for (const auto& item2 : mAllRitems)
		{
			if (item1 != item2 && item1->Bounds.Intersects(item2->Bounds))
			{
				// Handle collision between item1 and item2
			}
		}
	}

	return isColliding;
}