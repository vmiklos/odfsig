#ifndef _HAD_ODFSIG_H
#define _HAD_ODFSIG_H
/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
#include <string>
#include <vector>

#include <libxml/parser.h>
#include <xmlsec/xmldsig.h>
#include <xmlsec/xmlsec.h>
#include <zip.h>

namespace std
{
template <> struct default_delete<zip_t>
{
    void operator()(zip_t* ptr) { zip_close(ptr); }
};
template <> struct default_delete<zip_file_t>
{
    void operator()(zip_file_t* ptr) { zip_fclose(ptr); }
};
template <> struct default_delete<xmlDoc>
{
    void operator()(xmlDocPtr ptr) { xmlFreeDoc(ptr); }
};
template <> struct default_delete<xmlSecDSigCtx>
{
    void operator()(xmlSecDSigCtxPtr ptr) { xmlSecDSigCtxDestroy(ptr); }
};
template <> struct default_delete<xmlSecKeysMngr>
{
    void operator()(xmlSecKeysMngrPtr ptr) { xmlSecKeysMngrDestroy(ptr); }
};
}

namespace odfsig
{
class XmlGuard;
class XmlSecGuard;

/// Represents one specific signature in the document.
class Signature
{
  public:
    explicit Signature(xmlNode* signatureNode);

    ~Signature();

    const std::string& getErrorString() const;

    bool verify();

  private:
    std::string _errorString;

    xmlNode* _signatureNode = nullptr;
};

/// Verifies signatures of an ODF document.
class Verifier
{
  public:
    Verifier();

    ~Verifier();

    bool openZip(const std::string& path);

    const std::string& getErrorString() const;

    bool parseSignatures();

    std::vector<Signature>& getSignatures();

  private:
    bool locateSignatures();

    std::unique_ptr<zip_t> _zipArchive;
    std::string _errorString;
    zip_int64_t _signaturesZipIndex = 0;
    std::unique_ptr<XmlGuard> _xmlGuard;
    std::unique_ptr<XmlSecGuard> _xmlSecGuard;
    std::unique_ptr<zip_file_t> _zipFile;
    std::vector<char> _signaturesBytes;
    std::unique_ptr<xmlDoc> _signaturesDoc;
    std::vector<Signature> _signatures;
};
}
#endif /* _HAD_ZIP_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
