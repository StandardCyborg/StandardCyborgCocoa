//
//  DateTimeTransform.swift
//  Scanner
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import ObjectMapper

class DateTimeTransform: DateFormatterTransform {
    
    // DEV: Due to inconsistencies in the server, the server sends us
    //      a couple different date/time formats, so we have to support both
    private static let _Formatter = DateFormatter(withFormat: "yyyy-MM-dd HH:mm:ss Z", locale: "en_US_POSIX")
    
    private static let _FallbackFormatter: ISO8601DateFormatter = {
        let formatter = ISO8601DateFormatter()
        formatter.formatOptions.insert(ISO8601DateFormatter.Options.withFractionalSeconds)
        return formatter
    }()
    
    init() {
        super.init(dateFormatter: DateTimeTransform._Formatter)
    }
    
    override func transformFromJSON(_ value: Any?) -> Date? {
        if let value = value, let date = super.transformFromJSON(value) {
            return date
        }
        
        if let stringValue = value as? String {
            return DateTimeTransform._FallbackFormatter.date(from: stringValue)
        }
        
        return nil
    }
    
    override func transformToJSON(_ value: Date?) -> String? {
        guard let value = value else { return nil }
        
        if let string = super.transformToJSON(value) {
            return string
        }
        
        return DateTimeTransform._FallbackFormatter.string(from: value)
    }
}
