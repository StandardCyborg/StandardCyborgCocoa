// XCTest bridge for doctest — runs all doctest-registered test cases
// through XCTest so that `swift test` picks them up.

#define DOCTEST_CONFIG_IMPLEMENT
#import <doctest/doctest.h>
#import <XCTest/XCTest.h>
#import <Foundation/Foundation.h>

@interface SCSDKTests : XCTestCase
@end

@implementation SCSDKTests

- (void)testAll {
    // Set fixture path from SPM resource bundle so C++ tests can find test data
    NSBundle *bundle = SWIFTPM_MODULE_BUNDLE;
    NSString *fixturesPath = [bundle pathForResource:@"test_fixture_data" ofType:nil];
    if (fixturesPath) {
        setenv("SC_TEST_FIXTURES_DIR", [fixturesPath UTF8String], 1);
    }

    doctest::Context context;
    context.setOption("no-version", true);
    int result = context.run();
    XCTAssertEqual(result, 0, @"doctest tests failed — see output above for details");
}

@end
