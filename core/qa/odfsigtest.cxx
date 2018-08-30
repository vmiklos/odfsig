/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <gtest/gtest.h>

#include "odfsig.hxx"

TEST(OdfsigTest, testOpenZip)
{
    // ZipVerifier::openZip() negative testing.
    std::unique_ptr<odfsig::Verifier> verifier(odfsig::Verifier::create());
    ASSERT_EQ(false, verifier->openZip("non-existent.odt"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
