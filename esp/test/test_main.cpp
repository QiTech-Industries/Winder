#include <unity.h>

#include <helpers.cpp>

// run tests with `pio test -e native` in console

void test_putNumberInRange() {
    TEST_ASSERT_EQUAL_UINT16(20, putNumberInRange(20, 15, 45));
    TEST_ASSERT_EQUAL_UINT16(15, putNumberInRange(8, 15, 45));
    TEST_ASSERT_EQUAL_UINT16(45, putNumberInRange(90, 15, 45));
}

void test_speedUsToRpm() {
    TEST_ASSERT_EQUAL_FLOAT(93.75, speedUsToRpm(100, 6400));
    TEST_ASSERT_EQUAL_FLOAT(446.4286, speedUsToRpm(21, 6400));
    TEST_ASSERT_EQUAL_FLOAT(11.3311, speedUsToRpm(1050, 5043));
    TEST_ASSERT_EQUAL_FLOAT(0, speedUsToRpm(0, 5043));
}

void test_speedRpmToUs() {
    TEST_ASSERT_EQUAL_UINT32(937, speedRpmToUs(10, 6400));
    TEST_ASSERT_EQUAL_UINT32(9375, speedRpmToUs(1, 6400));
    TEST_ASSERT_EQUAL_UINT32(937, speedRpmToUs(-10, 6400));
    TEST_ASSERT_EQUAL_UINT32(0, speedRpmToUs(0, 0));
}

void test_indexOfClosestNumberInSortedArray() {
    uint16_t arr[40]{1,     4,     5,     6,     7,     8,     12,    15,
                     17,    34,    37,    49,    109,   203,   204,   206,
                     502,   553,   670,   750,   867,   1040,  1041,  1042,
                     1078,  2050,  6070,  6771,  6800,  6999,  10000, 10001,
                     10002, 10050, 10111, 11111, 11112, 11113, 11114, 11160};
    TEST_ASSERT_EQUAL_UINT16(
        8, arr[indexOfClosestNumberInSortedArray(7.8, arr, 40)]);
    TEST_ASSERT_EQUAL_UINT16(
        1, arr[indexOfClosestNumberInSortedArray(0, arr, 40)]);
    TEST_ASSERT_EQUAL_UINT16(
        11160, arr[indexOfClosestNumberInSortedArray(15000, arr, 40)]);
    TEST_ASSERT_EQUAL_UINT16(
        6999, arr[indexOfClosestNumberInSortedArray(6999.5, arr, 40)]);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_putNumberInRange);
    RUN_TEST(test_speedUsToRpm);
    RUN_TEST(test_speedRpmToUs);
    RUN_TEST(test_indexOfClosestNumberInSortedArray);

    UNITY_END();
}