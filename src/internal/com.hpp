/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COM_HPP
#define COM_HPP

#define Z7_USE_VIRTUAL_DESTRUCTOR_IN_IUNKNOWN

#include "bitwindows.hpp"
#include "internal/guiddef.hpp"
#include "internal/windows.hpp"

#include <Common/MyCom.h>

#ifdef Z7_COM_UNKNOWN_IMP_1 // 7-zip 23.01+
#define SEVENZIP_2301
#endif

#ifdef Z7_COM_QI_BEGIN // 7-zip 23.01+
#undef Z7_COM_QI_BEGIN
#define Z7_COM_QI_BEGIN \
  STDMETHOD(QueryInterface) (REFGUID iid, void **outObject) throw() Z7_override Z7_final \
    { *outObject = NULL;
#endif

#ifdef Z7_COM_ADDREF_RELEASE // 7-zip 23.01+
#undef Z7_COM_ADDREF_RELEASE
#define Z7_COM_ADDREF_RELEASE \
  STDMETHOD_(ULONG, AddRef)() throw() Z7_override Z7_final \
    { return ++_m_RefCount; } \
  STDMETHOD_(ULONG, Release)() throw() Z7_override Z7_final \
    { if (--_m_RefCount != 0) return _m_RefCount;  delete this;  return 0; }
#endif

#ifndef MY_UNKNOWN_IMP3 // 7-zip 23.01+
#define MY_UNKNOWN_IMP3 Z7_COM_UNKNOWN_IMP_3
#endif

#ifndef MY_UNKNOWN_IMP1 // 7-zip 23.01+
#define MY_UNKNOWN_IMP1 Z7_COM_UNKNOWN_IMP_1
#endif

#endif //COM_HPP
