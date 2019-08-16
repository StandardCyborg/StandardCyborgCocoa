//
//  ServerS3Support.swift
//  StandardCyborgNetworking
//
//  Created by Aaron Thompson on 8/15/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation
import PromiseKit

fileprivate struct ClientAPIPath {
    static let s3Files = "direct_uploads"
}

internal struct S3UploadInfo: Codable {
    let url: URL
    let uploadHeaders: [String : Any]
    let directUploadFileKey: String
    
    enum CodingKeys: String, CodingKey {
        case directUpload, directUploadFileKey = "key"
    }
    
    enum DirectUploadKeys: String, CodingKey {
        case url, headers
    }
    
    init(from decoder: Decoder) throws {
        let container = try! decoder.container(keyedBy: CodingKeys.self)
        
        let directUploadContainer = try! container.nestedContainer(keyedBy: DirectUploadKeys.self, forKey: .directUpload)
        url = try! directUploadContainer.decode(URL.self, forKey: .url)
        uploadHeaders = try! directUploadContainer.decode([String : String].self, forKey: .headers)
        directUploadFileKey = try! container.decode(String.self, forKey: .directUploadFileKey)
    }
    
    func encode(to encoder: Encoder) throws {
        fatalError("Not currently supported (but implemented since our generic networking code requires objects to conform to Encodable)")
    }
}


extension ServerOperation {
    
    internal func _createS3FileReference(for localFileURL: URL, remoteFilename: String) -> Promise<(URL, S3UploadInfo)> {
        return Promise { seal in
            guard let fileMD5 = MD5File(url: localFileURL) else {
                seal.reject(ServerOperationError.genericErrorString("Couldn't calculate MD5 for \(localFileURL)"))
                return
            }
            
            guard
                let fileAttributes = try? FileManager.default.attributesOfItem(atPath: localFileURL.path),
                let fileSize = fileAttributes[FileAttributeKey.size] as? UInt64
            else {
                seal.reject(ServerOperationError.genericErrorString("Couldn't get file size for \(localFileURL)"))
                return
            }
            
            let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.s3Files)
            let postDict = [
                "blob": [
                    "content_type": "text/plain",
                    "filename": remoteFilename,
                    "checksum": fileMD5.base64EncodedString(),
                    "byte_size": fileSize,
                ]
            ]
            
            print("Creating S3 file reference for \(localFileURL)")
            serverAPIClient.performJSONOperation(withURL: url,
                                                 httpMethod: HTTPMethod.POST,
                                                 httpBodyDict: postDict,
                                                 responseObjectRootKey: nil)
            { (result: Result<S3UploadInfo>) in
                switch result {
                case .success(let uploadInfo):
                    print("Successfully created S3 file reference \(uploadInfo.directUploadFileKey)")
                    seal.fulfill((localFileURL, uploadInfo))
                case .failure(let error):
                    seal.reject(ServerOperationError.genericErrorString("Error creating S3 file from result: \(error.localizedDescription)"))
                }
            }
        }
    }
    
}
