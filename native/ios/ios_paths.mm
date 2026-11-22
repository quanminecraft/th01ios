#include "ios_paths.h"
#import <Foundation/Foundation.h>

namespace th01_native {

std::string ios_documents_dir() {
    @autoreleasepool {
        NSArray<NSString*>* paths = NSSearchPathForDirectoriesInDomains(
            NSDocumentDirectory, NSUserDomainMask, YES);
        if (paths.count > 0) {
            return std::string([paths[0] UTF8String]);
        }
        return ".";
    }
}

std::string ios_bundle_assets_dir() {
    @autoreleasepool {
        NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
        if (resourcePath) {
            NSString* assetsPath = [resourcePath stringByAppendingPathComponent:@"assets"];
            return std::string([assetsPath UTF8String]);
        }
        return "";
    }
}

} // namespace th01_native
