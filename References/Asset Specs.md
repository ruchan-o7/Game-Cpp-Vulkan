# Foo game asset specifications.
All assets must create from Scene Editor. Scene editor should be responsible all asset importing and scene serializing. Game should only read scene and corresponding assets. All asset file tyoes stores two kind of information. One for asset and one for pointer to original asset data except material asset.

Currently there are 3 types of assets:
- Image asset
- Model asset
- Material asset

## Images
Contains meta-data for image file such as name, extension, format, import time, file pointer. etc... `.fimg` extension used for this asset type.

Two seperate file must be exists for per each image file that imported. Asset file stores with `.fimg` extension. Asset file and original image should be side-by-side. Asset data can be `*.png, *.jpg`.

|Name              | Type   |
|------------------|--------|
|Id                | UUID   |
|Name              | String |
|Create Time       | Date   |
|Last change       | Date   |
|Image Name        | String |
|Image Size        | u64    |
|Image extension   | String |
|Image fullname    | String |
|Image full path   | String |

## Materials
There are two kind of material right now `Lit` and `Unlit`. Lit for general purpose pbr like material. 
UnLit is generally debug purpose only diffuse map used material. `.fmat` extension used for this asset type.

<table>
<tr>
    <th>Unlit material specs</th>
    <th>Lit material specs</th>
</tr>
<tr>
<td>

|Name               | Type          |
|-------------------|---------------|
|Id                 | UUID          |
|Name               | String        |
|Create Time        | Date          |
|Last change        | Date          |
|Diffuse Map        | UUID          |
|Diffuse tint       | Array\<float> | // size = 3
</td>
<td>

|Name               | Type         |
|-------------------|--------------|
|Id                 | UUID          |
|Name               | String        |
|Create Time        | Date          |
|Last change        | Date          |
|Diffuse map        | UUID          |
|Diffuse tint       | Array\<float> | // size = 3
|Rougness map       | UUID          |
|Roughness factor   | float         |
|Metallic map       | UUID          |
|Metallic factor    | float         | 
</td>

</table>

## Models
`.fmodel` is used for this type Asset. Original model file should exact directory with asset file. Contains list of meshes. Mesh contains draw primitive data. 

|Name                  | Type         |
|----------------------|--------------|
|Id                    | UUID         |
|Name                  | String       |
|Create Time           | Date         |
|Last change           | Date         |
|Total bytes of size   | i64          |
|Total vertex count    | i64          |
|Total index count     | i64          |
|Total Primitive count | i64          |
|Total Mesh count      | i64          |
|Model path            | String       |
|Array of meshes       | Array\<Mesh> | // size = arbitrary

#### Mesh
Contains primitives that contains draw instructions.
| Name       | Type              |
|------------|-------------------|
| Name       | String            |
| Transform  | Mat4              |
| Primitives | Array\<Primitive> |

#### Primitive
| Name        | Type |
|-------------|------|
| First index | u64  |
| Index count | u64  |
| Material id | UUID |