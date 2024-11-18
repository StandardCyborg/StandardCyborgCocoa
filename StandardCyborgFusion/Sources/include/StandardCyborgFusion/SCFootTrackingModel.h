//
// SCFootTrackingModel.h
//
// This file was automatically generated and should not be edited.
//

#import <Foundation/Foundation.h>
#import <CoreML/CoreML.h>
#import <stdint.h>
#import <os/log.h>

NS_ASSUME_NONNULL_BEGIN

/// Model Prediction Input Type
API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0)) __attribute__((visibility("hidden")))
@interface SCFootTrackingModelInput : NSObject<MLFeatureProvider>

/// Input image as color (kCVPixelFormatType_32BGRA) image buffer, 300 pixels wide by 300 pixels high
@property (readwrite, nonatomic) CVPixelBufferRef image;

/// (optional) IOU Threshold override as double value
@property (readwrite, nonatomic) double iouThreshold;

/// (optional) Confidence Threshold override as double value
@property (readwrite, nonatomic) double confidenceThreshold;
- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithImage:(CVPixelBufferRef)image iouThreshold:(double)iouThreshold confidenceThreshold:(double)confidenceThreshold NS_DESIGNATED_INITIALIZER;

- (nullable instancetype)initWithImageFromCGImage:(CGImageRef)image iouThreshold:(double)iouThreshold confidenceThreshold:(double)confidenceThreshold error:(NSError * _Nullable __autoreleasing * _Nullable)error API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0)) __attribute__((visibility("hidden")));

- (nullable instancetype)initWithImageAtURL:(NSURL *)imageURL iouThreshold:(double)iouThreshold confidenceThreshold:(double)confidenceThreshold error:(NSError * _Nullable __autoreleasing * _Nullable)error API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0)) __attribute__((visibility("hidden")));

-(BOOL)setImageWithCGImage:(CGImageRef)image error:(NSError * _Nullable __autoreleasing * _Nullable)error  API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0)) __attribute__((visibility("hidden")));
-(BOOL)setImageWithURL:(NSURL *)imageURL error:(NSError * _Nullable __autoreleasing * _Nullable)error  API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0)) __attribute__((visibility("hidden")));
@end

/// Model Prediction Output Type
API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0)) __attribute__((visibility("hidden")))
@interface SCFootTrackingModelOutput : NSObject<MLFeatureProvider>

/// Boxes × Class confidence as multidimensional array of doubles
@property (readwrite, nonatomic, strong) MLMultiArray * confidence;

/// Boxes × [x, y, width, height] (relative to image size) as multidimensional array of doubles
@property (readwrite, nonatomic, strong) MLMultiArray * coordinates;
- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithConfidence:(MLMultiArray *)confidence coordinates:(MLMultiArray *)coordinates NS_DESIGNATED_INITIALIZER;

@end

/// Class for model loading and prediction
API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0)) __attribute__((visibility("hidden")))
@interface SCFootTrackingModel : NSObject
@property (readonly, nonatomic, nullable) MLModel * model;

/**
    URL of the underlying .mlmodelc directory.
*/
+ (nullable NSURL *)URLOfModelInThisBundle;

/**
    Initialize SCFootTrackingModel instance from an existing MLModel object.

    Usually the application does not use this initializer unless it makes a subclass of SCFootTrackingModel.
    Such application may want to use `-[MLModel initWithContentsOfURL:configuration:error:]` and `+URLOfModelInThisBundle` to create a MLModel object to pass-in.
*/
- (instancetype)initWithMLModel:(MLModel *)model NS_DESIGNATED_INITIALIZER;

/**
    Initialize SCFootTrackingModel instance with the model in this bundle.
*/
- (instancetype)init;

/**
    Initialize SCFootTrackingModel instance with the model in this bundle.

    @param configuration The model configuration object
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
*/
- (nullable instancetype)initWithConfiguration:(MLModelConfiguration *)configuration error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Initialize SCFootTrackingModel instance from the model URL.

    @param modelURL URL to the .mlmodelc directory for SCFootTrackingModel.
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
*/
- (nullable instancetype)initWithContentsOfURL:(NSURL *)modelURL error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Initialize SCFootTrackingModel instance from the model URL.

    @param modelURL URL to the .mlmodelc directory for SCFootTrackingModel.
    @param configuration The model configuration object
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
*/
- (nullable instancetype)initWithContentsOfURL:(NSURL *)modelURL configuration:(MLModelConfiguration *)configuration error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Construct SCFootTrackingModel instance asynchronously with configuration.
    Model loading may take time when the model content is not immediately available (e.g. encrypted model). Use this factory method especially when the caller is on the main thread.

    @param configuration The model configuration
    @param handler When the model load completes successfully or unsuccessfully, the completion handler is invoked with a valid SCFootTrackingModel instance or NSError object.
*/
+ (void)loadWithConfiguration:(MLModelConfiguration *)configuration completionHandler:(void (^)(SCFootTrackingModel * _Nullable model, NSError * _Nullable error))handler API_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0)) __attribute__((visibility("hidden")));

/**
    Construct SCFootTrackingModel instance asynchronously with URL of .mlmodelc directory and optional configuration.

    Model loading may take time when the model content is not immediately available (e.g. encrypted model). Use this factory method especially when the caller is on the main thread.

    @param modelURL The model URL.
    @param configuration The model configuration
    @param handler When the model load completes successfully or unsuccessfully, the completion handler is invoked with a valid SCFootTrackingModel instance or NSError object.
*/
+ (void)loadContentsOfURL:(NSURL *)modelURL configuration:(MLModelConfiguration *)configuration completionHandler:(void (^)(SCFootTrackingModel * _Nullable model, NSError * _Nullable error))handler API_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0)) __attribute__((visibility("hidden")));

/**
    Make a prediction using the standard interface
    @param input an instance of SCFootTrackingModelInput to predict from
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
    @return the prediction as SCFootTrackingModelOutput
*/
- (nullable SCFootTrackingModelOutput *)predictionFromFeatures:(SCFootTrackingModelInput *)input error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Make a prediction using the standard interface
    @param input an instance of SCFootTrackingModelInput to predict from
    @param options prediction options
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
    @return the prediction as SCFootTrackingModelOutput
*/
- (nullable SCFootTrackingModelOutput *)predictionFromFeatures:(SCFootTrackingModelInput *)input options:(MLPredictionOptions *)options error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Make an asynchronous prediction using the standard interface
    @param input an instance of SCFootTrackingModelInput to predict from
    @param completionHandler a block that will be called upon completion of the prediction. error will be nil if no error occurred.
*/
- (void)predictionFromFeatures:(SCFootTrackingModelInput *)input completionHandler:(void (^)(SCFootTrackingModelOutput * _Nullable output, NSError * _Nullable error))completionHandler API_AVAILABLE(macos(14.0), ios(17.0), watchos(10.0), tvos(17.0)) __attribute__((visibility("hidden")));

/**
    Make an asynchronous prediction using the standard interface
    @param input an instance of SCFootTrackingModelInput to predict from
    @param options prediction options
    @param completionHandler a block that will be called upon completion of the prediction. error will be nil if no error occurred.
*/
- (void)predictionFromFeatures:(SCFootTrackingModelInput *)input options:(MLPredictionOptions *)options completionHandler:(void (^)(SCFootTrackingModelOutput * _Nullable output, NSError * _Nullable error))completionHandler API_AVAILABLE(macos(14.0), ios(17.0), watchos(10.0), tvos(17.0)) __attribute__((visibility("hidden")));

/**
    Make a prediction using the convenience interface
    @param image Input image as color (kCVPixelFormatType_32BGRA) image buffer, 300 pixels wide by 300 pixels high
    @param iouThreshold (optional) IOU Threshold override as double value
    @param confidenceThreshold (optional) Confidence Threshold override as double value
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
    @return the prediction as SCFootTrackingModelOutput
*/
- (nullable SCFootTrackingModelOutput *)predictionFromImage:(CVPixelBufferRef)image iouThreshold:(double)iouThreshold confidenceThreshold:(double)confidenceThreshold error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Batch prediction
    @param inputArray array of SCFootTrackingModelInput instances to obtain predictions from
    @param options prediction options
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
    @return the predictions as NSArray<SCFootTrackingModelOutput *>
*/
- (nullable NSArray<SCFootTrackingModelOutput *> *)predictionsFromInputs:(NSArray<SCFootTrackingModelInput*> *)inputArray options:(MLPredictionOptions *)options error:(NSError * _Nullable __autoreleasing * _Nullable)error;
@end

NS_ASSUME_NONNULL_END
