/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <iostream>

#include "odfsig.hxx"

namespace
{
bool printSignatures(const std::string& odfPath,
                     std::vector<odfsig::Signature>& signatures)
{
    if (signatures.empty())
    {
        std::cerr << "File '" << odfPath << "' does not contain any signatures."
                  << std::endl;
        return false;
    }

    std::cerr << "Digital Signature Info of: " << odfPath << std::endl;
    for (size_t signatureIndex = 0; signatureIndex < signatures.size();
         ++signatureIndex)
    {
        odfsig::Signature& signature = signatures[signatureIndex];
        std::cerr << "Signature #" << (signatureIndex + 1) << ":" << std::endl;
        if (!signature.verify())
        {
            if (!signature.getErrorString().empty())
            {
                std::cerr << "Failed to verify signature: "
                          << signature.getErrorString() << "." << std::endl;
                return false;
            }

            std::cerr << "  - Signature Validation: Failed." << std::endl;
            return false;
        }

        std::cerr << "  - Signature Validation: Succeeded." << std::endl;
        std::cerr << "  - Certificate Validation: Not Implemented."
                  << std::endl;
    }

    return true;
}
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <ODF-file>" << std::endl;
        return 1;
    }

    odfsig::Verifier verifier;

    std::string odfPath(argv[1]);
    if (!verifier.openZip(odfPath))
    {
        std::cerr << "Can't open zip archive '" << odfPath
                  << "': " << verifier.getErrorString() << "." << std::endl;
        return 1;
    }

    if (!verifier.parseSignatures())
    {
        std::cerr << "Failed to parse signatures: " << verifier.getErrorString()
                  << "." << std::endl;
        return 1;
    }

    if (!printSignatures(odfPath, verifier.getSignatures()))
        return 1;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
