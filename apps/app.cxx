/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <iostream>

#include <odfsig/lib.hxx>

namespace
{
bool printSignatures(
    const std::string& odfPath,
    std::vector<std::unique_ptr<odfsig::Signature>>& signatures)
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
        odfsig::Signature* signature = signatures[signatureIndex].get();
        std::cerr << "Signature #" << (signatureIndex + 1) << ":" << std::endl;

        std::string subjectName = signature->getSubjectName();
        if (!subjectName.empty())
            std::cerr << "  - Signing Certificate Subject Name: " << subjectName
                      << std::endl;

        std::string date = signature->getDate();
        if (!date.empty())
            std::cerr << "  - Signing Date: " << date << std::endl;

        std::string method = signature->getMethod();
        if (!method.empty())
            std::cerr << "  - Signature Method Algorithm: " << method
                      << std::endl;

        std::string type = signature->getType();
        if (!type.empty())
            std::cerr << "  - Signature Type: " << type << std::endl;

        if (!signature->verify())
        {
            if (!signature->getErrorString().empty())
            {
                std::cerr << "Failed to verify signature: "
                          << signature->getErrorString() << "." << std::endl;
                return false;
            }

            std::cerr << "  - Signature Verification: Failed." << std::endl;
            return false;
        }

        std::cerr << "  - Signature Verification: Succeeded." << std::endl;
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

    std::unique_ptr<odfsig::Verifier> verifier(odfsig::Verifier::create());

    std::string odfPath(argv[1]);
    if (!verifier->openZip(odfPath))
    {
        std::cerr << "Can't open zip archive '" << odfPath
                  << "': " << verifier->getErrorString() << "." << std::endl;
        return 1;
    }

    if (!verifier->parseSignatures())
    {
        std::cerr << "Failed to parse signatures: "
                  << verifier->getErrorString() << "." << std::endl;
        return 1;
    }

    if (!printSignatures(odfPath, verifier->getSignatures()))
        return 1;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
