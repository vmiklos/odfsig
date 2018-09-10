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
    const std::string& odfPath, const std::set<std::string>& streams,
    std::vector<std::unique_ptr<odfsig::Signature>>& signatures,
    std::ostream& ostream)
{
    if (signatures.empty())
    {
        ostream << "File '" << odfPath << "' does not contain any signatures."
                << std::endl;
        return false;
    }

    ostream << "Digital Signature Info of: " << odfPath << std::endl;
    for (size_t signatureIndex = 0; signatureIndex < signatures.size();
         ++signatureIndex)
    {
        odfsig::Signature* signature = signatures[signatureIndex].get();
        ostream << "Signature #" << (signatureIndex + 1) << ":" << std::endl;

        std::string subjectName = signature->getSubjectName();
        if (!subjectName.empty())
            ostream << "  - Signing Certificate Subject Name: " << subjectName
                    << std::endl;

        std::string date = signature->getDate();
        if (!date.empty())
            ostream << "  - Signing Date: " << date << std::endl;

        std::string method = signature->getMethod();
        if (!method.empty())
            ostream << "  - Signature Method Algorithm: " << method
                    << std::endl;

        std::string type = signature->getType();
        if (!type.empty())
            ostream << "  - Signature Type: " << type << std::endl;

        std::set<std::string> signedStreams = signature->getSignedStreams();
        if (!signedStreams.empty())
        {
            ostream << "  - Signed Streams: ";
            bool first = true;
            for (const auto& signedStream : signedStreams)
            {
                if (first)
                    first = false;
                else
                    ostream << ", ";
                ostream << signedStream;
            }
            ostream << std::endl;
        }

        if (signedStreams == streams)
            ostream << "  - Total document signed." << std::endl;
        else
        {
            ostream << "  - Only part of the document is signed." << std::endl;
            return false;
        }

        if (!signature->verify())
        {
            if (!signature->getErrorString().empty())
            {
                ostream << "Failed to verify signature: "
                        << signature->getErrorString() << "." << std::endl;
                return false;
            }

            ostream << "  - Signature Verification: Failed." << std::endl;
            return false;
        }

        ostream << "  - Signature Verification: Succeeded." << std::endl;
    }

    return true;
}
}

namespace odfsig
{
int main(int argc, char* argv[], std::ostream& ostream)
{
    if (argc < 2)
    {
        ostream << "Usage: " << argv[0] << " <ODF-file>" << std::endl;
        return 1;
    }

    std::string cryptoConfig;
    const char* home = getenv("HOME");
    if (home)
        cryptoConfig = home;
    std::unique_ptr<odfsig::Verifier> verifier(
        odfsig::Verifier::create(cryptoConfig));

    std::string odfPath(argv[1]);
    if (!verifier->openZip(odfPath))
    {
        ostream << "Can't open zip archive '" << odfPath
                << "': " << verifier->getErrorString() << "." << std::endl;
        return 1;
    }

    if (!verifier->parseSignatures())
    {
        ostream << "Failed to parse signatures: " << verifier->getErrorString()
                << "." << std::endl;
        return 1;
    }

    std::set<std::string> streams = verifier->getStreams();
    if (!printSignatures(odfPath, streams, verifier->getSignatures(), ostream))
        return 1;

    return 0;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
