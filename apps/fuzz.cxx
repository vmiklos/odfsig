/*
 * Copyright 2018 Miklos Vajna
 *
 * SPDX-License-Identifier: MIT
 */

#include <libxml/xmlerror.h>

#include <odfsig/lib.hxx>

/// Error handler to avoid spam of error messages from libxml parser.
void ignore(void* /*ctx*/, const char* /*msg*/, ...) {}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    std::unique_ptr<odfsig::Verifier> verifier(
        odfsig::Verifier::create(std::string()));
    verifier->setInsecure(true);

    xmlSetGenericErrorFunc(nullptr, &ignore);

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
