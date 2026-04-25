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

#define EXPECT_POINT3D_EQ(pt, ex, ey, ez) \
    do { \
        EXPECT_FLOAT_EQ((ex), (pt).x) << "Point3D.x mismatch"; \
        EXPECT_FLOAT_EQ((ey), (pt).y) << "Point3D.y mismatch"; \
        EXPECT_FLOAT_EQ((ez), (pt).z) << "Point3D.z mismatch"; \
    } while(0)

#define EXPECT_VECTOR3D_EQ(v, ex, ey, ez) \
    do { \
        EXPECT_FLOAT_EQ((ex), (v).x) << "Vector3D.x mismatch"; \
        EXPECT_FLOAT_EQ((ey), (v).y) << "Vector3D.y mismatch"; \
        EXPECT_FLOAT_EQ((ez), (v).z) << "Vector3D.z mismatch"; \
    } while(0)

#define EXPECT_QUATERNION_EQ(q, ew, ex, ey, ez) \
    do { \
        EXPECT_FLOAT_EQ((ew), (q).w) << "Quaternion.w mismatch"; \
        EXPECT_FLOAT_EQ((ex), (q).x) << "Quaternion.x mismatch"; \
        EXPECT_FLOAT_EQ((ey), (q).y) << "Quaternion.y mismatch"; \
        EXPECT_FLOAT_EQ((ez), (q).z) << "Quaternion.z mismatch"; \
    } while(0)

#define EXPECT_QUATERNION_NEAR(q, ew, ex, ey, ez) \
    do { \
        EXPECT_TRUE(FL_EQUAL((q).w, (ew))) \
            << "Quaternion.w expected " << (ew) << ", got " << (q).w; \
        EXPECT_TRUE(FL_EQUAL((q).x, (ex))) \
            << "Quaternion.x expected " << (ex) << ", got " << (q).x; \
        EXPECT_TRUE(FL_EQUAL((q).y, (ey))) \
            << "Quaternion.y expected " << (ey) << ", got " << (q).y; \
        EXPECT_TRUE(FL_EQUAL((q).z, (ez))) \
            << "Quaternion.z expected " << (ez) << ", got " << (q).z; \
    } while(0)

#endif // CONTROLLER_TEST_HELPERS_H
