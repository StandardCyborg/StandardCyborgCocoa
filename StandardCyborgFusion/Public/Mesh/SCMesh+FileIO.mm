//
//  SCMesh+FileIO.mm
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 10/19/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <CoreImage/CoreImage.h>
#import <MetalKit/MetalKit.h>
#import <ModelIO/ModelIO.h>
#import <SSZipArchive/SSZipArchive.h>
#import <standard_cyborg/math/Vec3.hpp>
#import <standard_cyborg/math/Vec4.hpp>
#import <standard_cyborg/io/imgfile/ColorImageFileIO.hpp>
#import <standard_cyborg/io/ply/GeometryFileIO_PLY.hpp>
#import <vector>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wconditional-uninitialized"
#import "tiny_gltf.h"
#pragma clang diagnostic pop

#import "PointCloudIO.hpp"
#import "SCMesh+FileIO.h"
#import "SCMesh+Geometry.h"
#import "SCMesh_Private.h"
#import "WriteUSDZCompatibleZip.hpp"

using namespace standard_cyborg;

@implementation SCMesh (FileIO)

- (instancetype)initWithPLYPath:(NSString *)PLYPath
                       JPEGPath:(NSString *)JPEGPath
{
    sc3d::Geometry geometry;
    io::ply::ReadGeometryFromPLYFile(geometry, [PLYPath UTF8String]);
    
    return [SCMesh meshFromGeometry:geometry
                    textureJPEGPath:JPEGPath];
}

- (BOOL)writeTextureToJPEGAtPath:(NSString *)JPEGPath
{
    if (self.textureJPEGPath != nil) {
        if ([[NSFileManager defaultManager] copyItemAtPath:self.textureJPEGPath toPath:JPEGPath error:NULL]) {
            return YES;
        }
    }
    
    std::vector<math::Vec4> rgba(self.textureWidth * self.textureHeight, math::Vec4{0, 0, 0, 0});
    memcpy(rgba.data(), [self.textureData bytes], [self.textureData length]);
    
    sc3d::ColorImage image((int)self.textureWidth, (int)self.textureHeight, rgba);
    
    return (BOOL)io::imgfile::WriteColorImageToFile([JPEGPath UTF8String], image);
}

- (BOOL)writeToPLYAtPath:(NSString *)PLYPath
{
    sc3d::Geometry geometry;
    [self toGeometry:geometry];
    
    return (BOOL)io::ply::WriteGeometryToPLYFile([PLYPath UTF8String], geometry);
}

- (BOOL)writeToOBJZipAtPath:(NSString *)objZipPath
{
    // the name, without the path and extension.
    NSString *objZipName = [[objZipPath lastPathComponent] stringByDeletingPathExtension];
    
    NSString *zipDirectory = [NSTemporaryDirectory() stringByAppendingPathComponent:@"temp-zip-dir"];
    if ([[NSFileManager defaultManager] fileExistsAtPath:zipDirectory]) {
        [[NSFileManager defaultManager] removeItemAtPath:zipDirectory error:NULL];
    }
    [[NSFileManager defaultManager] createDirectoryAtPath:zipDirectory withIntermediateDirectories:NO attributes:nil error:NULL];
    
    NSString *jpegFilename = [NSString stringWithFormat:@"%@.jpeg", objZipName];
    NSString *objFilename = [NSString stringWithFormat:@"%@.obj", objZipName];
    NSString *mtlFilename = [NSString stringWithFormat:@"%@.mtl", objZipName];
    
    NSString *tmpObjPath = [zipDirectory stringByAppendingPathComponent:objFilename];
    NSString *tmpJpegPath = [zipDirectory stringByAppendingPathComponent:jpegFilename];
    NSString *tmpMtlPath = [zipDirectory stringByAppendingPathComponent:mtlFilename];
    
    // write .obj
    {
        FILE *file = fopen([tmpObjPath UTF8String], "w");
        if (file == NULL) {
            return false;
        }
        
        fprintf(file, "# StandardCyborgFusionVersion %s\n", SCFrameworkVersion());
        fprintf(file, "# StandardCyborgFusionMetadata { \"color_space\": \"sRGB\" }\n");
        
        fprintf(file, "mtllib %s\n", [mtlFilename UTF8String]);
        fprintf(file, "o %s\n", [objZipName UTF8String]);
        
        {
            const float *floatPositions = (const float *)[self.positionData bytes];
            for (int iv = 0; iv < self.vertexCount; ++iv) {
                fprintf(file, "v %f %f %f\n", floatPositions[4 * iv + 0], floatPositions[4 * iv + 1], floatPositions[4 * iv + 2]);
            }
        }
        
        {
            const float *floatNormals = (const float *)[self.normalData bytes];
            for (int iv = 0; iv < self.vertexCount; ++iv) {
                fprintf(file, "vn %f %f %f\n", floatNormals[4 * iv + 0], floatNormals[4 * iv + 1], floatNormals[4 * iv + 2]);
            }
        }
        
        {
            const float *floatTexCoords = (const float *)[self.texCoordData bytes];
            for (int iv = 0; iv < self.vertexCount; ++iv) {
                fprintf(file, "vt %f %f\n", floatTexCoords[2 * iv + 0], floatTexCoords[2 * iv + 1]);
            }
        }
        
        fprintf(file, "usemtl Texture\n");
        fprintf(file, "s off\n");
        
        {
            const int *intFaces = (const int *)[self.facesData bytes];
            
            for (int iFace = 0; iFace < self.faceCount; iFace++) {
                int index0 = intFaces[iFace * 3 + 0] + 1;
                int index1 = intFaces[iFace * 3 + 1] + 1;
                int index2 = intFaces[iFace * 3 + 2] + 1;
                
                fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                        index0, index0, index0,
                        index1, index1, index1,
                        index2, index2, index2);
            }
        }
        
        fclose(file);
    }
    
    // write .mtl
    {
        FILE *file = fopen([tmpMtlPath UTF8String], "w");
        if (file == NULL) {
            return false;
        }
        
        fprintf(file, "# StandardCyborgFusionVersion %s\n", SCFrameworkVersion());
        fprintf(file, "# StandardCyborgFusionMetadata { \"color_space\": \"sRGB\" }\n");
        
        fprintf(file, "newmtl Texture\n");
        fprintf(file, "Ns 0.000000\n");
        fprintf(file, "Kd 1.000000 1.000000 1.000000\n");
        fprintf(file, "Ka 0.000000 0.000000 0.000000\n");
        fprintf(file, "Ks 0.000000 0.000000 0.000000\n");
        fprintf(file, "Ke 0.000000 0.000000 0.000000\n");
        fprintf(file, "map_Kd %s\n", [jpegFilename UTF8String]);
        
        fclose(file);
    }
    
    // write .jpeg
    [self writeTextureToJPEGAtPath:tmpJpegPath];
    
    [SSZipArchive createZipFileAtPath:objZipPath withContentsOfDirectory:zipDirectory];
    
    return true;
}

- (BOOL)writeToGLBAtPath:(NSString *)GLBPath
{
    std::vector<unsigned char> vertexBufferBytes;
    
    int iBeginFaces, iBeginNormals, iBeginPositions;
    int facesByteLength, normalsByteLength, positionsByteLength;
    int iBeginTexCoords, texCoordsByteLength;
    
    float ma = std::numeric_limits<float>::max();
    
    // gltf wants to know the maxPos and minPos for some reason
    math::Vec3 minPos(+ma, +ma, +ma);
    math::Vec3 maxPos(-ma, -ma, -ma);
    {
        vertexBufferBytes.clear();
        iBeginFaces = (int)vertexBufferBytes.size();
        facesByteLength = (int)(self.faceCount * sizeof(int) * 3);
        
        {
            const int *intFaces = (const int *)[self.facesData bytes];
            
            std::vector<unsigned int> uintFaces;
            for (int ii = 0; ii < self.faceCount * 3; ++ii) {
                uintFaces.push_back(intFaces[ii]);
            }
            
            const unsigned char *uintFacesBytes = (const unsigned char *)uintFaces.data();
            
            for (int ii = 0; ii < facesByteLength; ++ii) {
                vertexBufferBytes.push_back(uintFacesBytes[ii]);
            }
        }
        
        iBeginNormals = (int)vertexBufferBytes.size();
        normalsByteLength = (int)(self.vertexCount * sizeof(math::Vec3));
        
        const unsigned char *normals = (const unsigned char *)[self.normalData bytes];
        for (int ii = 0; ii < normalsByteLength; ++ii) {
            vertexBufferBytes.push_back(normals[ii]);
        }
        
        iBeginPositions = (int)vertexBufferBytes.size();
        positionsByteLength = (int)(self.vertexCount * sizeof(math::Vec3));
        
        const unsigned char *positions = (const unsigned char *)[self.positionData bytes];
        for (int ii = 0; ii < positionsByteLength; ++ii) {
            vertexBufferBytes.push_back(positions[ii]);
        }
        
        const float *floatPositions = (const float *)[self.positionData bytes];
        for (int ii = 0; ii < self.vertexCount; ++ii) {
            float x = floatPositions[4 * ii + 0];
            float y = floatPositions[4 * ii + 1];
            float z = floatPositions[4 * ii + 2];
            
            minPos.x = std::min(x, minPos.x);
            minPos.y = std::min(y, minPos.y);
            minPos.z = std::min(z, minPos.z);
            
            maxPos.x = std::max(x, maxPos.x);
            maxPos.y = std::max(y, maxPos.y);
            maxPos.z = std::max(z, maxPos.z);
        }
        
        iBeginTexCoords = (int)vertexBufferBytes.size();
        texCoordsByteLength = (int)(self.vertexCount * sizeof(math::Vec2));
        
        const unsigned char *texCoords = (const unsigned char *)[self.texCoordData bytes];
        for (int ii = 0; ii < texCoordsByteLength; ++ii) {
            vertexBufferBytes.push_back(texCoords[ii]);
        }
    }
    
    NSData *JPEGData = [self encodeTextureToJPEGData];
    
    typedef tinygltf::Value::Object Object;
    typedef tinygltf::Value Value;
    
    tinygltf::Model model;
    
    model.asset.version = "2.0";
    model.asset.generator = "StandardCyborgFusion";
    model.asset.minVersion = "2.0";
    model.asset.copyright = "Standard Cyborg";
    model.defaultScene = 0;
    
    {
        tinygltf::Scene scene;
        scene.nodes.push_back(0); // specify root node of default scene.
        model.scenes.push_back(scene);
    }
    
    // make scene, with a root node, and child mesh node.
    {
        tinygltf::Node rootGltfNode;
        
        rootGltfNode.name = "root-node";
        rootGltfNode.children.push_back(1);
        model.nodes.push_back(rootGltfNode);
        
        tinygltf::Node meshGltfNode;
        
        meshGltfNode.name = "mesh-node";
        meshGltfNode.mesh = 0;
        model.nodes.push_back(meshGltfNode);
    }
    
    // mesh
    {
        tinygltf::Mesh mesh;
        
        tinygltf::Primitive primitive;
        primitive.material = 0;
        primitive.indices = 0;
        primitive.mode = 4; // TRIANGLES
        
        primitive.attributes["NORMAL"] = 1;
        primitive.attributes["POSITION"] = 2;
        primitive.attributes["TEXCOORD_0"] = 3;
        
        mesh.primitives.push_back(primitive);
        
        model.meshes.push_back(mesh);
    }
    
    {
        tinygltf::Material material;
        material.pbrMetallicRoughness.baseColorTexture.index = 0; // use the first texture.
        material.pbrMetallicRoughness.baseColorTexture.texCoord = 0; // use TEXCOORD_0
        
        model.materials.push_back(material);
    }
    
    {
        tinygltf::Texture texture;
        
        texture.sampler = 0; // first sampler
        texture.source = 0; // first source image.
        
        model.textures.push_back(texture);
    }
    
    {
        tinygltf::Sampler sampler;
        
        sampler.minFilter = 9986; // TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR
        sampler.magFilter = 9729; // TINYGLTF_TEXTURE_FILTER_LINEAR
        
        sampler.wrapS = 10497; // TINYGLTF_TEXTURE_WRAP_REPEAT
        sampler.wrapT = 10497;
        
        model.samplers.push_back(sampler);
    }
    
    {
        tinygltf::Image image;
        
        image.bufferView = 4;
        image.mimeType = "image/jpeg";
        
        model.images.push_back(image);
    }
    
    // buffer of vertex data
    {
        tinygltf::Buffer buffer;
        buffer.data = vertexBufferBytes;
        model.buffers.push_back(buffer);
    }
    
    // buffer of texture jpeg data.
    {
        unsigned char *jpegDataStart = (unsigned char *)[JPEGData bytes];
        unsigned char *jpegDataEnd = jpegDataStart + [JPEGData length] / sizeof(unsigned char);
        std::vector<unsigned char> jpegDataVector(jpegDataStart, jpegDataEnd);
        
        tinygltf::Buffer buffer;
        buffer.data = jpegDataVector;
        model.buffers.push_back(buffer);
    }
    
    // indices buffer view.
    {
        tinygltf::BufferView bufferView;
        
        bufferView.buffer = 0;
        bufferView.byteOffset = iBeginFaces;
        bufferView.byteLength = facesByteLength;
        bufferView.byteStride = 0;
        bufferView.target = 34963; // TARGET_ELEMENT_ARRAY_BUFFER
        
        model.bufferViews.push_back(bufferView);
    }
    
    // indices accessor.
    {
        tinygltf::Accessor accessor;
        
        accessor.bufferView = 0;
        accessor.byteOffset = 0;
        accessor.normalized = false;
        accessor.componentType = 5125; // UNSIGNED_INT
        accessor.count = self.faceCount * 3;
        accessor.type = 65; // TYPE_SCALAR
        
        model.accessors.push_back(accessor);
    }
    
    // normals buffer view.
    {
        tinygltf::BufferView bufferView;
        
        bufferView.buffer = 0;
        bufferView.byteOffset = iBeginNormals;
        bufferView.byteLength = normalsByteLength;
        bufferView.byteStride = sizeof(math::Vec3);
        bufferView.target = 34962; // TARGET_ARRAY_BUFFER
        
        model.bufferViews.push_back(bufferView);
    }
    
    // normals accessor.
    {
        tinygltf::Accessor accessor;
        
        accessor.bufferView = 1;
        accessor.byteOffset = 0;
        accessor.normalized = false;
        accessor.componentType = 5126; // COMPONENT_TYPE_FLOAT
        accessor.count = self.vertexCount;
        accessor.type = 3; // TYPE_VEC3
        
        model.accessors.push_back(accessor);
    }
    
    // positions buffer view.
    {
        tinygltf::BufferView bufferView;
        
        bufferView.buffer = 0;
        bufferView.byteOffset = iBeginPositions;
        bufferView.byteLength = positionsByteLength;
        bufferView.byteStride = sizeof(math::Vec3);
        bufferView.target = 34962; // TARGET_ARRAY_BUFFER
        
        model.bufferViews.push_back(bufferView);
    }
    
    // positions accessor.
    {
        tinygltf::Accessor accessor;
        
        accessor.bufferView = 2;
        accessor.byteOffset = 0;
        accessor.normalized = false;
        accessor.componentType = 5126; // COMPONENT_TYPE_FLOAT
        accessor.count = self.vertexCount;
        accessor.type = 3; // TYPE_VEC3
        
        accessor.minValues = {minPos.x, minPos.y, minPos.z};
        accessor.maxValues = {maxPos.x, maxPos.y, maxPos.z};
        
        model.accessors.push_back(accessor);
    }
    
    // texCoords buffer view.
    {
        tinygltf::BufferView bufferView;
        
        bufferView.buffer = 0;
        bufferView.byteOffset = iBeginTexCoords;
        bufferView.byteLength = texCoordsByteLength;
        bufferView.byteStride = sizeof(math::Vec2);
        bufferView.target = 34962; // TARGET_ARRAY_BUFFER
        
        model.bufferViews.push_back(bufferView);
    }
    
    // texCoords accessor.
    {
        tinygltf::Accessor accessor;
        
        accessor.bufferView = 3;
        accessor.byteOffset = 0;
        accessor.normalized = false;
        accessor.componentType = 5126; // COMPONENT_TYPE_FLOAT
        accessor.count = self.vertexCount;
        accessor.type = 2; // TYPE_VEC2
        
        model.accessors.push_back(accessor);
    }
    
    // jpeg image buffer view
    {
        tinygltf::BufferView bufferView;
        
        bufferView.buffer = 1;
        bufferView.byteOffset = 0;
        bufferView.byteLength = [JPEGData length];
        bufferView.byteStride = 0;
        
        model.bufferViews.push_back(bufferView);
    }
    
    tinygltf::TinyGLTF loader;
    
    return loader.WriteGltfSceneToFile(&model, [GLBPath UTF8String], true, true, false, true);
    
    return true;
}

- (BOOL)writeToUSDCAtPath:(NSString *)USDCPath
{
    MDLVertexAttribute *position = [[MDLVertexAttribute alloc] initWithName:MDLVertexAttributePosition
                                                                     format:MDLVertexFormatFloat4
                                                                     offset:0
                                                                bufferIndex:0];
    MDLVertexAttribute *normal = [[MDLVertexAttribute alloc] initWithName:MDLVertexAttributeNormal
                                                                   format:MDLVertexFormatFloat4
                                                                   offset:0
                                                              bufferIndex:1];
    MDLVertexAttribute *texCoord = [[MDLVertexAttribute alloc] initWithName:MDLVertexAttributeTextureCoordinate
                                                                     format:MDLVertexFormatFloat2
                                                                     offset:0
                                                                bufferIndex:2];
    
    MDLVertexDescriptor *descriptor = [[MDLVertexDescriptor alloc] init];
    [descriptor addOrReplaceAttribute:position];
    [descriptor addOrReplaceAttribute:normal];
    [descriptor addOrReplaceAttribute:texCoord];
    [descriptor setPackedOffsets];
    [descriptor setPackedStrides];
    
    /*
    // TODO: See if we can use the texture directly, without going to disk
    MDLTexture *texture = [[MDLTexture alloc] initWithData:self.textureData
                                             topLeftOrigin:YES
                                                      name:@"Texture"
                                                dimensions:simd_make_int2((int)self.textureWidth, (int)self.textureHeight)
                                                 rowStride:self.textureWidth * 4 * sizeof(float)
                                              channelCount:4
                                           channelEncoding:MDLTextureChannelEncodingFloat32
                                                    isCube:NO];
    */
    
    // Flip the texture vertically
    NSURL *textureURL = [NSURL fileURLWithPath:self.textureJPEGPath];
    NSURL *flippedTextureURL = [NSURL fileURLWithPath:[self.textureJPEGPath stringByReplacingOccurrencesOfString:@".jpeg" withString:@"-flipped.jpeg"]];
    CIImage *flippedTexture = [CIImage imageWithContentsOfURL:textureURL];
    flippedTexture = [flippedTexture imageByApplyingTransform:CGAffineTransformMakeScale(1, -1)];
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    [[CIContext context] writeJPEGRepresentationOfImage:flippedTexture
                                                  toURL:flippedTextureURL
                                             colorSpace:colorSpace
                                                options:@{}
                                                  error:NULL];
    CGColorSpaceRelease(colorSpace);
    
    MDLScatteringFunction *scattering = [[MDLScatteringFunction alloc] init];
    [[scattering baseColor] setURLValue:flippedTextureURL];
    
    MDLMaterial *material = [[MDLMaterial alloc] initWithName:@"BaseColor" scatteringFunction:scattering];
    [[material propertyWithSemantic:MDLMaterialSemanticBaseColor] setURLValue:flippedTextureURL];
    
    MDLMeshBufferDataAllocator *allocator = [[MDLMeshBufferDataAllocator alloc] init];
    
    id<MDLMeshBuffer> positionBuffer = [allocator newBufferWithData:self.positionData type:MDLMeshBufferTypeVertex];
    id<MDLMeshBuffer> normalBuffer = [allocator newBufferWithData:self.normalData type:MDLMeshBufferTypeVertex];
    id<MDLMeshBuffer> texCoordBuffer = [allocator newBufferWithData:self.texCoordData type:MDLMeshBufferTypeVertex];
    id<MDLMeshBuffer> faceBuffer = [allocator newBufferWithData:self.facesData type:MDLMeshBufferTypeIndex];
    
    MDLSubmesh *facesSubmesh = [[MDLSubmesh alloc] initWithName:@"Faces"
                                                    indexBuffer:faceBuffer
                                                     indexCount:3 * self.faceCount
                                                      indexType:MDLIndexBitDepthUInt32
                                                   geometryType:MDLGeometryTypeTriangles
                                                       material:material];
    
    MDLMesh *mesh = [[MDLMesh alloc] initWithVertexBuffers:@[positionBuffer, normalBuffer, texCoordBuffer]
                                               vertexCount:self.vertexCount
                                                descriptor:descriptor
                                                 submeshes:@[facesSubmesh]];
    
    MDLAsset *asset = [[MDLAsset alloc] initWithBufferAllocator:allocator];
    [asset addObject:mesh];
    [asset loadTextures];
    
    NSURL *USDCURL = [NSURL fileURLWithPath:USDCPath];
    
    if ([[NSFileManager defaultManager] fileExistsAtPath:USDCPath]) {
        [[NSFileManager defaultManager] removeItemAtPath:USDCPath error:NULL];
    }
    
    NSError *error;
    BOOL success = [asset exportAssetToURL:USDCURL error:&error];
    if (!success) {
        NSLog(@"Error exporting SCMesh to %@: %@", USDCURL, error);
    }
    
    return success;
}

- (BOOL)writeToUSDZAtPath:(NSString *)USDZPath
{
    NSString *USDCPath = [[USDZPath stringByDeletingPathExtension] stringByAppendingPathExtension:@"usdc"];
    BOOL success;
    
    @autoreleasepool {
        success = [self writeToUSDCAtPath:USDCPath];
    }
    
    if (success) {
        if ([[NSFileManager defaultManager] fileExistsAtPath:USDZPath]) {
            [[NSFileManager defaultManager] removeItemAtPath:USDZPath error:NULL];
        }
        
        WriteUSDZCompatibleZip([USDZPath UTF8String], {[USDCPath UTF8String]});
    }
    
    return success;
}

@end
