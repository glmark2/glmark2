#include "pixels-hash.h"
#include "options.h"
#include <chrono>
#include <libxxhash/xxhash.h>
#include <sstream>
#include <iomanip>

size_t PixelHash::buffersize = 0;
std::unique_ptr<char[]> PixelHash::pixelsBuffer = nullptr;
inline double durationInMilliseconds(const std::chrono::high_resolution_clock::time_point &start,
                                      const std::chrono::high_resolution_clock::time_point &end) {
    return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start).count();
}
void PixelHash::calcXxhash(std::string sceneName, int frameNum)
{
    XXH128_hash_t hash;
    std::stringstream hashStream;

    // only calc every "step" number of frames. This is to generate suffucient stress before retreive
    // the data and calc, which puts the GPU idle(relax) 
    if ( frameNum%Options::step )
        return;

    auto start = std::chrono::high_resolution_clock::now();
    glReadPixels(0, 0, Options::size.first, Options::size.first, GL_RGBA, GL_UNSIGNED_BYTE, pixelsBuffer.get());
    auto afterRead = std::chrono::high_resolution_clock::now();

    size_t sectionSize = buffersize / 4;
    for (int i = 0; i < 4; ++i) {
        int startIdx = i * sectionSize;
        int endIdx = (i + 1) * sectionSize;
        hash = XXH3_128bits(pixelsBuffer.get() + startIdx, endIdx - startIdx);
        hashStream << std::hex 
           << std::setfill('0') << std::setw(16) << hash.high64
           << std::setfill('0') << std::setw(16) << hash.low64;
        if (i != 3) 
            hashStream << "_";
    }
    auto afterCalc = std::chrono::high_resolution_clock::now();

    double readTime = durationInMilliseconds(start, afterRead);
    double calcTime = durationInMilliseconds(afterRead, afterCalc);
    if ( frameNum / Options::step == 1)
        printf("\n");
    printf("%s:%d:%s:(%.1f+%.1f)ms\n",
           ("scenehash:"+sceneName).c_str(), frameNum, hashStream.str().c_str(), readTime, calcTime);
}

bool PixelHash::allocatePixelsBuffer()
{
    buffersize = Options::size.first * Options::size.second * 4;
    try {
        pixelsBuffer = std::make_unique<char[]>(buffersize);
        if (Options::show_debug)
            printf("Buffer allocated successfully with %zu bytes\n", buffersize);
    }
    catch (const std::bad_alloc& e) {
        printf("Cannot allocate enough memory to calculate hash for window size %d x %d\n",
            Options::size.first, Options::size.second);
        return false;
    }
    return true;
}
