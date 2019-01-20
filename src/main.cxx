/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <odfsig/lib.hxx>
#include <odfsig/string.hxx>
#include <odfsig/version.hxx>

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
        {
            ostream << "  - Signing Certificate Subject Name: " << subjectName
                    << std::endl;
        }

        std::string date = signature->getDate();
        if (!date.empty())
        {
            ostream << "  - Signing Date: " << date << std::endl;
        }

        std::string method = signature->getMethod();
        if (!method.empty())
        {
            ostream << "  - Signature Method Algorithm: " << method
                    << std::endl;
        }

        std::string type = signature->getType();
        if (!type.empty())
        {
            ostream << "  - Signature Type: " << type << std::endl;
        }

        std::set<std::string> signedStreams = signature->getSignedStreams();
        if (!signedStreams.empty())
        {
            ostream << "  - Signed Streams: ";
            bool first = true;
            for (const auto& signedStream : signedStreams)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    ostream << ", ";
                }
                ostream << signedStream;
            }
            ostream << std::endl;
        }

        if (signedStreams == streams)
        {
            ostream << "  - Total document signed." << std::endl;
        }
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

        if (type == "XAdES")
        {
            if (!signature->verifyXAdES())
            {
                ostream << "  - Certificate Hash Verification: Failed."
                        << std::endl;
                return false;
            }
            ostream << "  - Certificate Hash Verification: Succeeded."
                    << std::endl;
        }
    }

    return true;
}

struct Options
{
    std::string _odfPath;
    std::vector<std::string> _trustedDers;
    bool _insecure = false;
    bool _help = false;
    bool _version = false;
};

/// Minimal option parser to avoid Boost.Program_options dependency.
bool parseOptions(const std::vector<const char*>& args, Options& options,
                  std::ostream& ostream)
{
    bool inTrustedDer = false;
    for (const auto& arg : args)
    {
        std::string argString(arg);
        if (argString == "--trusted-der")
        {
            inTrustedDer = true;
        }
        else if (inTrustedDer)
        {
            inTrustedDer = false;
            options._trustedDers.push_back(argString);
        }
        else if (argString == "--insecure")
        {
            options._insecure = true;
        }
        else if (argString == "--help")
        {
            options._help = true;
        }
        else if (argString == "--version")
        {
            options._version = true;
        }
        else if (odfsig::starts_with(argString, "--"))
        {
            ostream << "Error: unrecognized argument: " << argString
                    << std::endl;
            return false;
        }
        else
        {
            options._odfPath = argString;
        }
    }

    return true;
}

void usage(const std::string& self, std::ostream& ostream)
{
    ostream << "Usage: " << self << " [options] <ODF-file>" << std::endl;
    ostream << "--trusted-der <file>: load trusted (root) certificate from "
               "DER file <file>"
            << std::endl;
    ostream << "--insecure: do not validate certificates" << std::endl;
}
}

namespace odfsig
{
int main(const std::vector<const char*>& args, std::ostream& ostream)
{
    if (args.size() < 2)
    {
        usage(args[0], ostream);
        return 1;
    }

    Options options;
    if (!parseOptions(args, options, ostream))
    {
        return 2;
    }
    if (options._help)
    {
        usage(args[0], ostream);
        return 0;
    }

    if (options._version)
    {
        ostream << "odfsig version " << ODFSIG_VERSION_MAJOR << "."
                << ODFSIG_VERSION_MINOR;
        if (std::strlen(ODFSIG_VERSION_GIT) > 0)
        {
            ostream << "-g" ODFSIG_VERSION_GIT;
        }
        ostream << std::endl;
        return 0;
    }

    std::string cryptoConfig;
    const char* home = getenv("HOME");
    if (home != nullptr)
    {
        cryptoConfig = home;
    }
    std::unique_ptr<odfsig::Verifier> verifier(
        odfsig::Verifier::create(cryptoConfig));
    verifier->setTrustedDers(options._trustedDers);
    verifier->setInsecure(options._insecure);

    if (!verifier->openZip(options._odfPath))
    {
        ostream << "Can't open zip archive '" << options._odfPath
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
    if (!printSignatures(options._odfPath, streams, verifier->getSignatures(),
                         ostream))
    {
        return 1;
    }

    return 0;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
