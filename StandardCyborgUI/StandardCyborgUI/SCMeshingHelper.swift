import Foundation
import StandardCyborgFusion

public class SCMeshingHelper {
    enum Status {
        case inProgress(Float)
        case success(SCMesh)
        case failure(Error)
    }

    private static var defaultMeshingParameters: SCMeshingParameters {
        let meshingParameters = SCMeshingParameters()
        meshingParameters.resolution = 5
        meshingParameters.smoothness = 2
        meshingParameters.surfaceTrimmingAmount = 6
        meshingParameters.closed = true
        return meshingParameters
    }
    
    let pointCloud: SCPointCloud
    let meshTexturing: SCMeshTexturing
    let meshingParameters: SCMeshingParameters
    
    init(pointCloud: SCPointCloud, meshTexturing: SCMeshTexturing, meshingParameters: SCMeshingParameters? = nil) {
        self.pointCloud = pointCloud
        self.meshTexturing = meshTexturing
        self.meshingParameters = meshingParameters ?? Self.defaultMeshingParameters
    }
     
    func processMesh(onMeshStatusUpdate: @escaping ((Status) -> Void)) {
        meshTexturing.reconstructMesh(
            pointCloud: pointCloud,
            textureResolution: 2048,
            meshingParameters: meshingParameters,
            progress: { (progress, stop) in
                onMeshStatusUpdate(.inProgress(progress))
            },
            completion: { (error, mesh) in
                if let mesh = mesh {
                    onMeshStatusUpdate(.success(mesh))
                    return
                }
                
                onMeshStatusUpdate(.failure(error!))
        })
    }
}
