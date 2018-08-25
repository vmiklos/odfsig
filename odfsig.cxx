/*
 * Copyright 2018 Miklos Vajna. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <iostream>
#include <memory>
#include <vector>

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
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <ODF-file>" << std::endl;
        return 1;
    }

    std::string odfPath(argv[1]);
    int openFlags = 0;
    int errorCode = 0;
    std::unique_ptr<zip_t> zipArchive(
        zip_open(odfPath.c_str(), openFlags, &errorCode));
    if (!zipArchive)
    {
        zip_error_t zipError;
        zip_error_init_with_code(&zipError, errorCode);
        std::cerr << "Can't open zip archive '" << odfPath
                  << "': " << zip_error_strerror(&zipError) << "." << std::endl;
        zip_error_fini(&zipError);
        return 1;
    }

    zip_flags_t locateFlags = 0;
    zip_int64_t signatureZipIndex = zip_name_locate(
        zipArchive.get(), "META-INF/documentsignatures.xml", locateFlags);
    if (signatureZipIndex < 0)
    {
        std::cerr << "File '" << odfPath << "' does not contain any signatures."
                  << std::endl;
        return 1;
    }

    std::vector<char> signatureBuffer;
    std::unique_ptr<zip_file_t> zipFile(
        zip_fopen_index(zipArchive.get(), signatureZipIndex, 0));
    if (!zipFile)
    {
        std::cerr << "error, main: can't open file at index "
                  << signatureZipIndex << ": " << zip_strerror(zipArchive.get())
                  << std::endl;
        return 1;
    }

    std::cerr << "todo, main: verify signatures" << std::endl;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
