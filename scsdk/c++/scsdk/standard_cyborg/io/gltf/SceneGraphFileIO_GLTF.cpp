/*
Copyright 2020 Standard Cyborg

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "standard_cyborg/io/gltf/SceneGraphFileIO_GLTF.hpp"

#include "standard_cyborg/io/gltf/Base64.hpp"
#include "standard_cyborg/io/imgfile/ColorImageFileIO.hpp"
#include "standard_cyborg/io/ply/DepthImageFileIO_PLY.hpp"
#include "standard_cyborg/io/ply/GeometryFileIO_PLY.hpp"
#include "standard_cyborg/io/json/LandmarkFileIO_JSON.hpp"
#include "standard_cyborg/io/json/PerspectiveCameraFileIO_JSON.hpp"
#include "standard_cyborg/io/json/PlaneFileIO_JSON.hpp"
#include "standard_cyborg/io/json/PolylineFileIO_JSON.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/scene_graph/SceneGraph.hpp"

#include <iostream>
#include <sstream>
#include <stack>
#include <string>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_JSON
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
//#define STBI_NO_JPEG
//#define STBI_NO_PNG
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wconditional-uninitialized"
#include <nlohmann/json.hpp>
#include <tiny_gltf.h>
#pragma clang diagnostic pop


namespace standard_cyborg {
namespace io {
namespace gltf {


namespace sg = standard_cyborg::scene_graph;
using namespace standard_cyborg::io::imgfile;
using namespace standard_cyborg::io::json;
using namespace standard_cyborg::io::ply;


static bool startsWith(const std::string& s, const std::string& prefix)
{
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

static const char* dataURLPrefix = "data:text/plain;base64,";

static bool isDataURL(const std::string& str)
{
    return startsWith(str, dataURLPrefix);
}

static std::string decodeDataURL(const std::string& input)
{
    if (!isDataURL(input)) return "";

    size_t prefixLength = std::string(dataURLPrefix).size();
    size_t dataLength = input.size() - prefixLength;
    return base64_decode(input.data() + prefixLength, dataLength);
}

math::Mat3x4 columnMajorToMat3x4(const std::vector<double> m)
{
    return math::Mat3x4{
        (float)m[0], (float)m[4], (float)m[8], (float)m[12],
        (float)m[1], (float)m[5], (float)m[9], (float)m[13],
        (float)m[2], (float)m[6], (float)m[10], (float)m[14]};
}

std::vector<double> toColumnMajor(const math::Mat3x4& m)
{
    return std::vector<double>{
        m.m00, m.m10, m.m20, 0.0,
        m.m01, m.m11, m.m21, 0.0,
        m.m02, m.m12, m.m22, 0.0,
        m.m03, m.m13, m.m23, 1.0};
}

static std::string serializeNode(const sg::Node& node)
{
    using sg::Node;
    using sg::SGNodeType;

    std::ostringstream os;

    // Filed under: tech debt.
    Node& nonConstNode = const_cast<Node&>(node);

    switch (node.getType()) {
        case SGNodeType::Generic:
            break;
        case SGNodeType::Geometry:
            WriteGeometryToPLYStream(os, nonConstNode.asGeometryNode()->getGeometry());
            break;
        case SGNodeType::ColorImage:
            WriteColorImageToStream(os, nonConstNode.asColorImageNode()->getColorImage(), ImageFormat::PNG);
            break;
        case SGNodeType::DepthImage:
            WriteDepthImageToPLYStream(os, nonConstNode.asDepthImageNode()->getDepthImage());
            break;
        case SGNodeType::PerspectiveCamera:
            WritePerspectiveCameraToJSONStream(os, nonConstNode.asPerspectiveCameraNode()->getPerspectiveCamera());
            break;
        case SGNodeType::Landmark:
            WriteLandmarkToJSONStream(os, nonConstNode.asLandmarkNode()->getLandmark());
            break;
        case SGNodeType::Plane:
            WritePlaneToJSONStream(os, nonConstNode.asPlaneNode()->getPlane());
            break;
        case SGNodeType::Polyline:
            WritePolylineToJSONStream(os, nonConstNode.asPolylineNode()->getPolyline());
            break;
        case SGNodeType::Point:
        case SGNodeType::CoordinateFrame:
            break;
    }

    return os.str();
}

static void setMaterialFromGltfNode(const tinygltf::Node& gltfNode,
                                    const std::shared_ptr<sg::Node>& node)
{
    if (gltfNode.extras.IsObject() && gltfNode.extras.Has("material")) {
        {
            math::Vec3 objectColor;

            if (gltfNode.extras.Get("material").Has("objectColor")) {
                tinygltf::Value::Object materialObj = gltfNode.extras.Get("material").Get<tinygltf::Value::Object>();
                objectColor.x = (float)materialObj["objectColor"].Get(0).Get<double>();
                objectColor.y = (float)materialObj["objectColor"].Get(1).Get<double>();
                objectColor.z = (float)materialObj["objectColor"].Get(2).Get<double>();
            } else if (gltfNode.extras.Get("material").Has("emissive")) {
                // handle old files
                tinygltf::Value::Object materialObj = gltfNode.extras.Get("material").Get<tinygltf::Value::Object>();
                objectColor.x = (float)materialObj["emissive"].Get(0).Get<double>();
                objectColor.y = (float)materialObj["emissive"].Get(1).Get<double>();
                objectColor.z = (float)materialObj["emissive"].Get(2).Get<double>();
            } else {
                // handle old files, with no objectColor.
                objectColor.x = 1.0f;
                objectColor.y = 1.0f;
                objectColor.z = 1.0f;
            }
            node->getMaterial().objectColor = objectColor;
        }

        {
            sg::MaterialModel materialModel = sg::MaterialModel::Unlit;

            if (gltfNode.extras.Get("material").Has("materialModel")) {
                std::string materialModelString = gltfNode.extras.Get("material").Get("materialModel").Get<std::string>();
                if (materialModelString == "phong") {
                    materialModel = sg::MaterialModel::Phong;
                } else if (materialModelString == "unlit") {
                    materialModel = sg::MaterialModel::Unlit;
                } else {
                    std::cerr<<std::string("Warning: Unrecognized material model \"") + materialModelString + "\""<<std::endl;
                }
            }
            
            node->getMaterial().materialModel = materialModel;
        }
    }
}

static sg::SGNodeType SGNodeTypeFromGltfNode(const tinygltf::Node& gltfNode)
{
    using namespace sg;

    std::string dataType = "";
    if (gltfNode.extras.IsObject() && gltfNode.extras.Has("dataType") && gltfNode.extras.Get("dataType").IsString()) {
        dataType = gltfNode.extras.Get("dataType").Get<std::string>();
    }

    // Special case: we *expect* gltfNode.extras.Get("dataType") to equal "geometry",
    // but we opt not to enforce it since mesh implies that.
    if (gltfNode.mesh != -1 || dataType == "geometry") {
        return SGNodeType::Geometry;
    } else if (dataType == "colorImage") {
        return SGNodeType::ColorImage;
    } else if (dataType == "depthImage") {
        return SGNodeType::DepthImage;
    } else if (dataType == "perspectiveCamera") {
        return SGNodeType::PerspectiveCamera;
    } else if (dataType == "landmark") {
        return SGNodeType::Landmark;
    } else if (dataType == "plane") {
        return SGNodeType::Plane;
    } else if (dataType == "polyline") {
        return SGNodeType::Polyline;
    } else if (dataType == "coordinateFrame") {
        return SGNodeType::CoordinateFrame;
    } else if (dataType == "point") {
        return SGNodeType::Point;
    }
    else {
        return SGNodeType::Generic;
    }
}

// Read gltf stored on our platform into a SceneGraph. All meshes will contain dummy mesh data.
// In a post-processing pass, we read the .ply-files stored in the `extras` field, to acquire the
// actual mesh data.
std::vector<std::shared_ptr<sg::Node>> ReadSceneGraphFromGltf(const std::string& gltfSource)
{
    typedef std::pair<int, std::shared_ptr<sg::Node>> StackEntry;
    // typedef tinygltf::Value::Object Object;
    // typedef tinygltf::Value Value;

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string errorMessage;
    std::string warningMessage;
    std::vector<std::shared_ptr<sg::Node>> rootNodes;

    bool loadAsciiSuccess = loader.LoadASCIIFromString(&model, &errorMessage, &warningMessage, gltfSource.c_str(), (unsigned int)gltfSource.size(), "");

    if (!warningMessage.empty()) {
        std::cout << "Warning : " << warningMessage << std::endl;
    }
    if (!loadAsciiSuccess) {
        std::cerr << "Error: " << errorMessage << std::endl;
        return rootNodes;
    }
    if (model.scenes.size() == 0 || model.scenes[0].nodes.size() == 0) {
        std::cerr << "Error: GLTF at " << gltfSource << " has no scenes" << std::endl;
        return rootNodes;
    }
    
    std::stack<StackEntry> stack;
    for (int i = 0; i < model.scenes[0].nodes.size(); i++) {
        stack.push(StackEntry{model.scenes[0].nodes[i], nullptr});
    }

    while (!stack.empty()) {
        using namespace sg;
        
        StackEntry stackEntry = stack.top();
        stack.pop();
        int nodeIndex = stackEntry.first;
        std::shared_ptr<sg::Node> parentNode = stackEntry.second;
        tinygltf::Node gltfNode = model.nodes[nodeIndex];

        SGNodeType nodeType = SGNodeTypeFromGltfNode(gltfNode);
        const std::string& dataURI = gltfNode.extras.Get("dataURI").Get<std::string>();

        std::shared_ptr<sg::Node> node;

        switch (nodeType) {
            case SGNodeType::Generic:
                node.reset(new sg::Node());
                break;

            case SGNodeType::Geometry: {
                tinygltf::Mesh gltfMesh = model.meshes[gltfNode.mesh];
                const std::string& meshDataURI = gltfMesh.extras.Get("dataURI").Get<std::string>();

                node.reset(new GeometryNode());

                if (isDataURL(meshDataURI)) {
                    std::istringstream decodedData(decodeDataURL(meshDataURI));
                    ReadGeometryFromPLYStream(node->asGeometryNode()->getGeometry(), decodedData);
                }

                int iTexture = -1;

                if (gltfMesh.extras.Has("texture")) {
                    iTexture = gltfMesh.extras.Get("texture").Get<int>();
                }

                if (iTexture >= 0) {
                    SCASSERT(iTexture < model.textures.size(), "texture index out of bounds");

                    tinygltf::Texture gltfTexture = model.textures[iTexture];

                    SCASSERT(gltfTexture.sampler < model.samplers.size(), "sample index out of bounds");
                    SCASSERT(gltfTexture.source < model.images.size(), "source image index out of bounds");

                    tinygltf::Sampler sampler = model.samplers[gltfTexture.sampler];
                    tinygltf::Image sourceImage = model.images[gltfTexture.source];

                    SCASSERT(sourceImage.mimeType == "image/png", "Unsupported mime type");

                    SCASSERT(sourceImage.bufferView < model.bufferViews.size(), "bufferView index out of bounds ");

                    tinygltf::BufferView bufferView = model.bufferViews[sourceImage.bufferView];

                    SCASSERT(bufferView.buffer < model.buffers.size(), "buffer index out of bounds");

                    tinygltf::Buffer buffer = model.buffers[bufferView.buffer];

                    std::string dataStr(bufferView.byteLength, '\0');
                    for (int iByte = 0; iByte < bufferView.byteLength; ++iByte) {
                        char byte = buffer.data[bufferView.byteOffset + iByte];
                        dataStr[iByte] = byte;
                    }

                    std::istringstream datastream(dataStr);
                    sc3d::ColorImage texture;
                    ReadColorImageFromStream(texture, datastream);

                    node->asGeometryNode()->getGeometry().setTexture(texture);
                }

                break;
            }

            case SGNodeType::ColorImage: {
                node.reset(new ColorImageNode());

                if (isDataURL(dataURI)) {
                    std::istringstream decodedData(decodeDataURL(dataURI));
                    ReadColorImageFromStream(node->asColorImageNode()->getColorImage(), decodedData);
                }

                break;
            }

            case SGNodeType::DepthImage: {
                node.reset(new DepthImageNode());

                if (isDataURL(dataURI)) {
                    std::istringstream decodedData(decodeDataURL(dataURI));
                    ReadDepthImageFromPLYStream(node->asDepthImageNode()->getDepthImage(), decodedData);
                }

                break;
            }

            case SGNodeType::PerspectiveCamera: {
                node.reset(new PerspectiveCameraNode());

                if (isDataURL(dataURI)) {
                    std::istringstream decodedData(decodeDataURL(dataURI));
                    ReadPerspectiveCameraFromJSONStream(node->asPerspectiveCameraNode()->getPerspectiveCamera(), decodedData);
                }

                break;
            }

            case SGNodeType::Landmark: {
                node.reset(new LandmarkNode());

                if (isDataURL(dataURI)) {
                    std::istringstream decodedData(decodeDataURL(dataURI));
                    ReadLandmarkFromJSONStream(node->asLandmarkNode()->getLandmark(), decodedData);
                }

                break;
            }

            case SGNodeType::Plane: {
                node.reset(new PlaneNode());

                if (isDataURL(dataURI)) {
                    std::istringstream decodedData(decodeDataURL(dataURI));
                    ReadPlaneFromJSONStream(node->asPlaneNode()->getPlane(), decodedData);
                }

                break;
            }

            case SGNodeType::Polyline: {
                node.reset(new PolylineNode());

                if (isDataURL(dataURI)) {
                    std::istringstream decodedData(decodeDataURL(dataURI));
                    ReadPolylineFromJSONStream(node->asPolylineNode()->getPolyline(), decodedData);
                }

                break;
            }

            case SGNodeType::CoordinateFrame: {
                node.reset(new CoordinateFrameNode());
                break;
            }

            case SGNodeType::Point: {
                node.reset(new PointNode());
                break;
            }

            default:
                // Just a generic node
                node.reset(new sg::Node());
                break;
        }

        bool isVisible = true;
        if (gltfNode.extras.Has("visible")) {
            isVisible = gltfNode.extras.Get("visible").Get<bool>();
        }
        node->setVisibility(isVisible);
        
        node->setTransform(columnMajorToMat3x4(gltfNode.matrix));
        node->setName(gltfNode.name);
        node->setDataURI(dataURI);
        setMaterialFromGltfNode(gltfNode, node);

        if (parentNode != nullptr) {
            parentNode->appendChild(node);
        } else {
            rootNodes.push_back(node);
        }

        for (int child : gltfNode.children) {
            stack.push(StackEntry{child, node});
        }
    }

    return rootNodes;
}

tinygltf::Value vec3toGltfValue(const math::Vec3& v)
{
    std::vector<tinygltf::Value> arr;

    arr.push_back(tinygltf::Value(v.x));
    arr.push_back(tinygltf::Value(v.y));
    arr.push_back(tinygltf::Value(v.z));

    return tinygltf::Value(arr);
}

tinygltf::Node createGltfNode(sg::Node& node)
{
    tinygltf::Node gltfNode;
    gltfNode.matrix = toColumnMajor(math::Mat3x4::fromTransform(node.getTransform()));
    gltfNode.name = node.getName();

    {
        tinygltf::Value::Object gltfMaterialObj;
        gltfMaterialObj["objectColor"] = vec3toGltfValue(node.getMaterial().objectColor);

        std::string materialModelString;
        switch(node.getMaterial().materialModel) {
            case sg::MaterialModel::Phong:
                materialModelString = "phong";
                break;
            case sg::MaterialModel::Unlit:
            default:
                materialModelString = "unlit";
        }
        gltfMaterialObj["materialModel"] = tinygltf::Value(materialModelString);

        tinygltf::Value::Object gltfExtrasObj;
        gltfExtrasObj["material"] = tinygltf::Value(gltfMaterialObj);

        gltfNode.extras = tinygltf::Value(gltfExtrasObj);
    }

    return gltfNode;
}

typedef std::pair<std::shared_ptr<sg::Node>, int> StackEntry;

void NodePostProcess(const StackEntry& stackEntry, std::stack<StackEntry>& stack, tinygltf::Model& model)
{
    std::shared_ptr<sg::Node> node = stackEntry.first;

    int parentNodeIndex = stackEntry.second;
    int currentNodeIndex = (int)model.nodes.size() - 1;

    if (parentNodeIndex != -1) {
        model.nodes[parentNodeIndex].children.push_back(currentNodeIndex);
    }

    for (int iChild = 0; iChild < node->numChildren(); ++iChild) {
        std::shared_ptr<sg::Node> child = node->getChildSharedPtr(iChild);
        stack.push(StackEntry{child, currentNodeIndex});
    }
}

bool WriteSceneGraphToGltf(std::shared_ptr<sg::Node> sceneGraph, const std::string& path)
{
    return WriteSceneGraphToGltf(std::vector<std::shared_ptr<sg::Node>>{sceneGraph}, path);
}

// For backwards compatibility with the original API. If we received a raw pointer, we wrap it in a shared
// pointer while ensuring the resource is not actually freed when the shared pointer goes out of scope.
void null_deleter(sg::Node*) {}
bool WriteSceneGraphToGltf(sg::Node* sceneGraph, const std::string& path)
{
    std::shared_ptr<sg::Node> sceneGraphPtr(sceneGraph, &null_deleter);
    return WriteSceneGraphToGltf(std::vector<std::shared_ptr<sg::Node>>{sceneGraphPtr}, path);
}

// Write SceneGraph to gltf, that will be stored on our platform. Every mesh-node will contain an empty dummy mesh.
// The real mesh data is stored as a ply-path in the `extras` field of the same mesh-node.
bool WriteSceneGraphToGltf(std::vector<std::shared_ptr<sg::Node>> sceneGraph, const std::string& path)
{
    typedef tinygltf::Value::Object Object;
    typedef tinygltf::Value Value;

    tinygltf::Model model;

    model.asset.version = "2.0";
    model.asset.generator = "StandardCyborgSceneGraph";
    model.asset.minVersion = "2.0";
    model.asset.copyright = "Standard Cyborg";

    tinygltf::Scene scene;

    // `second` contains parent index of the node.
    std::stack<StackEntry> stack;
    for (int i = 0; i < sceneGraph.size(); i++) {
        stack.push(StackEntry{sceneGraph[i], -1});
    }

    int iGltfTexture = 0;
    int iGltfImage = 0;
    int iGltfSampler = 0;
    int iGltfBufferView = 0;
    int iGltfBuffer = 0;

    int nodeIndex = 0;
    while (!stack.empty()) {
        StackEntry stackEntry = stack.top();
        stack.pop();
        std::shared_ptr<sg::Node> node = stackEntry.first;
        tinygltf::Node gltfNode = createGltfNode(*node);

        using namespace sg;
        switch (node->getType()) {
            case SGNodeType::Generic:
                gltfNode.extras.Get<Object>()["visible"] = tinygltf::Value(node->isVisible());
                break;

            case SGNodeType::Geometry: {
                tinygltf::Mesh mesh;

                mesh.name = node->getName();
                mesh.extras = Value(Object());
                mesh.extras.Get<Object>()["visible"] = tinygltf::Value(node->isVisible());
                mesh.extras.Get<Object>()["dataType"] = tinygltf::Value(std::string("geometry"));
                mesh.extras.Get<Object>()["dataURI"] = tinygltf::Value(std::string("data:text/plain;base64,") + base64_encode(serializeNode(*node)));


                if (node->asGeometryNode()->getGeometry().hasTexture()) {

                    const sc3d::ColorImage& texture = node->asGeometryNode()->getGeometry().getTexture();


                    std::ostringstream os;
                    WriteColorImageToStream(os, texture, ImageFormat::PNG);
                    std::string oss = os.str();

                    std::vector<unsigned char> bufferData;
                    for (char c : oss) {
                        bufferData.push_back(c);
                    }

                    //std::vector<unsigned char> bufferData(&oss[0], oss.size());
                    /*
                    
                    const std::vector<Vec4>& vecData = texture.getData();

                    const unsigned char* byteData = (unsigned char*)vecData.data();
                    int bufferSize = (int)sizeof(Vec4) * (int)vecData.size();
                    
                    */
                    {
                        tinygltf::Texture texture;

                        texture.sampler = iGltfSampler; // first sampler
                        texture.source = iGltfImage; // first source image.

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

                        image.bufferView = iGltfBufferView;
                        image.mimeType = "image/png";

                        model.images.push_back(image);
                    }

                    {
                        tinygltf::BufferView bufferView;

                        bufferView.buffer = iGltfBuffer;
                        bufferView.byteOffset = 0;
                        bufferView.byteLength = bufferData.size();
                        bufferView.byteStride = 0;

                        model.bufferViews.push_back(bufferView);
                    }

                    // buffer of texture jpeg data.
                    {
                        tinygltf::Buffer buffer;
                        buffer.data = bufferData;
                        model.buffers.push_back(buffer);
                    }

                    mesh.extras.Get<Object>()["texture"] = tinygltf::Value((int)iGltfTexture);

                    iGltfTexture++;
                    iGltfImage++;
                    iGltfSampler++;
                    iGltfBufferView++;
                    iGltfBuffer++;

                } else {
                    mesh.extras.Get<Object>()["texture"] = tinygltf::Value((int)-1);
                }

                model.meshes.push_back(mesh);
                gltfNode.mesh = (int)model.meshes.size() - 1;

                break;
            }

            case SGNodeType::DepthImage:
                gltfNode.extras.Get<Object>()["visible"] = tinygltf::Value(node->isVisible());
                gltfNode.extras.Get<Object>()["dataType"] = tinygltf::Value(std::string("depthImage"));
                gltfNode.extras.Get<Object>()["dataURI"] = tinygltf::Value(std::string("data:text/plain;base64,") + base64_encode(serializeNode(*node)));
                break;

            case SGNodeType::ColorImage:
                gltfNode.extras.Get<Object>()["visible"] = tinygltf::Value(node->isVisible());
                gltfNode.extras.Get<Object>()["dataType"] = tinygltf::Value(std::string("colorImage"));
                gltfNode.extras.Get<Object>()["dataURI"] = tinygltf::Value(std::string("data:text/plain;base64,") + base64_encode(serializeNode(*node)));
                break;

            case SGNodeType::PerspectiveCamera:
                gltfNode.extras.Get<Object>()["visible"] = tinygltf::Value(node->isVisible());
                gltfNode.extras.Get<Object>()["dataType"] = tinygltf::Value(std::string("perspectiveCamera"));
                gltfNode.extras.Get<Object>()["dataURI"] = tinygltf::Value(std::string("data:text/plain;base64,") + base64_encode(serializeNode(*node)));
                break;

            case SGNodeType::Landmark:
                gltfNode.extras.Get<Object>()["visible"] = tinygltf::Value(node->isVisible());
                gltfNode.extras.Get<Object>()["dataType"] = tinygltf::Value(std::string("landmark"));
                gltfNode.extras.Get<Object>()["dataURI"] = tinygltf::Value(std::string("data:text/plain;base64,") + base64_encode(serializeNode(*node)));
                break;

            case SGNodeType::Plane:
                gltfNode.extras.Get<Object>()["visible"] = tinygltf::Value(node->isVisible());
                gltfNode.extras.Get<Object>()["dataType"] = tinygltf::Value(std::string("plane"));
                gltfNode.extras.Get<Object>()["dataURI"] = tinygltf::Value(std::string("data:text/plain;base64,") + base64_encode(serializeNode(*node)));
                break;

            case SGNodeType::Polyline:
                gltfNode.extras.Get<Object>()["visible"] = tinygltf::Value(node->isVisible());
                gltfNode.extras.Get<Object>()["dataType"] = tinygltf::Value(std::string("polyline"));
                gltfNode.extras.Get<Object>()["dataURI"] = tinygltf::Value(std::string("data:text/plain;base64,") + base64_encode(serializeNode(*node)));
                break;

            case SGNodeType::Point:
                gltfNode.extras.Get<Object>()["visible"] = tinygltf::Value(node->isVisible());
                gltfNode.extras.Get<Object>()["dataType"] = tinygltf::Value(std::string("point"));
                break;

            case SGNodeType::CoordinateFrame:
                gltfNode.extras.Get<Object>()["visible"] = tinygltf::Value(node->isVisible());
                gltfNode.extras.Get<Object>()["dataType"] = tinygltf::Value(std::string("coordinateFrame"));
                break;

            default:
                printf("Ignoring export of unsupported node type %d\n", node->getType());
                break;
        }

        model.nodes.push_back(gltfNode);

        // If this node has no parent, add it to the scene as a top-level node
        if (stackEntry.second == -1) {
            scene.nodes.push_back(nodeIndex);
        }

        NodePostProcess(stackEntry, stack, model);
        nodeIndex++;
    }

    model.scenes.push_back(scene);

    tinygltf::TinyGLTF loader;
    return loader.WriteGltfSceneToFile(&model, path, true, true, true, false);

    //     return loader.WriteGltfSceneToFile(&model, [GLBPath UTF8String], true, true, false, true);
}

} // namespace gltf
} // namespace io
} // namespace standard_cyborg
