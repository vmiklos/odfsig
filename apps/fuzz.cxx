/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <odfsig/lib.hxx>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    std::unique_ptr<odfsig::Verifier> verifier(
        odfsig::Verifier::create(std::string()));
    if (!verifier->openZipMemory(data, size))
    {
        return 1;
    }

    if (!verifier->parseSignatures())
    {
        return 1;
    }

    for (const auto& signature : verifier->getSignatures())
    {
        signature->verify();
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
