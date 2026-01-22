/*
 * Copyright 2018 Miklos Vajna
 *
 * SPDX-License-Identifier: MIT
 */

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <odfsig/lib.hxx>
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
        ostream << "File '" << odfPath << "' does not contain any signatures.\n";
        return false;
    }

    ostream << "Digital Signature Info of: " << odfPath << '\n';
    for (size_t signatureIndex = 0; signatureIndex < signatures.size();
         ++signatureIndex)
    {
        odfsig::Signature* signature = signatures[signatureIndex].get();
        ostream << "Signature #" << (signatureIndex + 1) << ":\n";

        std::string subjectName = signature->getSubjectName();
        if (!subjectName.empty())
        {
            ostream << "  - Signing Certificate Subject Name: " << subjectName
                    << '\n';
        }

        std::string date = signature->getDate();
        if (!date.empty())
        {
            ostream << "  - Signing Date: " << date << '\n';
        }

        std::string method = signature->getMethod();
        if (!method.empty())
        {
            ostream << "  - Signature Method Algorithm: " << method
                    << '\n';
        }

        std::string type = signature->getType();
        if (!type.empty())
        {
            ostream << "  - Signature Type: " << type << '\n';
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
            ostream << '\n';
        }

        if (signedStreams == streams)
        {
            ostream << "  - Total document signed.\n";
        }
        else
        {
            ostream << "  - Only part of the document is signed.\n";
            return false;
        }

        if (!signature->verify())
        {
            if (!signature->getErrorString().empty())
            {
                ostream << "Failed to verify signature: "
                        << signature->getErrorString() << ".\n";
                return false;
            }

            ostream << "  - Signature Verification: Failed.\n";
            return false;
        }

        ostream << "  - Signature Verification: Succeeded.\n";

        if (type == "XAdES")
        {
            if (!signature->verifyXAdES())
            {
                ostream << "  - Certificate Hash Verification: Failed.\n";
                return false;
            }
            ostream << "  - Certificate Hash Verification: Succeeded.\n";
        }
    }

    return true;
}

struct Options
{
    std::vector<std::string> _odfPaths;
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
    bool first = true;
    for (const auto& arg : args)
    {
        if (first)
        {
            first = false;
            continue;
        }

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
        else if (argString.starts_with("--"))
        {
            ostream << "Error: unrecognized argument: " << argString << '\n';
            return false;
        }
        else
        {
            options._odfPaths.push_back(argString);
        }
    }

    return true;
}

void usage(const std::string& self, std::ostream& ostream)
{
    ostream << "Usage: " << self << " [options] <ODF-file>\n";
    ostream << "--trusted-der <file>: load trusted (root) certificate from "
               "DER file <file>\n";
    ostream << "--insecure: do not validate certificates\n";
}
} // namespace

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
#ifdef ODFSIG_VERSION_GIT
        ostream << "-g" ODFSIG_VERSION_GIT;
#endif
        ostream << '\n';
        return 0;
    }

    std::string cryptoConfig;
    const char* home = getenv("HOME");
    if (home != nullptr)
    {
        cryptoConfig = home;
    }

    for (const auto& odfPath : options._odfPaths)
    {
        std::unique_ptr<odfsig::Verifier> verifier(
            odfsig::Verifier::create(cryptoConfig));
        verifier->setTrustedDers(options._trustedDers);
        verifier->setInsecure(options._insecure);

        if (!verifier->openZip(odfPath))
        {
            ostream << "Can't open zip archive '" << odfPath
                    << "': " << verifier->getErrorString() << ".\n";
            return 1;
        }

        if (!verifier->parseSignatures())
        {
            ostream << "Failed to parse signatures: "
                    << verifier->getErrorString() << ".\n";
            return 1;
        }

        std::set<std::string> streams = verifier->getStreams();
        if (!printSignatures(odfPath, streams, verifier->getSignatures(),
                             ostream))
        {
            return 1;
        }
    }

    return 0;
}
} // namespace odfsig

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
