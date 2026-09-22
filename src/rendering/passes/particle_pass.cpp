#include "particle_pass.hpp"

#include "core/logging.hpp"
#include "rendering/render_utils.hpp"
#include "rendering/render_data.hpp"

void ParticlePass::RenderParticlePass(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	SharedResources &sharedResources,
	std::vector<RenderView> &views
)
{
	BeginAnnotation("Compute Pass");

	RenderView *view = RenderUtils::GetView(views, ViewType::PRIMARY);
	if (!view)
	{
		LogWarn("Could not find a primary view to render to");
		return;
	}

	for (const ParticleSystemCommand &command : view->queue.particleSystemCommand)
	{
		ParticleSystemData data = {};
		data.count = command.count;
		data.lifeTime = command.lifeTime;
		data.spawnRadius = command.spawnRadius;
		data.atlasSize = command.atlasSize;
		data.velocity = command.velocity;
		data.animationSpeed = command.animationSpeed;
		data.acceleration = command.acceleration;
		data.startScale = command.startScale;
		data.startTint = command.startTint;
		data.endScale = command.endScale;
		data.endTint = command.endTint;

		RenderUtils::UploadConstantBuffer(deviceContext, this->particleSystemCB, data);
	}

	EndAnnotation();
}

void ParticlePass::RenderDrawPass(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	SharedResources &sharedResources,
	ID3D11RenderTargetView *renderTargetView,
	std::vector<RenderView> &views
)
{
	BeginAnnotation("Draw Pass");

	EndAnnotation();
}

void ParticlePass::Render(
	ID3D11Device *device, 
	ID3D11DeviceContext *deviceContext, 
	SharedResources &sharedResources, 
	ID3D11RenderTargetView *renderTargetView, 
	std::vector<RenderView> &views
)
{
	this->RenderParticlePass(device, deviceContext, sharedResources, views);
	this->RenderDrawPass(device, deviceContext, sharedResources, renderTargetView, views);
}

bool ParticlePass::Initialize(ID3D11Device* device)
{
	LogInfo("Initializing Particle Pass...");
	LogIndent();

	if (!RenderUtils::CreateConstantBuffer(device, sizeof(ParticleSystemData), &this->particleSystemCB, "Particle System Data"))
		return false;

	LogUnindent();

	return true;
}

void ParticlePass::Shutdown()
{
	LogInfo("Shutting down Particle Pass");

	SafeRelease(this->particleSystemCB);
}