#include <arm_neon.h>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include "Para-Hash.h"




#include <ctime>
#include <iomanip>
#include <string>
#include <sstream>

#define tag_size 64
extern void ParaHash_V3(
    const uint8_t* input, uint8_t* tag, uint8x16_t *roundKeys_1, const uint64_t lenght
);

#if defined(__APPLE__)
    #include <sys/types.h>
    #include <sys/sysctl.h>
#elif defined(__linux__)
    #include <fstream>
#endif

std::string get_cpu_name() {

#if defined(__APPLE__)

    char buffer[256];
    size_t size = sizeof(buffer);

    if (sysctlbyname("machdep.cpu.brand_string", buffer, &size, nullptr, 0) == 0) {
        return std::string(buffer);
    }
    return "Unknown Apple CPU";

#elif defined(__linux__)

    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;

    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos ||
            line.find("Model") != std::string::npos) {

            auto pos = line.find(':');
            if (pos != std::string::npos)
                return line.substr(pos + 2);
        }
    }
    return "Unknown ARM CPU";

#else

    return "Unsupported Platform";

#endif
}
void write_tag_hex(std::ofstream& file, const uint8_t* tag, size_t len = 64) {
    file << "Tag: ";
    file << std::hex << std::setfill('0');

    for (size_t i = 0; i < len; i++) {
        file << std::setw(2)
             << static_cast<unsigned>(tag[i]);
    }

    file << std::dec << "\n";
}




std::string get_compiler_info() {
#ifdef __clang__
    return "Clang C++" +
           std::to_string(__clang_major__) + "." +
           std::to_string(__clang_minor__) + "." +
           std::to_string(__clang_patchlevel__);
#elif defined(__GNUC__)
    return "GCC " + std::to_string(__GNUC__);
#else
    return "Unknown Compiler";
#endif
}


std::string get_cpp_standard() {
#if __cplusplus >= 202002L
    return "C++20";
#elif __cplusplus >= 201703L
    return "C++17";
#elif __cplusplus >= 201402L
    return "C++14";
#else
    return "Pre-C++14";
#endif
}

// Compiler barrier to prevent optimization
static inline void clobber_memory() {
    asm volatile("" : : : "memory");
}

std::string get_arch_info() {
    std::string arch;

#if defined(__aarch64__)
    arch = "ARM64";
#else
    arch = "Unknown Arch";
#endif

#if defined(__ARM_NEON)
    arch += " + NEON";
#endif

#if defined(__ARM_FEATURE_CRYPTO)
    arch += " + Crypto";
#endif

    return arch;
}

std::string get_datetime() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}



int main(int argc, char **argv) {

    if ((argc < 2) || (argc > 3)) {
		printf("Usage: ./test [base_frequency_GHz] [output_filename]\n");
		return 0;
	} 
    
    double CPU_FREQ = std::atof(argv[1]) * 1.e9; // Apple M1 ≈ 3.2 GHz
    constexpr int ITER = 100000;
    std::ofstream file(argv[2]);

    file << "Para-Hash-1 Precomputed Keys\n";
    file << get_cpu_name() << "\n";
    file << get_compiler_info() << " (" << get_cpp_standard() << ")\n";
    file << get_arch_info() << "\n";
    file << "Run " << get_datetime() << "\n\n";


    #if defined(__aarch64__) || defined(__arm__)
        file << "Architecture: ARM\n";

    #if defined(__ARM_FEATURE_CRYPTO)
        file << "ARM Crypto Extensions: ENABLED\n";
    #else
        file << "ARM Crypto Extensions: DISABLED\n";
    #endif

    #elif defined(__x86_64__) || defined(__i386__)
        file << "Architecture: x86\n";

        #if defined(__AVX2__)
            file << "AVX2: ENABLED\n";
        #else
            file << "AVX2: DISABLED\n";
        #endif

        #if defined(__AES__)
            file << "AES-NI: ENABLED\n";
        #else
            file << "AES-NI: DISABLED\n";
        #endif

        #if defined(__PCLMUL__)
            file << "PCLMULQDQ: ENABLED\n";
        #else
            file << "PCLMULQDQ: DISABLED\n";
        #endif
    #endif

    // Dummy round keys
    alignas(16) uint8x16_t roundKeys_zero[11];
    alignas(16) uint8x16_t roundKeys_one[11];
    alignas (16) uint8_t key[] = "abcdefghijklmnop";    
    uint8x16_t roundKeys[11];

    for (int i = 0; i < 11; i++) {
        roundKeys_zero[i] = vdupq_n_u8(0x00);
        roundKeys_one[i]  = vdupq_n_u8(0xFF);
    }

    uint8_t output[tag_size] = {};
    
    // file << "Key setup: 483 cycles\n\n";

    std::vector<size_t> sizes = {
        256, 512, 1024, 2048, 4096, 8192, 16384, 32768
    };

    for (size_t size : sizes) {
        std::vector<uint8_t> msg(size);
        for (size_t i = 0; i < size; i++) {
            msg[i] = static_cast<uint8_t>(i);
        }
        
        auto start = std::chrono::steady_clock::now();
        clobber_memory();

        KeyExpansion(key, roundKeys);
        uint8x16_t *obtained_keys = NULL;
        size_t num_blocks = size / 16;
        size_t bytes = num_blocks * sizeof(uint8x16_t);
        if (posix_memalign((void**)&obtained_keys, 16, bytes) != 0) {
            perror("posix_memalign");
            exit(EXIT_FAILURE);
        }

        for (int it = 0; it < ITER; it++) {
            generate_keys(roundKeys, size, obtained_keys);
            asm volatile("" :: "r"(output[0]) : "memory");
            
        }


        auto end = std::chrono::steady_clock::now();

        double seconds =
            std::chrono::duration<double>(end - start).count();

        double total_bytes = double(size) * ITER;
        double cycles = seconds * CPU_FREQ;
        double cpb = cycles / total_bytes;

        file << size << " -- "
             << seconds << " s  ("
             << cpb << " cpb) Key generation\n";


        clobber_memory();

        start = std::chrono::steady_clock::now();

        for (int it = 0; it < ITER; it++) {
            ParaHash_V3(
                msg.data(),
                output,
                obtained_keys,
                size
            );

            // Make output observable
            asm volatile("" :: "r"(output[0]) : "memory");
        }

        end = std::chrono::steady_clock::now();

        seconds =
            std::chrono::duration<double>(end - start).count();

        total_bytes = double(size) * ITER;
        cycles = seconds * CPU_FREQ;
        cpb = cycles / total_bytes;

        file << size << " -- "
             << seconds << " s  ("
             << cpb << " cpb) Para-Hash\n";
        write_tag_hex(file, output,tag_size);

        free(obtained_keys);
    }

    file.close();
    std::cout << "Benchmark completed.\n";
    return 0;
}
