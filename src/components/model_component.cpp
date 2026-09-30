#include "model_component.hpp"
#include "components/transform_component.hpp"
#include "scene/entity.hpp"
#include "assets/types/model.hpp"
#include "rendering/render_commands.hpp"
#include "assets/types/texture_2d.hpp"
#include "components/paintable_component.hpp"
#include "core/logging.hpp"
#include "components/animator_component.hpp"

void ModelComponent::Render(const RenderView &view, RenderQueue &queue) {
    if (!this->model)
        return;

    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    PaintableComponent *paintable = this->GetOwner()->GetComponent<PaintableComponent>();

    const XMMATRIX worldMatrix = transform->GetRenderMatrix();

    const std::vector<XMFLOAT4X4> *skinMatrices = nullptr;
    if (this->model->IsSkinned()) {
        AnimatorComponent *animator = this->GetOwner()->GetComponent<AnimatorComponent>();
        if (animator && animator->GetSkeleton() == this->model->GetSkeleton() &&
            animator->GetSkinMatrices().size() == this->model->GetSkeleton()->GetSkinJointCount()
        ) {
            skinMatrices = &animator->GetSkinMatrices();
        }
    }

    uint32_t meshIndex = 0;
    const auto &meshes = this->model->GetMeshes();
    for (const auto &mesh : meshes) 
    {
        ID3D11ShaderResourceView *paintMaskSRV = nullptr;

        if (paintable)
            paintMaskSRV = paintable->GetMaskSRV(meshIndex);

        const auto &subMeshes = mesh->GetSubMeshes();
        for (const auto &subMesh : subMeshes)
        {
            GeometryCommand command{};

            command.vertexBuffer = mesh->GetVertexBuffer();
            command.indexBuffer = mesh->GetIndexBuffer();

            command.startIndex = subMesh.startIndex;
            command.indexCount = subMesh.numIndicies;

            command.tint = this->tint;

            const Material &material = this->model->GetMaterials()[subMesh.materialIndex];

            command.material = material;

            if (material.albedoTextureIndex != -1)
                command.albedoSRV = this->model->GetTexture(material.albedoTextureIndex)->GetShaderResourceView();

            if (material.normalTextureIndex != -1)
                command.normalSRV = this->model->GetTexture(material.normalTextureIndex)->GetShaderResourceView();

            if (material.metallicRoughnessTextureIndex != -1)
                command.metallicRoughnessSRV = this->model->GetTexture(material.metallicRoughnessTextureIndex)->GetShaderResourceView();

            command.paintMaskSRV = paintMaskSRV;

            XMStoreFloat4x4(&command.worldMatrix, worldMatrix);

            if (skinMatrices && mesh->IsSkinned()) {
                command.skinVertexBuffer = mesh->GetSkinVertexBuffer();
                command.skinMatrices = skinMatrices->data();
                command.skinMatrixCount = static_cast<uint32_t>(skinMatrices->size());
            }

            queue.Submit(command);
        }

        meshIndex++;
    }
}

void ModelComponent::SetModel(std::shared_ptr<Model> model)
{
    this->model = model;
}

bool ModelComponent::GetWorldBounds(DirectX::BoundingBox &outBounds)
{
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return false;

    if (!this->model)
        return false;
    
    DirectX::BoundingBox boundingBox = this->model->GetBoundingBox();
    
    boundingBox.Transform(outBounds, transform->GetRenderMatrix());

    return true;
}
