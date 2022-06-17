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


#pragma once

#include <set>
#include <string>
#include <stack>
#include <vector>

#include "standard_cyborg/util/guid.hpp"
#include "standard_cyborg/math/Transform.hpp"
#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Vec3.hpp"

namespace standard_cyborg {

namespace math {
struct Mat3x4;
} // namespace math

namespace sc3d {
class Geometry;
class ColorImage;
class DepthImage;
class PerspectiveCamera;
class Landmark;
struct Plane;
class Polyline;
} // namespace sc3d


namespace scene_graph {

enum class SGNodeType {
    Generic,
    Geometry,
    Landmark,
    Plane,
    Polyline,
    ColorImage,
    DepthImage,
    PerspectiveCamera,
    CoordinateFrame,
    Point
};

struct GeometryNode;
struct ColorImageNode;
struct DepthImageNode;
struct PerspectiveCameraNode;
struct LandmarkNode;
struct PlaneNode;
struct PolylineNode;
struct CoordinateFrameNode;
struct PointNode;

enum class MaterialModel {
    Unlit,
    Phong
};

struct Material {
    math::Vec3 objectColor = math::Vec3(1, 1, 1);
    MaterialModel materialModel = MaterialModel::Unlit;
};

typedef xg::Guid Uuid;

class Node {
private:
    static std::set<Uuid> allocatedResources;

protected:
    std::vector<std::shared_ptr<Node>> children;
    math::Transform transform;
    std::string name;
    
    Material material;

    Uuid resourceId;

    int nodeRevision = 0;
    int treeRevision = 0;

    std::string dataURI;
    bool dataResolved = true;

    Uuid id;

    bool visible = true;

public:
    // MARK: - Virtual instance methods

    virtual ~Node();

    /** Gets the NodeType of this node */
    virtual SGNodeType getType() const;

    /** Gets the memory size of the contents of this Node. NB: this doesnt include the children of this node. */
    virtual int approximateSizeInBytes() const;

    /** Copies only this node (and not any of its children), without assigning a new id to the copy. */
    virtual Node* copy() const;

    /** Copies only this node (and not any of its children), assigning a new id to the copy. */
    virtual Node* deepCopy() const;

    // MARK: - Static methods

    /** Get list of the resource ids, of all allocated nodes so far.
     Useful for tracking whether we have memory leaks. */
    static std::vector<Uuid> getAllocatedResources();

    static int getNumAllocatedResources();

    /** Clear the list of allocated resource ids. Mainly useful for unit testing. */
    static void resetAllocatedResources();

    /** Query the approximate size of a list of nodes */
    static int calculateHistorySizeInBytes(const std::vector<std::shared_ptr<Node>>& history);

    /** Mutate a target node under a root node, returning a new root node */
    static std::shared_ptr<Node> mutateNode(std::shared_ptr<Node> targetNode,
                                            std::shared_ptr<Node> rootNode,
                                            const std::function<void(std::shared_ptr<Node>, std::shared_ptr<Node>)>& mutateFn);

    /** Find a node by id by searching in a node (including the root node itself). */
    static std::shared_ptr<Node> findNodeWithId(std::shared_ptr<Node> rootNode, Uuid id);

    /** Find a node parent by searching in a node */
    static std::shared_ptr<Node> findParent(std::shared_ptr<Node> rootNode,
                                            std::shared_ptr<Node> targetNode);

    /** Remove a node under a root node */
    static bool remove(std::shared_ptr<Node> rootNode, std::shared_ptr<Node> targetNode);

    // MARK: - Non-virtual instance methods -

    /** Construct an empty node */
    Node();

    /** Construct an empty generic node with the specified name */
    Node(std::string name);

    /** Copies this node's contents to the target */
    void copyToTarget(Node* target) const;

    /** Deep copy this node and its children, assigning new ids to all copied nodes. */
    std::shared_ptr<Node> deepCopyRecursive() const;

    /** Recursively traverse, and check for equality between two nodes, and all of its children.
     We can turn off check equality of the revision counters, since you dont always want to
     check them for equality in unit testing.
     */
    bool equals(const Node& that, bool checkRevisionCounters = false) const;

    /** Get an integer identifier, unique only per-execution */
    Uuid getId() const;

    /** Does not modify revision counters up to the root node if rootNode is not specified. */
    Node& setName(const std::string& name, std::shared_ptr<Node> rootNode = nullptr);
    std::string getName() const;
    
    /** Set the transform of this node. Does not modify revision counters up to the root nodeif rootNode is not specified. */
    Node& setTransform(const math::Transform& transform, std::shared_ptr<Node> rootNode = nullptr);
    Node& setTransform(const math::Mat3x4& transform, std::shared_ptr<Node> rootNode = nullptr);
    
    math::Transform getTransform() const;

    Node& setRotation(math::Quaternion rotation, std::shared_ptr<Node> rootNode = nullptr);
    Node& setTranslation(math::Vec3 translation, std::shared_ptr<Node> rootNode = nullptr);
    Node& setScale(math::Vec3 scale, std::shared_ptr<Node> rootNode = nullptr);
    Node& setShear(math::Vec3 shear, std::shared_ptr<Node> rootNode = nullptr);

    math::Quaternion setRotation(math::Quaternion rotation);
    math::Vec3 setTranslation(math::Vec3 translation);
    math::Vec3 setScale(math::Vec3 scale);
    math::Vec3 setShear(math::Vec3 shear);

    /** Set the visibility of the node */
    Node& setVisibility(bool newVisibility, std::shared_ptr<Node> rootNode = nullptr);

    /** Toggle node visibility and return the the resulting visibility */
    bool toggleVisibility(std::shared_ptr<Node> rootNode = nullptr);

    /** Get the visibility of the node */
    bool isVisible() const;

    Material& getMaterial();
    Material getMaterial() const;
    Node& setMaterial(const Material& material, std::shared_ptr<Node> rootNode = nullptr);

    /** Get a visual representation of the node for node types (like a plane or camera) whose visual representation
        does not directly correspond to the underlying data */
    std::shared_ptr<sc3d::Geometry> getRepresentationGeometry() const;

    bool hasRepresentationGeometry() const;

    // MARK: Revision counters for change tracking

    /** Return the current tree revision counter value */
    int getTreeRevision() const;

    /** Return the current node revision counter value */
    int getNodeRevision() const;

    /** If a node is re-allocated because of mutateNode, it still has the same
     value of getId(). However, however, the resourceId of this re-allocated node
     is new, and different from the old node. */
    Uuid getResourceId() const;

    /** Increment the revision counter and propagate up to the root node */
    void touch(std::shared_ptr<Node> rootNode);

    // MARK: Serialization support

    /** Set the data URI for the serialized content of the current node */
    Node& setDataURI(const std::string& str, std::shared_ptr<Node> rootNode = nullptr);

    /** Get the data URI of the serialized content of the current node */
    std::string getDataURI() const;

    /** Whether data reference has been resolved and parsed */
    bool dataIsResolved() const;
    Node& markDataResolved(std::shared_ptr<Node> rootNode = nullptr);
    Node& markDataUnresolved(std::shared_ptr<Node> rootNode = nullptr);

    // MARK: Child management

    /** Gets the number of children of this node */
    int numChildren() const;

    /** Gets child #i */
    Node* getChild(int i) const;
    std::shared_ptr<Node> getChildSharedPtr(int i) const;
    
    /** Returns the index of the specified child node of this parent node, or -1 if not found */
    int indexOfChild(std::shared_ptr<Node> childNode) const;

    /** Recursive */
    template <class NodeClass = Node>
    std::shared_ptr<NodeClass> firstChildNamed(std::string searchName) const
    {
        for (auto child : children) {
            if (child->name == searchName) {
                return std::dynamic_pointer_cast<NodeClass>(child);
            }

            auto childWithName = child->firstChildNamed<Node>(searchName);

            if (childWithName != nullptr) {
                return std::dynamic_pointer_cast<NodeClass>(childWithName);
            }
        }

        return nullptr;
    }

    
    /** Adds a child node under this node. Modifies revision counters up to the root node if rootNode is specified. */
    bool appendChild(std::shared_ptr<Node> child, std::shared_ptr<Node> rootNode = nullptr);
    
    /** Adds *and takes ownership* of the given child */
    void appendNewChild(Node *child) {
        appendChild(std::shared_ptr<Node>(child));
    }

    /** Adds a list of children under this node. Modifies revision counters up to the root node if if rootNode is specified. */
    bool appendChildren(std::vector<std::shared_ptr<Node>> children,
                        std::shared_ptr<Node> rootNode = nullptr);

    /** Removes the specified child node of this node, and all of its children.
        Modifies revision counters if root node is specified. */
    bool removeChild(std::shared_ptr<Node> targetNode,
                     std::shared_ptr<Node> rootNode = nullptr);

    /** Removes all child nodes of this node.
        Modifies revision counters if root node is specified. */
    void removeAllChildren(std::shared_ptr<Node> rootNode = nullptr);

    /** Remove this node from its current parent, and make it the child of some other node. */
    //bool reparent(Node* newParent);

    // MARK: Casting

    bool isGeometryNode() const;
    bool isColorImageNode() const;
    bool isDepthImageNode() const;
    bool isPerspectiveCameraNode() const;
    bool isLandmarkNode() const;
    bool isPlaneNode() const;
    bool isPolylineNode() const;
    bool isCoordinateFrameNode() const;
    bool isPointNode() const;

    GeometryNode* asGeometryNode() const;
    ColorImageNode* asColorImageNode() const;
    DepthImageNode* asDepthImageNode() const;
    PerspectiveCameraNode* asPerspectiveCameraNode() const;
    LandmarkNode* asLandmarkNode() const;
    PlaneNode* asPlaneNode() const;
    PolylineNode* asPolylineNode() const;
    CoordinateFrameNode* asCoordinateFrameNode() const;
    PointNode *asPointNode() const;
};

// MARK: - GeometryNode

struct GeometryNode : public Node {
private:
    std::shared_ptr<sc3d::Geometry> geometry;

public:
    virtual SGNodeType getType() const;
    virtual int approximateSizeInBytes() const;
    virtual GeometryNode* copy() const;
    virtual GeometryNode* deepCopy() const;

    GeometryNode();
    GeometryNode(const std::string& name, std::shared_ptr<sc3d::Geometry> geometry = nullptr);
    GeometryNode(const std::string& name, const sc3d::Geometry& geometry);
    virtual ~GeometryNode();

    sc3d::Geometry& getGeometry();
    const sc3d::Geometry& getGeometry() const;

#ifdef EMBIND_ONLY
    sc3d::Geometry* getGeometryPtr();
#endif // EMBIND_ONLY

    void setGeometry(std::shared_ptr<sc3d::Geometry> geometry);
    void setGeometry(const sc3d::Geometry& geometry);
};

// MARK: - DepthImageNode

struct DepthImageNode : public Node {
private:
    std::shared_ptr<sc3d::DepthImage> image;

public:
    virtual SGNodeType getType() const;
    virtual int approximateSizeInBytes() const;
    virtual DepthImageNode* copy() const;
    virtual DepthImageNode* deepCopy() const;

    DepthImageNode();
    DepthImageNode(const std::string& name, std::shared_ptr<sc3d::DepthImage> image = nullptr);
    DepthImageNode(const std::string& name, const sc3d::DepthImage& image);
    DepthImageNode(const std::string& name, sc3d::DepthImage&& image);
    virtual ~DepthImageNode();

    sc3d::DepthImage& getDepthImage();
    const sc3d::DepthImage& getDepthImage() const;

#ifdef EMBIND_ONLY
    sc3d::DepthImage* getDepthImagePtr();
#endif // EMBIND_ONLY

    void setDepthImage(std::shared_ptr<sc3d::DepthImage> otherImage);
    void setDepthImage(const sc3d::DepthImage& image);

    bool hasRepresentationGeometry() const;
    std::shared_ptr<sc3d::Geometry> getRepresentationGeometry() const;
};

// MARK: - CoordinateFrameNode

struct CoordinateFrameNode : public Node {
public:
    virtual SGNodeType getType() const;
    virtual int approximateSizeInBytes() const;
    virtual CoordinateFrameNode* copy() const;
    virtual CoordinateFrameNode* deepCopy() const;

    CoordinateFrameNode();
    CoordinateFrameNode(const std::string& name, const math::Mat3x4& axes = math::Mat3x4::Identity());
    virtual ~CoordinateFrameNode();
};

// MARK: - ColorImageNode

struct ColorImageNode : public Node {
private:
    std::shared_ptr<sc3d::ColorImage> image;
    
public:
    virtual SGNodeType getType() const;
    virtual int approximateSizeInBytes() const;
    virtual ColorImageNode* copy() const;
    virtual ColorImageNode* deepCopy() const;

    ColorImageNode();
    ColorImageNode(const std::string& name, std::shared_ptr<sc3d::ColorImage> image = nullptr);
    ColorImageNode(const std::string& name, const sc3d::ColorImage& image);
    ColorImageNode(const std::string& name, sc3d::ColorImage&& image);
    virtual ~ColorImageNode();

    sc3d::ColorImage& getColorImage();
    const sc3d::ColorImage& getColorImage() const;

#ifdef EMBIND_ONLY
    sc3d::ColorImage* getColorImagePtr();
#endif // EMBIND_ONLY

    void setColorImage(std::shared_ptr<sc3d::ColorImage> otherImage);
    void setColorImage(const sc3d::ColorImage& image);

    bool hasRepresentationGeometry() const;
    std::shared_ptr<sc3d::Geometry> getRepresentationGeometry() const;
};

// MARK: - PerspectiveCameraNode

struct PerspectiveCameraNode : public Node {
private:
    std::shared_ptr<sc3d::PerspectiveCamera> camera;

public:
    virtual SGNodeType getType() const;
    virtual int approximateSizeInBytes() const;
    virtual PerspectiveCameraNode* copy() const;
    virtual PerspectiveCameraNode* deepCopy() const;

    PerspectiveCameraNode();
    virtual ~PerspectiveCameraNode();

    PerspectiveCameraNode(const std::string& name, std::shared_ptr<sc3d::PerspectiveCamera> camera = nullptr);
    PerspectiveCameraNode(const std::string& name, const sc3d::PerspectiveCamera& camera);

    sc3d::PerspectiveCamera& getPerspectiveCamera();
    const sc3d::PerspectiveCamera& getPerspectiveCamera() const;

#ifdef EMBIND_ONLY
    sc3d::PerspectiveCamera* getPerspectiveCameraPtr();
#endif // EMBIND_ONLY

    void setPerspectiveCamera(std::shared_ptr<sc3d::PerspectiveCamera> camera);
    void setPerspectiveCamera(const sc3d::PerspectiveCamera& camera);

    bool hasRepresentationGeometry() const;
    std::shared_ptr<sc3d::Geometry> getRepresentationGeometry() const;
};

// MARK: - PointNode

struct PointNode : public Node {
    
    math::Vec3 position;

public:
    virtual SGNodeType getType() const;
    virtual int approximateSizeInBytes() const;
    virtual PointNode* copy() const;
    virtual PointNode* deepCopy() const;

    PointNode();
    PointNode(const std::string& name, const math::Vec3& position = math::Vec3(0, 0, 0));
    PointNode(const sc3d::Landmark& landmark);
    virtual ~PointNode();
    
    math::Vec3 getPosition() const {
        return position;
    }
    
    void setPosition(const math::Vec3& p) {
        this->position = p;
    }
    
    
    sc3d::Landmark getAsLandmark() const;
    
#ifdef EMBIND_ONLY
    // Return this point's data as if it's a landmark so that we don't need extra conditionals
    // in the rendering code. Beware that this returns a landmark at [0, 0, 0] so that the node
    // transform positions it correctly.
    sc3d::Landmark getLandmark() const;
#endif
};

// MARK: - LandmarkNode

struct LandmarkNode : public Node {
private:
    std::shared_ptr<sc3d::Landmark> landmark;

public:
    virtual SGNodeType getType() const;
    virtual int approximateSizeInBytes() const;
    virtual LandmarkNode* copy() const;
    virtual LandmarkNode* deepCopy() const;

    LandmarkNode();
    LandmarkNode(const std::string& name, std::shared_ptr<sc3d::Landmark> landmark = nullptr);
    LandmarkNode(const std::string& name, const sc3d::Landmark& landmark);

    sc3d::Landmark& getLandmark();
    const sc3d::Landmark& getLandmark() const;

    void setLandmark(std::shared_ptr<sc3d::Landmark> landmark);
    void setLandmark(const sc3d::Landmark& landmark);

#ifdef EMBIND_ONLY
    sc3d::Landmark* getLandmarkPtr();
#endif // EMBIND_ONLY
};

// MARK: - PolylineNode

struct PolylineNode : public Node {
private:
    std::shared_ptr<sc3d::Polyline> polyline;

public:
    virtual SGNodeType getType() const;
    virtual int approximateSizeInBytes() const;
    virtual PolylineNode* copy() const;
    virtual PolylineNode* deepCopy() const;

    PolylineNode();
    PolylineNode(const std::string& name, std::shared_ptr<sc3d::Polyline> polyline = nullptr);
    PolylineNode(const std::string& name, const sc3d::Polyline& polyline);

    sc3d::Polyline& getPolyline();
    const sc3d::Polyline& getPolyline() const;

    void setPolyline(std::shared_ptr<sc3d::Polyline> polyline);
    void setPolyline(const sc3d::Polyline& polyline);

#ifdef EMBIND_ONLY
    sc3d::Polyline* getPolylinePtr();
#endif // EMBIND_ONLY
};

// MARK: - PlaneNode

struct PlaneNode : public Node {
private:
    std::shared_ptr<sc3d::Plane> plane;
    math::Vec2 extents;

public:
    virtual SGNodeType getType() const;
    virtual int approximateSizeInBytes() const;
    virtual PlaneNode* copy() const;
    virtual PlaneNode* deepCopy() const;

    PlaneNode();
    PlaneNode(const std::string& name, std::shared_ptr<sc3d::Plane> plane = nullptr);
    PlaneNode(const std::string& name, const sc3d::Plane& plane);

    sc3d::Plane& getPlane();
    const sc3d::Plane& getPlane() const;

    void setPlane(std::shared_ptr<sc3d::Plane> plane);
    void setPlane(const sc3d::Plane& plane);

#ifdef EMBIND_ONLY
    sc3d::Plane* getPlanePtr();
#endif // EMBIND_ONLY

    math::Vec2 getExtents() const;
    void setExtents(math::Vec2 newExtents);
};

// MARK: - NodeIterator

class NodeIterator {
private:
    std::shared_ptr<Node> _parent;
    int _childIndex;

public:
    NodeIterator(std::shared_ptr<Node> parent, int childIndex = 0);

    std::shared_ptr<Node> operator*();
    NodeIterator operator++();
    bool operator==(const NodeIterator& rhs);
    bool operator!=(const NodeIterator& rhs);
};

NodeIterator begin(std::shared_ptr<Node> parent);
NodeIterator end(std::shared_ptr<Node> parent);


} // namespace scene_graph
} // namespace standard_cyborg
