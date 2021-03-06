// Copyright (c) Google LLC 2019
//
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <deque>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "third_party/brotli/interface/brotli.h"
#include "../common/constants.h"
#include <brunsli/jpeg_data.h>
#include <brunsli/status.h>
#include <brunsli/types.h>
#include <brunsli/brunsli_decode.h>
#include <brunsli/jpeg_data_writer.h>
#include "../dec/state.h"
#include "./test_utils.h"

namespace brunsli {

using ::brunsli::internal::dec::State;

TEST(StreamDecodeTest, DoneDone) {
  std::vector<uint8_t> src = GetSmallBrunsliFile();

  JPEGData jpg;
  State state;
  state.data = src.data();
  state.len = src.size();
  state.pos = 0;

  uint8_t foo[] = {42};

  // Decoding is finished.
  ASSERT_EQ(BRUNSLI_OK, internal::dec::ProcessJpeg(&state, &jpg));
  ASSERT_EQ(src.size(), state.pos);

  // It is OK to "continue" decoding, result is still "OK"...

  ASSERT_EQ(BRUNSLI_OK, internal::dec::ProcessJpeg(&state, &jpg));

  // ... unless more data is added.
  state.data = foo;
  state.len = 1;
  state.pos = 0;
  ASSERT_EQ(BRUNSLI_INVALID_BRN, internal::dec::ProcessJpeg(&state, &jpg));
}

TEST(StreamDecodeTest, ErrorError) {
  std::vector<uint8_t> src = GetSmallBrunsliFile();

  JPEGData jpg;
  State state;
  state.data = src.data();
  state.len = src.size();

  // Once decoder detects corrupted input...
  uint8_t original = src[0];
  src[0] = 42;
  ASSERT_EQ(BRUNSLI_INVALID_BRN, internal::dec::ProcessJpeg(&state, &jpg));

  // ... passing fixed input does not switch decoder to a good state.
  src[0] = original;
  ASSERT_EQ(BRUNSLI_INVALID_BRN, internal::dec::ProcessJpeg(&state, &jpg));
}

TEST(StreamDecodeTest, BytewiseInput) {
  std::vector<uint8_t> src = GetSmallBrunsliFile();

  JPEGData jpg;
  State state;
  for (size_t start = 0; start < src.size(); ++start) {
    state.data = src.data() + start;
    state.pos = 0;
    state.len = 1;
    ASSERT_EQ(start + 1 < src.size() ? BRUNSLI_NOT_ENOUGH_DATA : BRUNSLI_OK,
              internal::dec::ProcessJpeg(&state, &jpg));
    ASSERT_EQ(1, state.pos) << start;
  }
}

// 1600x1600 progressive JPEG.
// Together Brunsli + Brotli give over 100x compression!
// TODO(eustas): create progressive + interleaved image to demonstrate
//               MCU granularity
uint8_t kBrnBr[] = {
  0xD1, 0x90, 0xB4, 0x00, 0xB5, 0x21, 0x49, 0x39, 0x04, 0x40, 0xA7, 0x5E,
  0x65, 0x8C, 0x64, 0xA1, 0xEF, 0x04, 0x04, 0x83, 0x9D, 0xA5, 0xA0, 0x53,
  0xD0, 0x43, 0x7F, 0x23, 0xF9, 0x04, 0xCD, 0x44, 0xA1, 0x05, 0xF5, 0xD6,
  0x39, 0xD0, 0x9C, 0xD2, 0x26, 0x90, 0x40, 0x35, 0xA6, 0x53, 0xBE, 0x22,
  0x30, 0x52, 0x61, 0xF9, 0x24, 0x60, 0x90, 0xB8, 0xD4, 0x56, 0xED, 0xFD,
  0x47, 0x36, 0xDC, 0x58, 0x5B, 0xF3, 0x95, 0xAE, 0xA1, 0x07, 0x4B, 0x65,
  0x47, 0x9C, 0x93, 0x95, 0x33, 0x1E, 0x68, 0xA9, 0x9E, 0x37, 0xB2, 0x36,
  0x9D, 0xE8, 0xBF, 0x70, 0xD3, 0xBD, 0x95, 0xEA, 0x20, 0x43, 0xF3, 0x9F,
  0xC1, 0x93, 0x9F, 0x3D, 0x4C, 0xBC, 0xF9, 0x3E, 0xEA, 0x64, 0xCB, 0xC4,
  0xBE, 0xDB, 0xFB, 0xFA, 0x81, 0x2C, 0xEB, 0xF9, 0xF7, 0x58, 0xBF, 0xB4,
  0xE4, 0x92, 0x11, 0xB7, 0x96, 0x29, 0xD1, 0xFB, 0x5C, 0xA0, 0xA6, 0xB6,
  0x21, 0x1B, 0x1C, 0x94, 0x0B, 0xDB, 0x1E, 0x7B, 0x33, 0xB6, 0x3A, 0x36,
  0x59, 0xDE, 0x36, 0xF4, 0xA2, 0xF5, 0x6F, 0xD0, 0x8C, 0xD5, 0x12, 0x52,
  0x53, 0xCB, 0xD7, 0xD5, 0x47, 0xA5, 0xB6, 0x4C, 0x0F, 0x7B, 0x29, 0xFF,
  0xAC, 0x82, 0xB2, 0xAA, 0x72, 0x7C, 0x86, 0x41, 0xCF, 0x95, 0xD9, 0xA5,
  0x41, 0x65, 0xDB, 0xE7, 0x5A, 0xF1, 0x8B, 0xB9, 0xF4, 0xC6, 0x3A, 0x5E,
  0x10, 0xD0, 0xF6, 0xCE, 0x40, 0xAD, 0x29, 0xB2, 0x4D, 0x2C, 0x7B, 0x1A,
  0x76, 0xF3, 0x66, 0x8C, 0xA2, 0x56, 0x14, 0xA6, 0x58, 0x27, 0x15, 0x8D,
  0xAB, 0x7F, 0xB1, 0xA2, 0xBE, 0xEC, 0xFA, 0x04, 0x79, 0xA9, 0xE2, 0x7F,
  0x69, 0x71, 0xAD, 0xAC, 0x29, 0xB6, 0x5E, 0x2A, 0x5A, 0xAF, 0x48, 0x9A,
  0x51, 0x9C, 0x2E, 0x55, 0xC8, 0x9E, 0x37, 0x35, 0x3E, 0x19, 0x77, 0xB3,
  0x69, 0xDA, 0x73, 0xC5, 0x14, 0xE9, 0x13, 0xEB, 0x57, 0xE3, 0x6E, 0x8A,
  0x34, 0x1C, 0x67, 0xAC, 0x5B, 0xF2, 0x24, 0xEC, 0xE6, 0xF4, 0xB8, 0x72,
  0xF1, 0x55, 0xF1, 0x04, 0x99, 0x62, 0xDA, 0x73, 0x91, 0x5C, 0x05, 0x80,
  0x9E, 0x4A, 0xC1, 0xFB, 0x05, 0x01, 0xE5, 0xBC, 0xD9, 0xBB, 0x36, 0xD6,
  0x2E, 0x28, 0xAE, 0xD7, 0xC8, 0x7F, 0x34, 0xE7, 0x9C, 0xA9, 0x26, 0x9D,
  0x7F, 0x08, 0x02, 0x26, 0x00, 0xB4, 0xB5, 0x9D, 0xB9, 0xFB, 0xA9, 0x71,
  0x94, 0x6A, 0xDC, 0xB2, 0x49, 0x6C, 0x92, 0x0F, 0x3B, 0xFE, 0xB9, 0x6C,
  0x60, 0x5C, 0x41, 0x57, 0xCF, 0x7F, 0x26, 0x2B, 0xA4, 0xFB, 0xBA, 0xDC,
  0xAD, 0xDD, 0xF5, 0xF5, 0x67, 0xE9, 0x67, 0x43, 0xE5, 0x24, 0xC3, 0x17,
  0xD7, 0x1F, 0xB8, 0xBF, 0xE8, 0x3D, 0x70, 0xBA, 0xEF, 0xEA, 0x3B, 0x97,
  0x7C, 0x65, 0x35, 0x1A, 0xE7, 0xF2, 0xA5, 0xE1, 0x6D, 0x1E, 0xEE, 0x76,
  0xFD, 0x4B, 0x0D, 0xD7, 0xC6, 0x8C, 0xFF, 0x1E, 0xD6, 0xFB, 0xDB, 0x35,
  0x01, 0x13, 0x40, 0x0B, 0xBF, 0xAB, 0x67, 0x34, 0x00, 0xE0, 0x48, 0x3B,
  0x80, 0x8D, 0xBD, 0x69, 0xA5, 0x38, 0xCD, 0x2E, 0x76, 0xB2, 0xED, 0xC5,
  0x49, 0x8A, 0x33, 0xC3, 0x29, 0xFD, 0x6E, 0xD1, 0xE6, 0xAA, 0xA3, 0x8A,
  0xA7, 0xE3, 0x0E, 0xBD, 0xA3, 0x76, 0x0B, 0x44, 0x2C, 0x31, 0xC7, 0x2B,
  0xC1, 0x68, 0xBC, 0xA2, 0x51, 0x2D, 0xD7, 0x11, 0x4D, 0x31, 0x08, 0xCB,
  0x76, 0xF8, 0x39, 0x0D, 0x8C, 0x62, 0x5B, 0x2F, 0x9C, 0xC7, 0x49, 0x5C,
  0x7B, 0x19, 0x4A, 0x2B, 0x95, 0x36, 0x6D, 0x1C, 0x46, 0x63, 0x10, 0x5A,
  0x25, 0x0D, 0x57, 0x5A, 0x49, 0xF7, 0x63, 0x09, 0x62, 0xB3, 0x3E, 0xBE,
  0x4F, 0x3D, 0xA3, 0x71, 0x1C, 0x80, 0x5A, 0xBB, 0x11, 0xAF, 0x5E, 0x81,
  0x1A, 0x57, 0x47, 0x6B, 0xF1, 0x91, 0x3A, 0x7E, 0x24, 0xFA, 0x64, 0x26,
  0xFD, 0x0A, 0x11, 0xEE, 0x8F, 0x9C, 0x3A, 0x7B, 0x2A, 0x46, 0x32, 0x00,
  0x13, 0x3C, 0x15, 0x99, 0xFF, 0x16, 0xB3, 0x4D, 0x4A, 0xDF, 0xB7, 0x17,
  0xEA, 0xCC, 0x01, 0x9C, 0xF7, 0x13, 0xFA, 0xA9, 0xEB, 0x31, 0xFB, 0xDD,
  0x51, 0xE3, 0xD7, 0x63, 0x25, 0x49, 0xD4, 0x0D, 0x3B, 0xF1, 0xA3, 0xAB,
  0x68, 0x97, 0xBC, 0xC7, 0x3D, 0xFD, 0x4F, 0x7F, 0xEC, 0x30, 0x7D, 0x91,
  0x07, 0x66, 0x9D, 0x17, 0xB2, 0xFE, 0x20, 0xC2, 0x31, 0x12, 0xB7, 0xF8,
  0x36, 0xA2, 0x43, 0x1C, 0x62, 0xB4, 0x09, 0xE6, 0x97, 0x35, 0x36, 0xF4,
  0x19, 0x2E, 0x3D, 0x03, 0xD7, 0xAE, 0x84, 0x2E, 0xCC, 0x14, 0x6F, 0x51,
  0x80, 0x79, 0x91, 0x8D, 0x4A, 0x2A, 0xA0, 0xBE, 0xF2, 0x12, 0x51, 0x16,
  0x86, 0xD9, 0x2F, 0x20, 0x5E, 0xF7, 0x41, 0xA6, 0x3C, 0xA4, 0xAF, 0x0C,
  0xC6, 0xAB, 0x17, 0x52, 0xE6, 0x76, 0x47, 0xDD, 0xDA, 0x8C, 0x3C, 0xFC,
  0x11, 0xBF, 0xB4, 0x89, 0xAE, 0xF8, 0x11, 0xEE, 0x73, 0x12, 0x65, 0xB8,
  0x0A, 0x66, 0xA2, 0x26, 0x32, 0xF2, 0x32, 0x2A, 0xF5, 0x22, 0xFD, 0xA7,
  0xD1, 0x38, 0xD1, 0x7E, 0xF6, 0xBF, 0x4D, 0x07
};

TEST(StreamDecodeTest, StreamingSerialization) {
  std::string br_bytes =
      std::string(reinterpret_cast<const char*>(kBrnBr), sizeof(kBrnBr));
  std::string brn_bytes;
  util::compression::Brotli decompressor;
  decompressor.Uncompress(br_bytes, &brn_bytes);
  const uint8_t* data = reinterpret_cast<uint8_t*>(brn_bytes.data());
  size_t size = brn_bytes.size();

  std::deque<size_t> output_chunk_size = {235, 15033, 16380, 38966};

  BrunsliDecoder decoder;

  for (size_t start = 0; start < size; ++start) {
    size_t available_in = 1;
    const uint8_t* next_in = data + start;
    size_t available_out = 0;
    uint8_t out[1];
    uint8_t* next_out = out;
    BrunsliDecoder::Status result =
        decoder.Decode(&available_in, &next_in, &available_out, &next_out);
    ASSERT_TRUE(result != BrunsliDecoder::ERROR);

    size_t output_streak_len = 0;
    while (true) {
      available_in = 0;
      available_out = 1;
      next_out = out;
      result =
          decoder.Decode(&available_in, &next_in, &available_out, &next_out);
      if (available_out == 1) {
        ASSERT_TRUE((result == BrunsliDecoder::NEEDS_MORE_INPUT) ||
                    (result == BrunsliDecoder::DONE));
        break;
      } else {
        output_streak_len++;
        ASSERT_TRUE((result == BrunsliDecoder::NEEDS_MORE_OUTPUT) ||
                    (result == BrunsliDecoder::NEEDS_MORE_INPUT) ||
                    (result == BrunsliDecoder::DONE));
      }
    }
    if (output_streak_len != 0) {
      ASSERT_EQ(output_chunk_size[0], output_streak_len);
      output_chunk_size.pop_front();
    }
  }
  ASSERT_TRUE(output_chunk_size.empty());
}

TEST(StreamDecodeTest, BytewiseFallbackInput) {
  std::vector<uint8_t> src = GetFallbackBrunsliFile();

  JPEGData jpg;
  State state;
  size_t start = 0;
  for (size_t end = 0; end < src.size(); ++end) {
    state.data = src.data() + start;
    state.pos = 0;
    state.len = end - start;
    ASSERT_EQ(BRUNSLI_NOT_ENOUGH_DATA,
              internal::dec::ProcessJpeg(&state, &jpg));
    start += state.pos;
  }
  state.data = src.data() + start;
  state.pos = 0;
  state.len = src.size() - start;
  ASSERT_EQ(BRUNSLI_OK, internal::dec::ProcessJpeg(&state, &jpg));
}
}  // namespace brunsli
