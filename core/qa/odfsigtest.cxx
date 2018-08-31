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

TEST(OdfsigTest, testParseSignaturesEmptyStream)
{
    // ZipVerifier::parseSignatures(), empty signatures stream.
    std::unique_ptr<odfsig::Verifier> verifier(odfsig::Verifier::create());
    ASSERT_TRUE(verifier->openZip("core/qa/data/empty-stream.odt"));
    ASSERT_TRUE(verifier->parseSignatures());
    ASSERT_TRUE(verifier->getSignatures().empty());
}

TEST(OdfsigTest, testParseSignaturesNoStream)
{
    // ZipVerifier::parseSignatures(), no signatures stream.
    std::unique_ptr<odfsig::Verifier> verifier(odfsig::Verifier::create());
    ASSERT_TRUE(verifier->openZip("core/qa/data/no-stream.odt"));
    ASSERT_TRUE(verifier->parseSignatures());
    ASSERT_TRUE(verifier->getSignatures().empty());
}

TEST(OdfsigTest, testGood)
{
    // XmlSignature::verify(), positive testing.
    std::unique_ptr<odfsig::Verifier> verifier(odfsig::Verifier::create());
    ASSERT_TRUE(verifier->openZip("core/qa/data/good.odt"));
    ASSERT_TRUE(verifier->parseSignatures());
    std::vector<std::unique_ptr<odfsig::Signature>>& signatures =
        verifier->getSignatures();
    ASSERT_EQ(1, signatures.size());
    ASSERT_TRUE(signatures[0]->verify());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
