//
//  DateTimeTransform.swift
//  StandardCyborgNetworking
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation

public class DateTimeTransform {
    
    // DEV: Due to inconsistencies in the server, the server sends us
    //      a couple different date/time formats, so we have to support both
    private static let _Formatter: DateFormatter = {
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd HH:mm:ss Z"
        formatter.locale = Locale(identifier: "en_US_POSIX")
        return formatter
    }()
    
    private static let _FallbackFormatter: ISO8601DateFormatter = {
        let formatter = ISO8601DateFormatter()
        formatter.formatOptions.insert(ISO8601DateFormatter.Options.withFractionalSeconds)
        return formatter
    }()
    
    private init() {}
    
    public static func fromString(_ value: String) -> Date? {
        return _Formatter.date(from: value) ?? _FallbackFormatter.date(from: value)
    }
    
    public static func toString(_ value: Date) -> String {
        return _Formatter.string(from: value)
    }
}
