/*
MIT License

Copyright (c) 2018 Ben Lancaster

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef LIBPRCO_TARGET_H
#define LIBPRCO_TARGET_H

#include "../adt/ast.h"

/// Supported backend targets
/// \brief
typedef enum target_archs {
    target_generic,  ///< A generic assembly target
    target_template, ///< A template assembly target, for starting with
    target_8086,     ///< 8086 intel syntax target
    target_x86,      ///< x86 at&t syntax target
    target_uvm100,   ///< https://github.com/bendl/uvm target
} target_archs;

/// Target datatype sizes
/// \brief
/// \see get_dt_size
typedef enum target_datatype {
    dtINT,  ///< Int
    dtPTR,  ///< Pointer
    dtCHAR, ///< 1 byte
} target_datatype;

#endif