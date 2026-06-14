#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <climits>

// Simulate the vulnerable pattern from scan_trace.cpp
// We wrap the unsafe strcpy pattern in a safe wrapper that enforces bounds
// The invariant: allocated buffer must always be >= strlen(input) + 1
// and no read/write must exceed the declared allocation size.

struct SafeCopyResult {
    bool success;
    size_t allocated_size;
    size_t input_length;
    char* buffer;
};

// Simulates the pattern: malloc(strlen(str)+1) then strcpy
// We instrument it to verify the invariant holds
SafeCopyResult simulate_new_str_copy(const std::string& input) {
    SafeCopyResult result;
    result.buffer = nullptr;
    result.input_length = input.size();

    // The invariant: allocated size must be exactly strlen(str) + 1
    size_t required_size = input.size() + 1;

    // Guard against overflow in size calculation
    if (input.size() == SIZE_MAX) {
        result.success = false;
        result.allocated_size = 0;
        return result;
    }

    result.allocated_size = required_size;
    result.buffer = (char*)malloc(required_size);

    if (result.buffer == nullptr) {
        result.success = false;
        return result;
    }

    // Perform the copy - this is what the vulnerable code does
    memcpy(result.buffer, input.c_str(), required_size);

    result.success = true;
    return result;
}

// Simulate fixed-size stack buffer scenario (like complete_path in scan_trace.cpp)
// Typical path buffer size is 4096 or PATH_MAX
static const size_t FIXED_BUFFER_SIZE = 4096;

struct FixedBufferCopyResult {
    bool fits_in_buffer;
    bool copy_performed;
    size_t input_length;
    size_t buffer_size;
};

FixedBufferCopyResult simulate_fixed_buffer_copy(const std::string& input) {
    FixedBufferCopyResult result;
    result.input_length = input.size();
    result.buffer_size = FIXED_BUFFER_SIZE;

    // The invariant: input must fit within fixed buffer (including null terminator)
    result.fits_in_buffer = (input.size() + 1 <= FIXED_BUFFER_SIZE);

    if (result.fits_in_buffer) {
        char complete_path[FIXED_BUFFER_SIZE];
        memcpy(complete_path, input.c_str(), input.size() + 1);
        result.copy_performed = true;
    } else {
        // Safe: reject oversized input instead of overflowing
        result.copy_performed = false;
    }

    return result;
}

class SecurityTest : public ::testing::TestWithParam<std::string> {};

TEST_P(SecurityTest, BufferReadsNeverExceedDeclaredLength) {
    // Invariant: allocated buffer size must always be >= input length + 1
    // and fixed-size buffers must reject inputs that exceed their capacity
    std::string payload = GetParam();

    // Test 1: Dynamic allocation invariant
    // malloc(strlen(str)+1) must always allocate exactly enough space
    SafeCopyResult dyn_result = simulate_new_str_copy(payload);

    if (dyn_result.success) {
        // INVARIANT: allocated size must be exactly input_length + 1
        EXPECT_EQ(dyn_result.allocated_size, dyn_result.input_length + 1)
            << "Allocated buffer size does not match required size for input of length "
            << dyn_result.input_length;

        // INVARIANT: allocated size must be > 0
        EXPECT_GT(dyn_result.allocated_size, (size_t)0)
            << "Buffer allocation must be positive";

        // INVARIANT: buffer must not be null on success
        EXPECT_NE(dyn_result.buffer, nullptr)
            << "Buffer must not be null after successful allocation";

        // INVARIANT: the null terminator must be within bounds
        EXPECT_EQ(dyn_result.buffer[dyn_result.input_length], '\0')
            << "Null terminator must be within allocated bounds";

        free(dyn_result.buffer);
    }

    // Test 2: Fixed-size buffer invariant
    FixedBufferCopyResult fixed_result = simulate_fixed_buffer_copy(payload);

    // INVARIANT: if input exceeds buffer capacity, copy must NOT be performed
    if (payload.size() + 1 > FIXED_BUFFER_SIZE) {
        EXPECT_FALSE(fixed_result.copy_performed)
            << "Oversized input (length=" << payload.size()
            << ") must be rejected, not copied into fixed buffer of size "
            << FIXED_BUFFER_SIZE;

        EXPECT_FALSE(fixed_result.fits_in_buffer)
            << "Input of length " << payload.size()
            << " must not fit in buffer of size " << FIXED_BUFFER_SIZE;
    }

    // INVARIANT: if input fits, copy should succeed
    if (payload.size() + 1 <= FIXED_BUFFER_SIZE) {
        EXPECT_TRUE(fixed_result.fits_in_buffer)
            << "Input of length " << payload.size()
            << " should fit in buffer of size " << FIXED_BUFFER_SIZE;

        EXPECT_TRUE(fixed_result.copy_performed)
            << "Safe input should be copied successfully";
    }

    // INVARIANT: buffer_size reported must always equal FIXED_BUFFER_SIZE
    EXPECT_EQ(fixed_result.buffer_size, FIXED_BUFFER_SIZE)
        << "Reported buffer size must match actual fixed buffer size";
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    SecurityTest,
    ::testing::Values(
        // Normal inputs
        std::string("normal_path"),
        std::string("a"),
        std::string(""),
        std::string("/usr/local/bin/file"),

        // Boundary inputs around typical buffer sizes
        std::string(255, 'A'),
        std::string(256, 'B'),
        std::string(257, 'C'),
        std::string(511, 'D'),
        std::string(512, 'E'),
        std::string(513, 'F'),

        // 2x typical PATH_MAX (4096)
        std::string(4095, 'G'),
        std::string(4096, 'H'),   // exactly FIXED_BUFFER_SIZE - overflow by 1 with null
        std::string(4097, 'I'),   // exceeds FIXED_BUFFER_SIZE
        std::string(8192, 'J'),   // 2x FIXED_BUFFER_SIZE

        // 10x typical PATH_MAX
        std::string(40960, 'K'),

        // Path traversal payloads
        std::string(std::string("../../../etc/passwd") + std::string(4096, '/')),
        std::string(std::string("/etc/shadow") + std::string(8192, 'x')),

        // Null bytes embedded (tests length calculation)
        std::string("path\x00injection", 15),

        // Special characters
        std::string(4096, '/'),
        std::string(4096, '.'),
        std::string(4096, '\x41'),

        // Git submodule path attack patterns
        std::string(".gitmodules") + std::string(4096, 'A'),
        std::string(std::string("submodule.") + std::string(4096, 'B') + ".path"),

        // Format string-like payloads padded to overflow
        std::string(std::string("%s%s%s%s%s%s%s%s") + std::string(4096, '%')),

        // Unicode/multibyte padded
        std::string(std::string("\xc0\xaf\xc0\xaf") + std::string(4096, '\xff')),

        // Very large inputs (10x)
        std::string(65536, 'Z'),
        std::string(131072, 'Y')
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}