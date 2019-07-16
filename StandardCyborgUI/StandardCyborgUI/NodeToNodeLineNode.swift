//
//  NodeToNodeLineNode.swift
//  StandardCyborgUI
//
//  Created by Aaron Thompson on 7/15/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import SceneKit

@objc public class NodeToNodeLineNode: SCNNode {
    
    private let _cylinderNode: SCNNode
    
    @objc public init(startNode: SCNNode,
                      endNode: SCNNode,
                      thickness: CGFloat,
                      color: UIColor)
    {
        let cylinder = SCNCylinder(radius: thickness, height: 1)
        cylinder.firstMaterial?.diffuse.contents = color
        _cylinderNode = SCNNode(geometry: cylinder)
        _cylinderNode.eulerAngles = SCNVector3(x: Float.pi / 2.0, y: 0, z: 0)
        
        super.init()
        
        addChildNode(_cylinderNode)
        
        let positionConstraint = SCNTransformConstraint.positionConstraint(inWorldSpace: false) {
            [weak startNode, weak endNode, unowned cylinder]
            (node: SCNNode, position: SCNVector3) -> SCNVector3 in
            guard let startNode = startNode, let endNode = endNode else { return SCNVector3Zero }
            
            // Update the cylinder's height while we're at it
            cylinder.height = CGFloat(SCNVector3Distance(startNode.position, endNode.position))
            
            let start = startNode.position
            let end = endNode.position
            let middle = SCNVector3Midpoint(start, end)
            
            return middle
        }
        
        let lookAtConstraint = SCNLookAtConstraint(target: endNode)
        
        positionConstraint.isIncremental = false
        lookAtConstraint.isIncremental = false
        
        constraints = [positionConstraint, lookAtConstraint]
    }
    
    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
}

public func SCNVector3Distance(_ a: SCNVector3, _ b: SCNVector3) -> Float
{
    let dx: Float = a.x - b.x
    let dy: Float = a.y - b.y
    let dz: Float = a.z - b.z
    
    return sqrtf(dx * dx + dy * dy + dz * dz)
}

public func SCNVector3Midpoint(_ a: SCNVector3, _ b: SCNVector3) -> SCNVector3
{
    let dx: Float = b.x - a.x
    let dy: Float = b.y - a.y
    let dz: Float = b.z - a.z
    
    return SCNVector3(x: dx / 2.0 + a.x,
                      y: dy / 2.0 + a.y,
                      z: dz / 2.0 + a.z)
}
