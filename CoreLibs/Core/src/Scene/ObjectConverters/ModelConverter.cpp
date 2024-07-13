#include "ModelConverter.h"
#include "../../Base.h"
namespace FooGame
{
    // Asset::FModel ModelToFModel(const Model& input)
    // {
    //     Asset::FModel fmodel;

    //     fmodel.Name         = input.Name;
    //     fmodel.MeshCount    = input.Meshes.size();
    //     fmodel.VertexCount  = input.Vertices.size();
    //     fmodel.IndicesCount = input.Indices.size();
    //     fmodel.TotalSize =
    //         (fmodel.VertexCount * sizeof(float)) + (fmodel.IndicesCount * sizeof(u32));

    //     fmodel.Name = input.Name;
    //     fmodel.Vertices.reserve(input.Vertices.size() * sizeof(input.Vertices[0]));
    //     for (const auto& v : input.Vertices)
    //     {
    //         fmodel.Vertices.push_back(v.Position.x);
    //         fmodel.Vertices.push_back(v.Position.y);
    //         fmodel.Vertices.push_back(v.Position.z);

    //         fmodel.Vertices.push_back(v.Normal.x);
    //         fmodel.Vertices.push_back(v.Normal.y);
    //         fmodel.Vertices.push_back(v.Normal.z);

    //         fmodel.Vertices.push_back(v.Color.x);
    //         fmodel.Vertices.push_back(v.Color.y);
    //         fmodel.Vertices.push_back(v.Color.z);

    //         fmodel.Vertices.push_back(v.TexCoord.x);
    //         fmodel.Vertices.push_back(v.TexCoord.y);
    //     }
    //     fmodel.Indices = input.Indices;
    //     fmodel.Meshes.reserve(input.Meshes.size());
    //     fmodel.Meshes = input.Meshes;
    //     return fmodel;
    // }
    // Unique<Model> FModelToModel(const Asset::FModel& input)
    // {
    //     Model* model   = new Model();
    //     model->Name    = input.Name;
    //     model->Indices = input.Indices;
    //     model->Meshes  = input.Meshes;
    //     model->Vertices.reserve(input.VertexCount);
    //     for (size_t i = 0; i < input.VertexCount * 11; i += 11)
    //     {
    //         Vertex v;
    //         v.Position.x = input.Vertices[i];
    //         v.Position.y = input.Vertices[i + 1];
    //         v.Position.z = input.Vertices[i + 2];

    //         v.Normal.x = input.Vertices[i + 3];
    //         v.Normal.y = input.Vertices[i + 4];
    //         v.Normal.z = input.Vertices[i + 5];

    //         v.Color.x = input.Vertices[i + 6];
    //         v.Color.y = input.Vertices[i + 7];
    //         v.Color.z = input.Vertices[i + 8];

    //         v.TexCoord.x = input.Vertices[i + 9];
    //         v.TexCoord.y = input.Vertices[i + 10];
    //         model->Vertices.emplace_back(std::move(v));
    //     }
    //     return Unique<Model>(model);
    // }
}  // namespace FooGame
