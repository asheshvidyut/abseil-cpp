# BUILD file for container internal tests

# Description:
#   Red-Black Tree implementation for Abseil's ordered containers.

cc_library(
    name = "rbtree",
    hdrs = ["rbtree.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "rbtree_test",
    srcs = ["test_rbtree.cc"],
    data = ["words.txt"],
    deps = [
        ":rbtree",
        "@googletest//:gtest_main",
    ],
)
