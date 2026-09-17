#include "model_component.hpp"
#include "components/transform_component.hpp"
#include "scene/entity.hpp"
#include "assets/types/model.hpp"
#include "rendering/render_commands.hpp"
#include "assets/types/texture_2d.hpp"

void ModelComponent::Render(const RenderView &view, RenderQueue &queue) {
    if (!this->model)
        return;

    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    const XMMATRIX worldMatrix = transform->GetRenderMatrix();

    const auto &meshes = this->model->GetMeshes();
    for (const auto &mesh : meshes) 
    {
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

            command.albedoSRV = this->model->GetTexture(material.colorTextureIndex)->GetShaderResourceView();
            command.normalSRV = this->model->GetTexture(material.normalTextureIndex)->GetShaderResourceView();

            XMStoreFloat4x4(&command.worldMatrix, worldMatrix);

            queue.Submit(command);
        }
    }
}

void ModelComponent::SetModel(std::shared_ptr<Model> model)
{
    this->model = model;
}