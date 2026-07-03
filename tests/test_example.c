#include <prompteditor/example.h>
#include <prompteditor/version.h>

#include "test_assert.h"

#include <limits.h>
#include <stdio.h>

static int test_version(void) {
    PromptEditorVersion version = prompteditor_version();
    if (!PROMPTEDITOR_EXPECT_INT_EQ(version.major, PROMPTEDITOR_VERSION_MAJOR)) {
        return 1;
    }
    if (!PROMPTEDITOR_EXPECT_INT_EQ(version.minor, PROMPTEDITOR_VERSION_MINOR)) {
        return 1;
    }
    if (!PROMPTEDITOR_EXPECT_INT_EQ(version.patch, PROMPTEDITOR_VERSION_PATCH)) {
        return 1;
    }
    return 0;
}

static int test_checked_add(void) {
    int value = 0;

    if (!PROMPTEDITOR_EXPECT_TRUE(prompteditor_add_checked(2, 3, &value))) {
        return 1;
    }
    if (!PROMPTEDITOR_EXPECT_INT_EQ(value, 5)) {
        return 1;
    }
    if (!PROMPTEDITOR_EXPECT_TRUE(!prompteditor_add_checked(INT_MAX, 1, &value))) {
        return 1;
    }
    if (!PROMPTEDITOR_EXPECT_TRUE(!prompteditor_add_checked(INT_MIN, -1, &value))) {
        return 1;
    }
    if (!PROMPTEDITOR_EXPECT_TRUE(!prompteditor_add_checked(1, 1, NULL))) {
        return 1;
    }
    return 0;
}

int main(void) {
    if (test_version()) {
        return 1;
    }
    if (test_checked_add()) {
        return 1;
    }

    printf("[PASS] example tests\n");
    return 0;
}
