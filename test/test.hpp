/*
 * Copyright (c) 2020-2023 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "gtest/gtest.h"

/* EXPECT_THROW but with a match to the exception string */
#define EXPECT_THROW_MATCH(code, type, str) \
EXPECT_THROW({                              \
try {                                       \
    code;                                   \
} catch (type& e) {                         \
    EXPECT_STREQ(str, e.what());            \
    throw e;                                \
}                                           \
}, type)
