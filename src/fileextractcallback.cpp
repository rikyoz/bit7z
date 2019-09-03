// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "../include/fileextractcallback.hpp"

#include "Windows/FileDir.h"
#include "Windows/FileFind.h"

#include "../include/bitpropvariant.hpp"
#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"

using namespace std;
using namespace NWindows;
using namespace bit7z;

/* Most of this code, though heavily modified, is taken from the CExtractCallback class in Client7z.cpp of the 7z SDK
 * Main changes made:
 *  + Use of wstring instead of UString
 *  + Error messages are not showed. Instead, they are memorized into a wstring and used by BitExtractor to throw
 *    exceptions (see also Callback interface). Note that this class doesn't throw exceptions, as other classes in bit7,
 *    because it must implement interfaces with nothrow methods.
 *  + The work performed originally by the Init method is now performed by the class constructor */

FileExtractCallback::FileExtractCallback( const BitArchiveHandler& handler,
                                          const BitInputArchive& inputArchive,
                                          const wstring& inFilePath,
                                          const wstring& directoryPath )
    : ExtractCallback( handler, inputArchive ),
      mInFilePath( inFilePath ),
      mDirectoryPath( directoryPath ),
      mProcessedFileInfo(),
      mOutFileStreamSpec( nullptr ) {
    //NFile::NName::NormalizeDirPathPrefix( mDirectoryPath );
    filesystem::fsutil::normalizePath( mDirectoryPath );
}

FileExtractCallback::~FileExtractCallback() {}

//TODO: clean and optimize!
STDMETHODIMP FileExtractCallback::GetStream( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode ) try {
    *outStream = nullptr;
    mOutFileStream.Release();
    // Get Name
    BitPropVariant prop = mInputArchive.getItemProperty( index, BitProperty::Path );

    if ( prop.isEmpty() ) {
        mFilePath = !mInFilePath.empty() ? filesystem::fsutil::filename( mInFilePath ) : kEmptyFileAlias;
    } else if ( prop.isString() ) {
        mFilePath = prop.getString();
    } else {
        return E_FAIL;
    }

    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract ) {
        return S_OK;
    }

    // Get Attrib
    BitPropVariant prop2 = mInputArchive.getItemProperty( index, BitProperty::Attrib );

    if ( prop2.isEmpty() ) {
        mProcessedFileInfo.Attrib = 0;
        mProcessedFileInfo.AttribDefined = false;
    } else {
        if ( !prop2.isUInt32() ) {
            return E_FAIL;
        }

        mProcessedFileInfo.Attrib = prop2.getUInt32();
        mProcessedFileInfo.AttribDefined = true;
    }

    //RINOK( IsArchiveItemFolder( mInputArchive, index, mProcessedFileInfo.isDir ) );
    mProcessedFileInfo.isDir = mInputArchive.isItemFolder( index );

    // Get Modified Time
    BitPropVariant prop3 = mInputArchive.getItemProperty( index, BitProperty::MTime );
    mProcessedFileInfo.MTimeDefined = false;

    switch ( prop3.type() ) {
        case BitPropVariantType::Empty:
            // mProcessedFileInfo.MTime = _utcMTimeDefault;
            break;

        case BitPropVariantType::Filetime:
            mProcessedFileInfo.MTime = prop3.getFiletime();
            mProcessedFileInfo.MTimeDefined = true;
            break;

        default:
            return E_FAIL;
    }

    // Create folders for file
    size_t slashPos = mFilePath.rfind( WCHAR_PATH_SEPARATOR );

    if ( slashPos != wstring::npos ) {
        NFile::NDir::CreateComplexDir( ( mDirectoryPath + mFilePath.substr( 0, slashPos ) ).c_str() );
    }
    wstring fullProcessedPath = mDirectoryPath + mFilePath;
    mDiskFilePath = fullProcessedPath;

    if ( mProcessedFileInfo.isDir ) {
        NFile::NDir::CreateComplexDir( fullProcessedPath.c_str() );
    } else {
        NFile::NFind::CFileInfo fi;

        if ( mHandler.fileCallback() ) {
            wstring filename = filesystem::fsutil::filename( fullProcessedPath, true );
            mHandler.fileCallback()( filename );
        }

        if ( fi.Find( fullProcessedPath.c_str() ) ) {
            if ( !NFile::NDir::DeleteFileAlways( fullProcessedPath.c_str() ) ) {
                mErrorMessage = L"Cannot delete output file " + fullProcessedPath;
                return E_ABORT;
            }
        }

        mOutFileStreamSpec = new COutFileStream;
        CMyComPtr< ISequentialOutStream > outStreamLoc( mOutFileStreamSpec );

        if ( !mOutFileStreamSpec->Open( fullProcessedPath.c_str(), CREATE_ALWAYS ) ) {
            mErrorMessage = L"Cannot open output file " + fullProcessedPath;
            return E_ABORT;
        }

        mOutFileStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    }

    return S_OK;
} catch ( const BitException& ) {
    return E_OUTOFMEMORY;
}

STDMETHODIMP FileExtractCallback::SetOperationResult( Int32 operationResult ) {
    switch ( operationResult ) {
        case NArchive::NExtract::NOperationResult::kOK:
            break;

        default: {
            mNumErrors++;

            switch ( operationResult ) {
                case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
                    mErrorMessage = kUnsupportedMethod;
                    break;

                case NArchive::NExtract::NOperationResult::kCRCError:
                    mErrorMessage = kCRCFailed;
                    break;

                case NArchive::NExtract::NOperationResult::kDataError:
                    mErrorMessage = kDataError;
                    break;

                default:
                    mErrorMessage = kUnknownError;
            }
        }
    }

    if ( mOutFileStream != nullptr ) {
        if ( mProcessedFileInfo.MTimeDefined ) {
            mOutFileStreamSpec->SetMTime( &mProcessedFileInfo.MTime );
        }

        RINOK( mOutFileStreamSpec->Close() );
    }

    mOutFileStream.Release();

    if ( mExtractMode && mProcessedFileInfo.AttribDefined ) {
        NFile::NDir::SetFileAttrib( mDiskFilePath.c_str(), mProcessedFileInfo.Attrib );
    }

    if ( mNumErrors > 0 ) {
        return E_FAIL;
    }

    return S_OK;
}
