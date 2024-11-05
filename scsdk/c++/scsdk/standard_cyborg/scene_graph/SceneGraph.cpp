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

#include "standard_cyborg/scene_graph/SceneGraph.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"
#include "standard_cyborg/sc3d/ColorImage.hpp"
#include "standard_cyborg/sc3d/DepthImage.hpp"
#include "standard_cyborg/sc3d/PerspectiveCamera.hpp"
#include "standard_cyborg/sc3d/Landmark.hpp"
#include "standard_cyborg/sc3d/Plane.hpp"
#include "standard_cyborg/sc3d/Polyline.hpp"

#include <stack>

namespace standard_cyborg {
namespace scene_graph {


using sc3d::Geometry;
using sc3d::ColorImage;
using sc3d::DepthImage;
using sc3d::PerspectiveCamera;
using sc3d::Landmark;
using sc3d::Plane;
using sc3d::Polyline;


// MARK: Static methods

std::set<Uuid> Node::allocatedResources;

std::vector<Uuid> Node::getAllocatedResources()
{
    std::vector<Uuid> v(allocatedResources.size());
    std::copy(allocatedResources.begin(), allocatedResources.end(), v.begin());
    return v;
}

int Node::getNumAllocatedResources()
{
    return (int)allocatedResources.size();
}

void Node::resetAllocatedResources()
{
    allocatedResources.clear();
}

std::shared_ptr<Node> Node::findParent(std::shared_ptr<Node> rootNode,
                                       std::shared_ptr<Node> targetNode)
{
    typedef std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> StackEntry;

    std::stack<StackEntry> stack;

    stack.push({rootNode, nullptr});

    while (!stack.empty()) {
        std::shared_ptr<Node> node = stack.top().first;
        std::shared_ptr<Node> parentNode = stack.top().second;

        stack.pop();

        if (node.get() == targetNode.get()) {
            return parentNode;
        }

        for (int iChild = 0; iChild < node->numChildren(); ++iChild) {
            std::shared_ptr<Node> child = node->getChildSharedPtr((node->numChildren() - iChild - 1));

            stack.push({child, node});
        }
    }

    return nullptr;
}

std::shared_ptr<Node> Node::mutateNode(std::shared_ptr<Node> targetNode,
                                       std::shared_ptr<Node> rootNode,
                                       const std::function<void(std::shared_ptr<Node>,
                                                                std::shared_ptr<Node>)>& mutateFn)
{
    std::shared_ptr<Node> copiedTargetNode = nullptr;
    std::shared_ptr<Node> copiedRootNode = nullptr;

    {
        std::vector<Uuid> finalPath;

        {
            std::stack<std::pair<std::shared_ptr<Node>, std::vector<Uuid>>> stack;

            stack.push({rootNode, {}});

            while (!stack.empty()) {
                std::shared_ptr<Node> node = stack.top().first;
                std::vector<Uuid> totalPath = stack.top().second;

                stack.pop();

                totalPath.push_back(node->getId());

                if (node.get() == targetNode.get()) {
                    finalPath = totalPath;
                    break;
                }

                for (int iChild = 0; iChild < node->numChildren(); ++iChild) {
                    std::shared_ptr<Node> child = node->getChildSharedPtr((node->numChildren() - iChild - 1));

                    stack.push({child, totalPath});
                }
            }
        }

        if (finalPath.size() == 0) {
            printf("warning final path is empty\n");
        }

        std::set<Uuid> toBeCopied;

        for (Uuid ii : finalPath) {
            toBeCopied.insert(ii);
        }

        typedef std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> StackEntry;

        std::stack<StackEntry> stack;

        stack.push(StackEntry{std::shared_ptr<Node>(rootNode), nullptr});

        while (!stack.empty()) {
            StackEntry stackEntry = stack.top();
            stack.pop();

            std::shared_ptr<Node> itNode = stackEntry.first;
            std::shared_ptr<Node> parentNode = stackEntry.second;

            if (copiedRootNode == nullptr) {
                copiedRootNode.reset(itNode->copy());

                itNode = copiedRootNode;
            } else {
                if (toBeCopied.count(itNode->getId()) != 0) {
                    itNode.reset(itNode->copy());
                }

                parentNode->children.push_back(std::shared_ptr<Node>(itNode));
            }

            if (itNode->getId() == targetNode->getId()) {
                copiedTargetNode = itNode;
            }

            if (toBeCopied.count(stackEntry.first->getId()) != 0) {
                for (int iChild = 0; iChild < stackEntry.first->numChildren(); ++iChild) {
                    std::shared_ptr<Node> child = stackEntry.first->children[(stackEntry.first->numChildren() - iChild - 1)];

                    stack.push({child, itNode});
                }
            }
        }
    }

    mutateFn(copiedTargetNode, copiedRootNode);

    return std::shared_ptr<Node>(copiedRootNode);
}

std::shared_ptr<Node> Node::findNodeWithId(std::shared_ptr<Node> rootNode, Uuid id)
{
    // Note that this is a static method so that it can return the root node itself as the
    // original shared pointer if that object is a hit. In order to make this an instance
    // method, we'd have to create a new shared pointer in the one very special case where
    // the root node is what you were looking for.

    std::stack<std::shared_ptr<Node>> stack;

    stack.push(rootNode);

    while (!stack.empty()) {
        std::shared_ptr<Node> stackEntry = stack.top();
        stack.pop();

        if (stackEntry->getId() == id) {
            return stackEntry;
        }

        for (int iChild = 0; iChild < stackEntry->numChildren(); ++iChild) {
            std::shared_ptr<Node> child = stackEntry->getChildSharedPtr((stackEntry->numChildren() - iChild - 1));

            stack.push(child);
        }
    }

    printf("Can't find id \"%s\" in node \"%s\" (id=\"%s\")\n", id.str().c_str(), rootNode->getName().c_str(), (rootNode->getId().str()).c_str());

    return std::shared_ptr<Node>();
}

int Node::calculateHistorySizeInBytes(const std::vector<std::shared_ptr<Node>>& history)
{
    // we wanna use this set, to prevent us from calculating the size of the same node twice.
    std::set<Uuid> traversedIds;

    int totalSize = 0;

    for (std::shared_ptr<Node> rootNode : history) {
        typedef std::shared_ptr<Node> StackEntry;

        std::stack<StackEntry> stack;

        stack.push(rootNode);

        while (!stack.empty()) {
            std::shared_ptr<Node> node = stack.top();
            stack.pop();

            if (traversedIds.count(node->getId()) == 0) {
                totalSize += node->approximateSizeInBytes();
                traversedIds.insert(node->getId());
            }

            for (int iChild = 0; iChild < node->numChildren(); ++iChild) {
                std::shared_ptr<Node> child = node->getChildSharedPtr((node->numChildren() - iChild - 1));
                stack.push(child);
            }
        }
    }

    return totalSize;
}

// MARK: - Non-virtual instance methods

Node::Node()
{
    id = xg::newGuid();
    resourceId = xg::newGuid();
    allocatedResources.insert(resourceId);
    name = std::string("node-") + id.str();
}

Node::Node(std::string name_) :
    Node()
{
    name = name_;
}

void Node::copyToTarget(Node* target) const
{
    target->transform = this->transform;
    target->name = this->name;
    target->material = this->material;
    target->id = this->id;
    target->nodeRevision = this->nodeRevision;
    target->treeRevision = this->treeRevision;
    target->dataURI = this->dataURI;
    target->dataResolved = this->dataResolved;
    target->visible = this->visible;
}

std::shared_ptr<Node> Node::deepCopyRecursive() const
{
    typedef std::pair<const Node*, std::shared_ptr<Node>> StackEntry;
    std::stack<StackEntry> stack;
    stack.push(StackEntry{this, nullptr});

    std::shared_ptr<Node> copiedRootNode = nullptr;

    while (!stack.empty()) {
        StackEntry stackEntry = stack.top();
        stack.pop();
        std::shared_ptr<Node> parentNode = stackEntry.second;

        const Node* originalNode = stackEntry.first;

        std::shared_ptr<Node> copiedNode(originalNode->deepCopy());
        // important to give this node a new ID.
        copiedNode->id = xg::newGuid();

        if (copiedRootNode == nullptr) {
            copiedRootNode = copiedNode;
        }

        if (parentNode != nullptr) {
            parentNode->appendChild(copiedNode, copiedRootNode);
        }

        for (int iChild = originalNode->numChildren() - 1; iChild >= 0; --iChild) {
            Node* child = originalNode->getChild(iChild);
            //thisStack.push(child);
            stack.push(StackEntry{child, copiedNode});
        }
    }

    return copiedRootNode;
}

Uuid Node::getId() const { return id; }

std::string Node::getName() const { return name; }

Node& Node::setName(const std::string& name_, std::shared_ptr<Node> rootNode)
{
    name = name_;
    touch(rootNode);
    return *this;
}

math::Transform Node::getTransform() const {
  return transform;
}

Node& Node::setTransform(const math::Transform& transform_, std::shared_ptr<Node> rootNode)
{
    transform = transform_;
    touch(rootNode);
    return *this;
}

Node& Node::setTransform(const math::Mat3x4& transform_, std::shared_ptr<Node> rootNode)
{
    return setTransform(math::Transform::fromMat3x4(transform_));
}

Material Node::getMaterial() const { return material; }

Material& Node::getMaterial() { return material; }

Node& Node::setMaterial(const Material& material_, std::shared_ptr<Node> rootNode)
{
    material = material_;
    touch(rootNode);
    return *this;
}

// MARK: - Undo/Redo support

int Node::getTreeRevision() const { return treeRevision; }

int Node::getNodeRevision() const { return nodeRevision; }

Uuid Node::getResourceId() const { return resourceId; }

void Node::touch(std::shared_ptr<Node> rootNode)
{
    ++this->nodeRevision;

    // the root node itself doesnt have a root node. so just pass 'nullptr' for these situations
    if (rootNode == nullptr) {
        ++this->treeRevision;
        return;
    }

    std::stack<std::pair<std::shared_ptr<Node>, std::vector<Node*>>> stack;

    stack.push({rootNode, {}});

    std::vector<Node*> pathFromParentToThis;

    while (!stack.empty()) {

        std::shared_ptr<Node> node = stack.top().first;
        std::vector<Node*> totalPath = stack.top().second;

        stack.pop();

        totalPath.push_back(node.get());

        if (node.get() == this) {
            pathFromParentToThis = totalPath;
            break;
        }

        for (int iChild = 0; iChild < node->numChildren(); ++iChild) {
            std::shared_ptr<Node> child = node->getChildSharedPtr((node->numChildren() - iChild - 1));

            stack.push({child, totalPath});
        }
    }

    for (Node* n : pathFromParentToThis) {
        ++n->treeRevision;
    }
}

// MARK: - Serialization support

std::string Node::getDataURI() const { return dataURI; }

Node& Node::setDataURI(const std::string& dataURI_, std::shared_ptr<Node> rootNode)
{
    dataURI = dataURI_;
    touch(rootNode);
    return *this;
}

bool Node::dataIsResolved() const { return dataResolved; }

Node& Node::markDataResolved(std::shared_ptr<Node> rootNode)
{
    dataResolved = true;
    touch(rootNode);
    return *this;
}

Node& Node::markDataUnresolved(std::shared_ptr<Node> rootNode)
{
    dataResolved = false;
    touch(rootNode);
    return *this;
}

// MARK: - Child management

int Node::numChildren() const { return (int)children.size(); }

Node* Node::getChild(int i) const { return children[i].get(); }

std::shared_ptr<Node> Node::getChildSharedPtr(int i) const { return children[i]; }

int Node::indexOfChild(std::shared_ptr<Node> childNode) const
{
    // Could use std::find, but that's equally awkward
    for (int iChild = 0; iChild < numChildren(); ++iChild) {
        if (children[iChild].get() == childNode.get()) {
            return iChild;
        }
    }

    return -1;
}

bool Node::appendChild(std::shared_ptr<Node> child,
                       std::shared_ptr<Node> rootNode)
{
    if (rootNode != nullptr) {
        if (rootNode.get() == child.get()) { // pushing the root node as a child is nonsensical.
            return false;
        }

        std::shared_ptr<Node> previousParent = Node::findParent(rootNode, child);

        // Already has a parent, so can't be added.
        if (previousParent != nullptr) {
            return false;
        }
    }

    children.push_back(child);

    if (rootNode != nullptr) {
        touch(rootNode);
    }

    return true;
}

Node& Node::setVisibility(bool newVisibility, std::shared_ptr<Node> rootNode)
{
    visible = newVisibility;

    if (rootNode != nullptr) {
        touch(rootNode);
    }

    return *this;
}

bool Node::toggleVisibility(std::shared_ptr<Node> rootNode)
{
    visible = !visible;

    if (rootNode != nullptr) {
        touch(rootNode);
    }

    return visible;
}

bool Node::isVisible() const
{
    return visible;
}

bool Node::appendChildren(std::vector<std::shared_ptr<Node>> children,
                          std::shared_ptr<Node> rootNode)
{
    bool allSucceeded = true;

    for (std::shared_ptr<Node> child : children) {
        bool succeeded = appendChild(child, rootNode);
        allSucceeded = allSucceeded && succeeded;
    }

    return allSucceeded;
}

bool Node::removeChild(std::shared_ptr<Node> childNode,
                       std::shared_ptr<Node> rootNode)
{
    int childIndex = indexOfChild(childNode);
    if (childIndex < 0) {
        return false;
    }

    children.erase(children.begin() + childIndex);

    if (rootNode != nullptr) {
        touch(rootNode);
    }

    return true;
}

void Node::removeAllChildren(std::shared_ptr<Node> rootNode)
{
    children.clear();

    if (rootNode != nullptr) {
        touch(rootNode);
    }
}

bool Node::remove(std::shared_ptr<Node> rootNode, std::shared_ptr<Node> targetNode)
{
    if (targetNode.get() == rootNode.get()) {
        // we dont allow removing root nodes at all, since the WebViewer would just end up in an invalid state
        // where there is no root node onto which we can add new nodes.
        return false;
    }

    std::shared_ptr<Node> parent = Node::findParent(rootNode, targetNode);

    targetNode->children.clear();

    if (parent) {
        int iChild = 0;
        for (iChild = 0; iChild < parent->numChildren(); ++iChild) {
            if (parent->getChild(iChild) == targetNode.get()) {
                break;
            }
        }

        // make sure the parent no longer points to this node.
        parent->children.erase(parent->children.begin() + iChild);
    }

    parent->touch(rootNode);

    return true;
}

/*
bool Node::reparent(Node* newParent)
{
    Node* node = this;
    if(node->isValidRootNode()){
        // Making the root node the child of some other node makes no sense.
        return false;
    }
    
    Node* oldParent = node->parent;
    int iChild = 0;
    // find the child index of this snode in the old parent:
    if(oldParent != nullptr) {
        for(iChild = 0; iChild < oldParent->numChildren(); ++iChild) {
            if(oldParent->getChild(iChild) == node) {
                break;
            }
        }
    }
    
    // reparent, to new parent, by transfering ownership to the new parent.
    {
        node->parent = newParent;
        
        newParent->children.push_back(std::move(oldParent->children[iChild]));
        newParent->touch();
        oldParent->touch();
    }
    
    // make sure the old parent no longer points to this node.
    oldParent->children.erase(oldParent->children.begin() + iChild);
    
    return true;
}
*/

// MARK: - Casting

bool Node::isGeometryNode() const { return getType() == SGNodeType::Geometry; }
bool Node::isColorImageNode() const { return getType() == SGNodeType::ColorImage; }
bool Node::isDepthImageNode() const { return getType() == SGNodeType::DepthImage; }
bool Node::isPerspectiveCameraNode() const { return getType() == SGNodeType::PerspectiveCamera; }
bool Node::isLandmarkNode() const { return getType() == SGNodeType::Landmark; }
bool Node::isPlaneNode() const { return getType() == SGNodeType::Plane; }
bool Node::isPolylineNode() const { return getType() == SGNodeType::Polyline; }
bool Node::isCoordinateFrameNode() const { return getType() == SGNodeType::CoordinateFrame; }
bool Node::isPointNode() const { return getType() == SGNodeType::Point; }

GeometryNode* Node::asGeometryNode() const { return (GeometryNode*)(this); }
ColorImageNode* Node::asColorImageNode() const { return (ColorImageNode*)(this); }
DepthImageNode* Node::asDepthImageNode() const { return (DepthImageNode*)(this); }
PerspectiveCameraNode* Node::asPerspectiveCameraNode() const { return (PerspectiveCameraNode*)(this); }
PlaneNode* Node::asPlaneNode() const { return (PlaneNode*)(this); }
PolylineNode* Node::asPolylineNode() const { return (PolylineNode*)(this); }
LandmarkNode* Node::asLandmarkNode() const { return (LandmarkNode*)(this); }
CoordinateFrameNode* Node::asCoordinateFrameNode() const { return (CoordinateFrameNode*)(this); }
PointNode* Node::asPointNode() const { return (PointNode*)(this); }

// MARK: -

bool Node::equals(const Node& that, bool checkRevisionCounters) const
{
    std::stack<const Node*> thisStack;
    std::stack<const Node*> thatStack;

    thisStack.push(this);
    thatStack.push(&that);

    while (!thisStack.empty()) {
        // clang-format off
        const Node* thisNode = thisStack.top(); thisStack.pop();
        const Node* thatNode = thatStack.top(); thatStack.pop();
        
        if (thisNode->getType() != thatNode->getType() ||
            thisNode->numChildren() != thatNode->numChildren() ||
            thisNode->getName() != thatNode->getName() ||
            thisNode->getTransform() != thatNode->getTransform() ||
            thisNode->getMaterial().objectColor != thatNode->getMaterial().objectColor ||
            thisNode->getMaterial().materialModel != thatNode->getMaterial().materialModel 
        ) {
            return false;
        }
        
        if (checkRevisionCounters &&
            (thisNode->getTreeRevision() != thatNode->getTreeRevision() ||
             thisNode->getNodeRevision() != thatNode->getNodeRevision()))
        {
            return false;
        }
        // clang-format on

        switch (thisNode->getType()) {
            case SGNodeType::Generic:
                break;

            case SGNodeType::CoordinateFrame:
                break;

            case SGNodeType::Point: {
                math::Vec3 thisPosition = thisNode->asPointNode()->getPosition();
                math::Vec3 thatPosition = thatNode->asPointNode()->getPosition();

                if (thisPosition != thatPosition) {
                    return false;
                }

                break;
            }
                
                    
            case SGNodeType::Geometry: {
                const Geometry& thisGeometry = thisNode->asGeometryNode()->getGeometry();
                const Geometry& thatGeometry = thatNode->asGeometryNode()->getGeometry();

                if (thisGeometry.getPositions().size() != thatGeometry.getPositions().size()) {
                    return false;
                }

                if (thisGeometry.getNormals().size() != thatGeometry.getNormals().size()) {
                    return false;
                }

                if (thisGeometry.getColors().size() != thatGeometry.getColors().size()) {
                    return false;
                }

                if (thisGeometry.getFaces().size() != thatGeometry.getFaces().size()) {
                    return false;
                }

                break;
            }

            case SGNodeType::Landmark: {
                Landmark thisLandmark = thisNode->asLandmarkNode()->getLandmark();
                Landmark thatLandmark = thatNode->asLandmarkNode()->getLandmark();

                if (thisLandmark != thatLandmark) {
                    return false;
                }

                break;
            }

            case SGNodeType::Plane: {
                Plane thisPlane = thisNode->asPlaneNode()->getPlane();
                Plane thatPlane = thatNode->asPlaneNode()->getPlane();

                if (thisPlane != thatPlane) {
                    return false;
                }

                break;
            }

            case SGNodeType::ColorImage: {
                ColorImage thisColorImage = thisNode->asColorImageNode()->getColorImage();
                ColorImage thatColorImage = thatNode->asColorImageNode()->getColorImage();

                if (!(thisColorImage == thatColorImage)) {
                    return false;
                }

                break;
            }

            case SGNodeType::DepthImage: {
                DepthImage thisDepthImage = thisNode->asDepthImageNode()->getDepthImage();
                DepthImage thatDepthImage = thatNode->asDepthImageNode()->getDepthImage();

                if (!(thisDepthImage == thatDepthImage)) {
                    return false;
                }

                break;
            }
                
            case SGNodeType::Polyline: {
                const Polyline& thisPolyline = thisNode->asPolylineNode()->getPolyline();
                const Polyline& thatPolyline = thatNode->asPolylineNode()->getPolyline();

                if (!(thisPolyline.getPositions() == thatPolyline.getPositions())) {
                    return false;
                }

                break;
            }

            case SGNodeType::PerspectiveCamera: {
                const PerspectiveCamera& thisPerspectiveCamera = thisNode->asPerspectiveCameraNode()->getPerspectiveCamera();
                const PerspectiveCamera& thatPerspectiveCamera = thatNode->asPerspectiveCameraNode()->getPerspectiveCamera();

                if (!(thisPerspectiveCamera == thatPerspectiveCamera)) {
                    return false;
                }

                break;
            }
                
            default:
                // Unhandled type; return false
                return false;
        }

        if (thisNode->numChildren() != thatNode->numChildren()) {
            return false;
        }

        for (int iChild = 0; iChild < thisNode->numChildren(); ++iChild) {
            Node* child = thisNode->getChild(iChild);
            thisStack.push(child);
        }

        for (int iChild = 0; iChild < thatNode->numChildren(); ++iChild) {
            Node* child = thatNode->getChild(iChild);
            thatStack.push(child);
        }
    }

    return true;
}

// MARK: - Range-based enumeration

NodeIterator::NodeIterator(std::shared_ptr<Node> parent, int childIndex) :
    _parent(parent),
    _childIndex(childIndex)
{}

std::shared_ptr<Node> NodeIterator::operator*()
{
    return _parent->getChildSharedPtr(_childIndex);
}

NodeIterator NodeIterator::operator++()
{
    NodeIterator iterator = *this;
    ++_childIndex;
    return iterator;
}

bool NodeIterator::operator==(const NodeIterator& rhs)
{
    return _childIndex == rhs._childIndex;
}

bool NodeIterator::operator!=(const NodeIterator& rhs)
{
    return _childIndex != rhs._childIndex;
}


NodeIterator begin(std::shared_ptr<Node> parent)
{
    return NodeIterator(parent);
}

NodeIterator end(std::shared_ptr<Node> parent)
{
    return NodeIterator(parent, parent->numChildren());
}

} // namespace scene_graph
} // namespace standard_cyborg
