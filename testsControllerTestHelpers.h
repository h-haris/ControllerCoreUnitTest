// ControllerTestHelpers.h
// Helper macros and utilities for Controller Core Unit Tests
// Migrated from MinUnit to Google Test

#ifndef CONTROLLER_TEST_HELPERS_H
#define CONTROLLER_TEST_HELPERS_H

#include <cmath>

// ============================================================================
// Floating Point Comparison
// ============================================================================

#define FL_EPSILON (0.00001)

#define FL_EQUAL(result, expected) \
    (std::fabs((result) - (expected)) < FL_EPSILON)

// ============================================================================
// Platform-Specific Expectations
// ============================================================================

// For controller comparison on different platforms
#ifdef __MWERKS__
    #define EXPECT_PLATFORM_CONTROLLER_EQUALITY(client, driver) \
        EXPECT_EQ(client, driver) << "Controllers should be equal (CodeWarrior)"
#else
    #define EXPECT_PLATFORM_CONTROLLER_EQUALITY(client, driver) \
        EXPECT_EQ(kCFCompareEqualTo, \
                  CFStringCompare((CFStringRef)client, (CFStringRef)driver, \
                                  kCFCompareCaseInsensitive)) \
            << "Controllers should be equal (modern)"
#endif

// ============================================================================
// Status Checks
// ============================================================================

#define EXPECT_Q3_SUCCESS(status) \
    EXPECT_EQ(kQ3Success, (status)) << "Expected kQ3Success"

#define EXPECT_Q3_FAILURE(status) \
    EXPECT_EQ(kQ3Failure, (status)) << "Expected kQ3Failure"

// ============================================================================
// 3D Math Assertions
// ============================================================================

#define EXPECT_POINT3D_EQ(p, x, y, z) \
    do { \
        EXPECT_FLOAT_EQ((x), (p).x) << "Point3D.x mismatch"; \
        EXPECT_FLOAT_EQ((y), (p).y) << "Point3D.y mismatch"; \
        EXPECT_FLOAT_EQ((z), (p).z) << "Point3D.z mismatch"; \
    } while(0)

#define EXPECT_VECTOR3D_EQ(v, x, y, z) \
    do { \
        EXPECT_FLOAT_EQ((x), (v).x) << "Vector3D.x mismatch"; \
        EXPECT_FLOAT_EQ((y), (v).y) << "Vector3D.y mismatch"; \
        EXPECT_FLOAT_EQ((z), (v).z) << "Vector3D.z mismatch"; \
    } while(0)

#define EXPECT_QUATERNION_EQ(q, w, x, y, z) \
    do { \
        EXPECT_FLOAT_EQ((w), (q).w) << "Quaternion.w mismatch"; \
        EXPECT_FLOAT_EQ((x), (q).x) << "Quaternion.x mismatch"; \
        EXPECT_FLOAT_EQ((y), (q).y) << "Quaternion.y mismatch"; \
        EXPECT_FLOAT_EQ((z), (q).z) << "Quaternion.z mismatch"; \
    } while(0)

#define EXPECT_QUATERNION_NEAR(q, w, x, y, z) \
    do { \
        EXPECT_TRUE(FL_EQUAL((q).w, (w))) \
            << "Quaternion.w expected " << (w) << ", got " << (q).w; \
        EXPECT_TRUE(FL_EQUAL((q).x, (x))) \
            << "Quaternion.x expected " << (x) << ", got " << (q).x; \
        EXPECT_TRUE(FL_EQUAL((q).y, (y))) \
            << "Quaternion.y expected " << (y) << ", got " << (q).y; \
        EXPECT_TRUE(FL_EQUAL((q).z, (z))) \
            << "Quaternion.z expected " << (z) << ", got " << (q).z; \
    } while(0)

#endif // CONTROLLER_TEST_HELPERS_H
