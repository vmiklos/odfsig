/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <odfsig/lib.hxx>

namespace
{
class NullBuffer : public std::streambuf
{
  public:
    int overflow(int c) { return c; }
};
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    std::unique_ptr<odfsig::Verifier> verifier(
        odfsig::Verifier::create(std::string()));
    verifier->setInsecure(true);

    NullBuffer nullBuffer;
    std::ostream nullStream(&nullBuffer);
    verifier->setLogger(nullStream);

    if (!verifier->openZipMemory(data, size))
    {
        return 0;
    }

    if (!verifier->parseSignatures())
    {
        return 0;
    }

    for (const auto& signature : verifier->getSignatures())
    {
        signature->verify();
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
