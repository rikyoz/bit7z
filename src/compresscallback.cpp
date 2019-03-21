#include "../include/compresscallback.hpp"

using namespace bit7z;

CompressCallback::CompressCallback( const BitArchiveCreator& creator, const BitInputArchive* old_arc )
    : mCreator( creator ),
      mOldArc( old_arc ),
      mOldArcItemsCount( old_arc ? old_arc->itemsCount() : 0 ),
      mAskPassword( false ),
      mNeedBeClosed( false ) {}

HRESULT CompressCallback::Finilize() {
    if ( mNeedBeClosed ) {
        mNeedBeClosed = false;
    }

    return S_OK;
}

HRESULT CompressCallback::SetTotal( UInt64 size ) {
    if ( mCreator.totalCallback() ) {
        mCreator.totalCallback()( size );
    }
    return S_OK;
}

HRESULT CompressCallback::SetCompleted( const UInt64* completeValue ) {
    if ( mCreator.progressCallback() && completeValue != nullptr ) {
        mCreator.progressCallback()( *completeValue );
    }
    return S_OK;
}

HRESULT CompressCallback::EnumProperties( IEnumSTATPROPSTG** /* enumerator */ ) {
    return E_NOTIMPL;
}

HRESULT CompressCallback::GetUpdateItemInfo( UInt32 index, Int32* newData,
                                          Int32* newProperties, UInt32* indexInArchive ) {

    bool isOldItem = index < mOldArcItemsCount;

    if ( newData != nullptr ) {
        *newData = isOldItem ? 0 : 1; //= true;
    }
    if ( newProperties != nullptr ) {
        *newProperties = isOldItem ? 0 : 1; //= true;
    }
    if ( indexInArchive != nullptr ) {
        *indexInArchive = isOldItem ? index : static_cast< uint32_t >( -1 );
    }

    return S_OK;
}

HRESULT CompressCallback::SetOperationResult( Int32 /* operationResult */ ) {
    mNeedBeClosed = true;
    return S_OK;
}

HRESULT CompressCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password ) {
    if ( !mCreator.isPasswordDefined() ) {
        if ( mAskPassword ) {
            // You can ask real password here from user
            // Password = GetPassword(OutStream);
            // PasswordIsDefined = true;
            mErrorMessage = L"Password is not defined";
            return E_ABORT;
        }
    }

    *passwordIsDefined = ( mCreator.isPasswordDefined() ? 1 : 0 );
    return StringToBstr( mCreator.password().c_str(), password );
}
