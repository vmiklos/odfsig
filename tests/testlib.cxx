/*
 * Copyright 2018 Miklos Vajna
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstddef>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <odfsig/lib.hxx>
#include <odfsig/string.hxx>

TEST(OdfsigTest, testOpenZip)
{
    // ZipVerifier::openZip() negative testing.
    std::unique_ptr<odfsig::Verifier> verifier(
        odfsig::Verifier::create(std::string()));
    verifier->setTrustedDers({"tests/keys/ca-chain.cert.der"});

    ASSERT_EQ(false, verifier->openZip("non-existent.odt"));
}

TEST(OdfsigTest, testParseSignaturesEmptyStream)
{
    // ZipVerifier::parseSignatures(), empty signatures stream.
    std::unique_ptr<odfsig::Verifier> verifier(
        odfsig::Verifier::create(std::string()));
    verifier->setTrustedDers({"tests/keys/ca-chain.cert.der"});

    ASSERT_TRUE(verifier->openZip("tests/data/empty-stream.odt"));
    ASSERT_TRUE(verifier->parseSignatures());
    ASSERT_TRUE(verifier->getSignatures().empty());
}

TEST(OdfsigTest, testParseSignaturesNoStream)
{
    // ZipVerifier::parseSignatures(), no signatures stream.
    std::unique_ptr<odfsig::Verifier> verifier(
        odfsig::Verifier::create(std::string()));
    verifier->setTrustedDers({"tests/keys/ca-chain.cert.der"});

    ASSERT_TRUE(verifier->openZip("tests/data/no-stream.odt"));
    ASSERT_TRUE(verifier->parseSignatures());
    ASSERT_TRUE(verifier->getSignatures().empty());
}

TEST(OdfsigTest, testGood)
{
    // XmlSignature::verify(), positive testing.
    std::unique_ptr<odfsig::Verifier> verifier(
        odfsig::Verifier::create(std::string()));
    verifier->setTrustedDers({"tests/keys/ca-chain.cert.der"});

    ASSERT_TRUE(verifier->openZip("tests/data/good.odt"));
    ASSERT_TRUE(verifier->parseSignatures());
    std::vector<std::unique_ptr<odfsig::Signature>>& signatures =
        verifier->getSignatures();
    ASSERT_EQ(static_cast<size_t>(1), signatures.size());
    std::unique_ptr<odfsig::Signature>& signature = signatures[0];
    ASSERT_TRUE(signature->verify());
    ASSERT_TRUE(signature->verifyXAdES());

    std::string subjectName = signature->getSubjectName();

    // Map CNG result to NSS reference.
    odfsig::replace_all(subjectName, ", S=", ", ST=");
    odfsig::replace_all(subjectName, ", ", ",");

    ASSERT_EQ("CN=odfsig test example alice,O=odfsig test,ST=Budapest,C=HU",
              subjectName);

    ASSERT_EQ("2018-08-31T22:38:51.034635578", signature->getDate());
    ASSERT_EQ("rsa-sha256", signature->getMethod());
    ASSERT_EQ("XAdES", signature->getType());

    std::set<std::string> signedStreams{
        "styles.xml",   "settings.xml",
        "manifest.rdf", "META-INF/manifest.xml",
        "mimetype",     "Thumbnails/thumbnail.png",
        "content.xml",  "meta.xml"};
    ASSERT_EQ(signedStreams, signature->getSignedStreams());

    // All streams are signed.
    std::set<std::string> streams = verifier->getStreams();
    ASSERT_EQ(signedStreams, verifier->getStreams());
}

TEST(OdfsigTest, testBadCertificate)
{
    // Missing setTrustedDers() should result in failure.
    std::unique_ptr<odfsig::Verifier> verifier(
        odfsig::Verifier::create(std::string()));

    ASSERT_TRUE(verifier->openZip("tests/data/good.odt"));
    ASSERT_TRUE(verifier->parseSignatures());
    std::vector<std::unique_ptr<odfsig::Signature>>& signatures =
        verifier->getSignatures();
    ASSERT_EQ(static_cast<size_t>(1), signatures.size());
    ASSERT_FALSE(signatures[0]->verify());
}

TEST(OdfsigTest, testTrustedDerCmdline)
{
    // --trusted-der results in working validation.
    std::vector<const char*> args{"odfsig", "--trusted-der",
                                  "tests/keys/ca-chain.cert.der",
                                  "tests/data/good.odt"};
    std::stringstream stream;
    ASSERT_EQ(0, odfsig::main(args, stream));
}

TEST(OdfsigTest, testInsecureCmdline)
{
    // --insecure results in working validation.
    std::vector<const char*> args{"odfsig", "--insecure",
                                  "tests/data/good.odt"};
    std::stringstream stream;
    ASSERT_EQ(0, odfsig::main(args, stream));
}

TEST(OdfsigTest, testCmdlineHelp)
{
    // --help resulted in an error.
    std::vector<const char*> args{"odfsig", "--help"};
    std::stringstream stream;
    ASSERT_EQ(0, odfsig::main(args, stream));
}

TEST(OdfsigTest, testCmdlineVersion)
{
    // --version resulted in an error.
    std::vector<const char*> args{"odfsig", "--version"};
    std::stringstream stream;
    ASSERT_EQ(0, odfsig::main(args, stream));
}

TEST(OdfsigTest, testCmdlineNoStream)
{
    // No signatures stream.
    std::vector<const char*> args{"odfsig", "tests/data/no-stream.odt"};
    std::stringstream stream;
    ASSERT_EQ(1, odfsig::main(args, stream));
}

TEST(OdfsigTest, testCmdlineBad)
{
    // Bad signatures stream.
    std::vector<const char*> args{"odfsig", "--trusted-der",
                                  "tests/keys/ca-chain.cert.der",
                                  "tests/data/bad.odt"};
    std::stringstream stream;
    ASSERT_EQ(1, odfsig::main(args, stream));
}

TEST(OdfsigTest, testCmdlineBadMulti)
{
    // Bad and good files.
    std::vector<const char*> args{"odfsig", "--trusted-der",
                                  "tests/keys/ca-chain.cert.der",
                                  "tests/data/bad.odt", "tests/data/good.odt"};
    std::stringstream stream;
    // This failed, only the last file was verified.
    ASSERT_EQ(1, odfsig::main(args, stream));
}

TEST(OdfsigTest, testCmdlineBadPath)
{
    // Non-existing path.
    std::vector<const char*> args{"odfsig", "tests/data/asdf.odt"};
    std::stringstream stream;
    ASSERT_EQ(1, odfsig::main(args, stream));
}

TEST(OdfsigTest, testCmdlineNoArgs)
{
    // No arguments.
    std::vector<const char*> args{"odfsig"};
    std::stringstream stream;
    ASSERT_EQ(1, odfsig::main(args, stream));
}

TEST(OdfsigTest, testCmdlineBadArg)
{
    // Bad argument.
    std::vector<const char*> args{"odfsig", "--trusted-derr",
                                  "tests/keys/ca-chain.cert.der",
                                  "tests/data/good.odt"};
    std::stringstream stream;
    // This was 1, bad argument was just ignored silently.
    ASSERT_EQ(2, odfsig::main(args, stream));
}

TEST(OdfsigTest, testCmdlineDirArg)
{
    // Directory argument.
    std::vector<const char*> args{"odfsig", "tests/data/"};
    std::stringstream stream;
    ASSERT_EQ(1, odfsig::main(args, stream));
}

TEST(OdfsigTest, testCmdlineNonZip)
{
    // Input is not a ZIP file: this used to crash.
    std::vector<const char*> args{"odfsig", "tests/data/non-zip.odt"};
    std::stringstream stream;
    ASSERT_EQ(1, odfsig::main(args, stream));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
