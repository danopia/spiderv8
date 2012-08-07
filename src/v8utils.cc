// Copyright 2011 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdarg.h>

#include "v8.h"

#include "platform.h"

#include "sys/stat.h"

namespace v8 {
namespace internal {


void PrintF(const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  OS::VPrint(format, arguments);
  va_end(arguments);
}


void PrintF(FILE* out, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  OS::VFPrint(out, format, arguments);
  va_end(arguments);
}


void PrintPID(const char* format, ...) {
  OS::Print("[pid] ");
  va_list arguments;
  va_start(arguments, format);
  OS::VPrint(format, arguments);
  va_end(arguments);
}


void StringBuilder::AddFormatted(const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  AddFormattedList(format, arguments);
  va_end(arguments);
}


void StringBuilder::AddFormattedList(const char* format, va_list list) {
  ASSERT(!is_finalized() && position_ < buffer_.length());
  int n = OS::VSNPrintF(buffer_ + position_, format, list);
  if (n < 0 || n >= (buffer_.length() - position_)) {
    position_ = buffer_.length();
  } else {
    position_ += n;
  }
}


MemoryMappedExternalResource::MemoryMappedExternalResource(const char* filename)
    : filename_(NULL),
      data_(NULL),
      length_(0),
      remove_file_on_cleanup_(false) {
  Init(filename);
}


MemoryMappedExternalResource::
    MemoryMappedExternalResource(const char* filename,
                                 bool remove_file_on_cleanup)
    : filename_(NULL),
      data_(NULL),
      length_(0),
      remove_file_on_cleanup_(remove_file_on_cleanup) {
  Init(filename);
}


MemoryMappedExternalResource::~MemoryMappedExternalResource() {
  // Release the resources if we had successfully acquired them:
  if (file_ != NULL) {
    delete file_;
    DeleteArray<char>(filename_);
  }
}


void MemoryMappedExternalResource::Init(const char* filename) {
  file_ = OS::MemoryMappedFile::open(filename);
  if (file_ != NULL) {
    filename_ = StrDup(filename);
    data_ = reinterpret_cast<char*>(file_->memory());
    length_ = file_->size();
  }
}


bool MemoryMappedExternalResource::EnsureIsAscii(bool abort_if_failed) const {
  bool is_ascii = true;

  int line_no = 1;
  const char* start_of_line = data_;
  const char* end = data_ + length_;
  for (const char* p = data_; p < end; p++) {
    char c = *p;
    if ((c & 0x80) != 0) {
      // Non-ASCII detected:
      is_ascii = false;

      // Report the error and abort if appropriate:
      if (abort_if_failed) {
        int char_no = static_cast<int>(p - start_of_line) - 1;

        ASSERT(filename_ != NULL);
        PrintF("\n\n\n"
               "Abort: Non-Ascii character 0x%.2x in file %s line %d char %d",
               c, filename_, line_no, char_no);

        // Allow for some context up to kNumberOfLeadingContextChars chars
        // before the offending non-ASCII char to help the user see where
        // the offending char is.
        const int kNumberOfLeadingContextChars = 10;
        const char* err_context = p - kNumberOfLeadingContextChars;
        if (err_context < data_) {
          err_context = data_;
        }
        // Compute the length of the error context and print it.
        int err_context_length = static_cast<int>(p - err_context);
        if (err_context_length != 0) {
          PrintF(" after \"%.*s\"", err_context_length, err_context);
        }
        PrintF(".\n\n\n");
        OS::Abort();
      }

      break;  // Non-ASCII detected.  No need to continue scanning.
    }
    if (c == '\n') {
      start_of_line = p;
      line_no++;
    }
  }

  return is_ascii;
}


} }  // namespace v8::internal
