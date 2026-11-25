/*
 * Copyright (c) 2025 Rizki Rakasiwi <rizkirr.xyz@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * libjson — zero-copy JSON parser (STB-style single-header library)
 *
 * In exactly ONE C file, do:
 *   #define LIBJSON_IMPLEMENTATION
 *   #include "libjson.h"
 *
 * All other files just #include "libjson.h" for declarations only.
 *
 * This header targets C99 and later, and also compiles as C++. If you can
 * require C23, libjson23.h offers the same public API with [[nodiscard]] on
 * every fallible function and overflow-checked bounds arithmetic. See the
 * "Choosing a header" section of README.md.
 */

#ifndef LIBJSON_H
#define LIBJSON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifndef JSON_NO_STDIO
#include <float.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#endif
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char *data;
  size_t len;
} JsonSlice;

typedef struct {
  JsonSlice array;
  const char *cursor;
} JsonArrayIter;

typedef struct {
  JsonSlice object;
  const char *cursor;
} JsonObjectIter;

/*
 * Build a slice from an existing pointer and length.
 *
 * The slice does not own the memory. The caller must keep `data` alive for as
 * long as the slice is used.
 */
JsonSlice json_from_parts(const char *data, size_t len);

/*
 * Build a slice from a NUL-terminated JSON string.
 *
 * This is a convenience wrapper around `json_from_parts()`.
 */
JsonSlice json_from_cstr(const char *json);

/*
 * Copy a slice into a caller-provided buffer and append a trailing NUL byte.
 *
 * Returns false if `buffer` is NULL, the slice is invalid, or the buffer is
 * too small. This helper is optional and exists for callers that need a
 * conventional C string without using heap allocation.
 */
bool json_slice_copy(JsonSlice slice, char *buffer, size_t buffer_size);

/*
 * Decode JSON escape sequences from `slice` into a caller-provided buffer and
 * append a trailing NUL byte.
 *
 * Parsed slices are zero-copy views of the original document, so string keys
 * and values still contain their raw escape sequences. This helper resolves
 * them: `\" \\ \/ \b \f \n \r \t` and `\uXXXX`, including surrogate pairs,
 * which are encoded as UTF-8. The decoded byte length (excluding the NUL) is
 * stored in `*out_len` when `out_len` is non-NULL.
 *
 * Decoded output is never longer than the input, so a buffer of
 * `slice.len + 1` bytes always suffices.
 *
 * Returns false if `buffer` is NULL, the buffer is too small, or the slice
 * contains an invalid escape sequence, an unpaired surrogate, or a trailing
 * lone backslash.
 */
bool json_slice_unescape(JsonSlice slice, char *buffer, size_t buffer_size,
                         size_t *out_len);

#ifndef JSON_NO_STDIO
/*
 * Convert a slice holding a JSON number token into a double.
 *
 * This is the reader-side counterpart to `json_write_double()`: a value
 * written by that function and parsed back out reproduces the original double
 * bit-for-bit. Pass the slice returned by `json_get()` or one of the
 * iterators; the slice is not required to be NUL-terminated.
 *
 * Returns false if `out` is NULL, if the slice is not a syntactically valid
 * RFC 8259 number, if the token is longer than `JSON_DOUBLE_MAX_TOKEN` bytes,
 * or if the value overflows the range of a double. Values too small to
 * represent underflow to zero and are accepted. `*out` is only written on
 * success.
 *
 * Note that `json_get()` returns string values without their surrounding
 * quotes, so the JSON value `"1.5"` and the JSON value `1.5` yield identical
 * slices and both convert successfully. Callers that must distinguish a
 * quoted number from a bare one have to inspect the raw document.
 *
 * Requires `strtod()`, so this function is unavailable in `JSON_NO_STDIO`
 * builds.
 */
bool json_slice_to_double(JsonSlice slice, double *out);

/*
 * Maximum length, in bytes, of a number token `json_slice_to_double()` accepts.
 *
 * A double never needs more than ~770 significant characters, so this only
 * rejects pathologically zero-padded input such as a million leading zeroes.
 */
#define JSON_DOUBLE_MAX_TOKEN 1024
#endif

/*
 * Look up `key` with an explicit key length.
 *
 * This avoids recomputing `strlen(key)` in hot lookup paths.
 */
bool json_getn(JsonSlice object, const char *key, size_t key_len,
               JsonSlice *out);

/*
 * Look up `key` in a JSON object slice and return the matched value as a
 * zero-copy slice.
 *
 * String results exclude the surrounding quotes. Object and array results keep
 * their delimiters. Number, boolean, and null results are returned as raw
 * tokens. Returns false if the key is missing or the input is not a valid
 * object-shaped slice.
 */
bool json_get(JsonSlice object, const char *key, JsonSlice *out);

/*
 * Initialize an iterator for a JSON object slice.
 *
 * The iterator walks key/value pairs in-place over the original input buffer.
 */
void json_object_iter_init(JsonSlice object, JsonObjectIter *iter);

/*
 * Return the next object key/value pair as zero-copy slices.
 *
 * Key slices exclude surrounding quotes. Returns true while entries remain.
 * Returns false when the object is exhausted or when invalid input is
 * encountered.
 */
bool json_object_iter_next(JsonObjectIter *iter, JsonSlice *key,
                           JsonSlice *value);

/*
 * Initialize an iterator for a JSON array slice.
 *
 * The iterator does not allocate and walks the array in-place over the
 * original input buffer.
 */
void json_array_iter_init(JsonSlice array, JsonArrayIter *iter);

/*
 * Return the next array item as a zero-copy slice.
 *
 * Returns true while elements remain. Returns false when the array is
 * exhausted or when invalid input is encountered.
 */
bool json_array_iter_next(JsonArrayIter *iter, JsonSlice *out);

/* ========================================================================= */
/*                          JSON WRITER (Serialization)                       */
/* ========================================================================= */

/*
 * JsonWriter — zero-allocation JSON serializer.
 *
 * Writes JSON into a caller-provided buffer. If the buffer runs out of space
 * the overflow flag is set and all subsequent writes become silent no-ops.
 * Check `json_writer_ok()` once after building the document.
 *
 * Comma insertion between values is fully automatic. Bitmasks track the
 * "needs comma" state and the container type at each nesting depth, using zero
 * extra RAM. Since the masks are 32-bit and depth 0 is the top level, nesting
 * is limited to JSON_WRITER_MAX_DEPTH levels; exceeding it sets the overflow
 * flag rather than corrupting the masks.
 *
 * Unbalanced or mismatched container ends (`json_write_array_end()` closing an
 * object, or an end with no matching begin) also set the overflow flag, so a
 * single `json_writer_ok()` check at the end catches structural mistakes as
 * well as a too-small buffer.
 */
#define JSON_WRITER_MAX_DEPTH 31

typedef struct {
  char *buf;
  size_t cap;
  size_t pos;
  bool overflow;
  unsigned int depth;
  unsigned int needs_comma;
  unsigned int is_array;
} JsonWriter;

/*
 * Initialize a writer that targets `buf` with `cap` bytes of space.
 *
 * The buffer is NOT owned by the writer; the caller manages its lifetime.
 * One byte is always reserved for a trailing NUL, so the maximum JSON payload
 * length is `cap - 1`.
 */
void json_writer_init(JsonWriter *w, char *buf, size_t cap);

/*
 * Return the number of JSON bytes written so far (excluding NUL terminator).
 */
size_t json_writer_len(const JsonWriter *w);

/*
 * Return true if the writer has not overflowed.
 */
bool json_writer_ok(const JsonWriter *w);

/*
 * Return the NUL-terminated output buffer. Only valid when `json_writer_ok()`
 * is true.
 */
const char *json_writer_output(const JsonWriter *w);

/*
 * Write a JSON null literal.
 */
void json_write_null(JsonWriter *w);

/*
 * Write a JSON boolean literal.
 */
void json_write_bool(JsonWriter *w, bool value);

/*
 * Write a signed integer as a JSON number.
 */
void json_write_int(JsonWriter *w, long value);

/*
 * Write an unsigned integer as a JSON number.
 */
void json_write_uint(JsonWriter *w, unsigned long value);

#ifndef JSON_NO_STDIO
/*
 * Write a double as a JSON number.
 *
 * The value is printed as the shortest decimal string that reads back as
 * exactly the same double, so `0.1` emits `0.1` rather than
 * `0.10000000000000001` while still round-tripping bit-for-bit.
 *
 * JSON has no representation for NaN or infinity. Those values are written as
 * `null`, mirroring how `json_write_str()` handles a NULL pointer.
 *
 * Requires `snprintf()` and `strtod()`, so this function is unavailable in
 * `JSON_NO_STDIO` builds.
 */
void json_write_double(JsonWriter *w, double value);
#endif

/*
 * Write a NUL-terminated C string as a JSON string with proper escaping.
 *
 * Characters that require escaping per RFC 8259 (", \, control characters)
 * are escaped automatically.
 */
void json_write_str(JsonWriter *w, const char *str);

/*
 * Write a `"key":"value"` pair inside an object in a single call.
 *
 * Equivalent to `json_write_key(w, key); json_write_str(w, value);`. Both the
 * key and the value are escaped per RFC 8259. If `value` is NULL the entry is
 * written as `"key":null`.
 */
void json_write_str_kv(JsonWriter *w, const char *key, const char *value);

/*
 * Write a string with explicit length as a JSON string with proper escaping.
 */
void json_write_strn(JsonWriter *w, const char *str, size_t len);

/*
 * Write a `"key":"value"` pair where the value has an explicit length.
 *
 * Equivalent to `json_write_key(w, key); json_write_strn(w, value, len);`. If
 * `value` is NULL the entry is written as `"key":null`.
 */
void json_write_strn_kv(JsonWriter *w, const char *key, const char *value,
                        size_t len);

/*
 * Write pre-formatted raw JSON bytes without any escaping or quoting.
 *
 * Useful for embedding pre-built JSON fragments or custom number formatting
 * (e.g. fixed-point representations).
 */
void json_write_raw(JsonWriter *w, const char *raw, size_t len);

/*
 * Write a `"key":<raw>` pair where the value is pre-formatted raw JSON bytes.
 */
void json_write_raw_kv(JsonWriter *w, const char *key, const char *raw,
                       size_t len);

/*
 * Write a `"key":null` pair inside an object in a single call.
 */
void json_write_null_kv(JsonWriter *w, const char *key);

/*
 * Write a `"key":<true|false>` pair inside an object in a single call.
 */
void json_write_bool_kv(JsonWriter *w, const char *key, bool value);

/*
 * Write a `"key":<signed integer>` pair inside an object in a single call.
 */
void json_write_int_kv(JsonWriter *w, const char *key, long value);

/*
 * Write a `"key":<unsigned integer>` pair inside an object in a single call.
 */
void json_write_uint_kv(JsonWriter *w, const char *key, unsigned long value);

#ifndef JSON_NO_STDIO
/*
 * Write a `"key":<double>` pair inside an object in a single call.
 *
 * Equivalent to `json_write_key(w, key); json_write_double(w, value);`, and
 * subject to the same NaN/infinity and `JSON_NO_STDIO` rules.
 */
void json_write_double_kv(JsonWriter *w, const char *key, double value);
#endif

/*
 * Begin a `"key":{` object entry inside an object. Pair with
 * `json_write_object_end()`.
 */
void json_write_object_begin_k(JsonWriter *w, const char *key);

/*
 * Begin a `"key":[` array entry inside an object. Pair with
 * `json_write_array_end()`.
 */
void json_write_array_begin_k(JsonWriter *w, const char *key);

/*
 * Begin a JSON object. Must be paired with `json_write_object_end()`.
 */
void json_write_object_begin(JsonWriter *w);

/*
 * End the current JSON object.
 */
void json_write_object_end(JsonWriter *w);

/*
 * Write an object key. The next write call produces the associated value.
 *
 * Writes `"key":` with automatic comma insertion before the key when needed.
 */
void json_write_key(JsonWriter *w, const char *key);

/*
 * Begin a JSON array. Must be paired with `json_write_array_end()`.
 */
void json_write_array_begin(JsonWriter *w);

/*
 * End the current JSON array.
 */
void json_write_array_end(JsonWriter *w);

#ifdef __cplusplus
}
#endif

#endif /* LIBJSON_H */

/* ========================================================================= */
/*                            IMPLEMENTATION                                 */
/* ========================================================================= */

#ifdef LIBJSON_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

#ifdef JSON_NO_STDIO
static void json__error(const char *message) { (void)message; }
#else
static void json__error(const char *message) {
  fprintf(stderr, "%s\n", message);
}
#endif

static const char *json__skip_whitespace(const char *cursor, const char *end) {
  while (cursor < end) {
    unsigned char ch = (unsigned char)*cursor;
    if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r')
      break;
    cursor++;
  }
  return cursor;
}

/*
 * Return the position just past the closing quote of the string at `cursor`.
 *
 * Only the closing quote is searched for with memchr; a candidate quote is a
 * terminator when the run of backslashes immediately before it has even
 * length. Those runs never overlap between candidates, so the whole scan stays
 * linear even on escape-dense input.
 */
static const char *json__skip_string(const char *cursor, const char *end) {
  const char *body;

  if (cursor >= end || *cursor != '"') {
    json__error("json: expected string");
    return NULL;
  }

  body = cursor + 1;
  cursor = body;
  while (cursor < end) {
    const char *quote =
        (const char *)memchr(cursor, '"', (size_t)(end - cursor));
    const char *scan;
    size_t backslashes = 0;

    if (!quote)
      break;

    scan = quote;
    while (scan > body && *(scan - 1) == '\\') {
      backslashes++;
      scan--;
    }

    if ((backslashes & 1) == 0)
      return quote + 1;

    cursor = quote + 1;
  }

  json__error("json: unterminated string");
  return NULL;
}

/*
 * Return the position just past the matching close delimiter.
 *
 * This only balances delimiters and steps over strings; it does not validate
 * the grammar of the contents. Nested values are validated when the caller
 * descends into them via json_get() or an iterator.
 */
static const char *json__skip_compound(const char *cursor, const char *end,
                                       char open_ch, char close_ch) {
  int depth = 1;

  cursor++;
  while (cursor < end) {
    char ch = *cursor;

    if (ch == '"') {
      cursor = json__skip_string(cursor, end);
      if (!cursor)
        return NULL;
      continue;
    }

    if (ch == open_ch) {
      depth++;
    } else if (ch == close_ch) {
      depth--;
      if (depth == 0)
        return cursor + 1;
    }

    cursor++;
  }

  json__error("json: unterminated compound value");
  return NULL;
}

/* Match a JSON number per RFC 8259 against exactly the range [cursor, end). */
static bool json__is_number(const char *cursor, const char *end) {
  if (cursor >= end)
    return false;

  if (*cursor == '-')
    cursor++;

  if (cursor >= end)
    return false;

  if (*cursor == '0') {
    cursor++;
  } else if (*cursor >= '1' && *cursor <= '9') {
    while (cursor < end && *cursor >= '0' && *cursor <= '9')
      cursor++;
  } else {
    return false;
  }

  if (cursor < end && *cursor == '.') {
    cursor++;
    if (cursor >= end || *cursor < '0' || *cursor > '9')
      return false;
    while (cursor < end && *cursor >= '0' && *cursor <= '9')
      cursor++;
  }

  if (cursor < end && (*cursor == 'e' || *cursor == 'E')) {
    cursor++;
    if (cursor < end && (*cursor == '+' || *cursor == '-'))
      cursor++;
    if (cursor >= end || *cursor < '0' || *cursor > '9')
      return false;
    while (cursor < end && *cursor >= '0' && *cursor <= '9')
      cursor++;
  }

  return cursor == end;
}

/*
 * Consume a bare token and verify it is `true`, `false`, `null`, or a number.
 *
 * Returns NULL for anything else, including the empty token produced by input
 * such as `{"a":}`.
 */
static const char *json__skip_primitive(const char *cursor, const char *end) {
  const char *start = cursor;
  size_t len;

  while (cursor < end) {
    char ch = *cursor;
    if (ch == ',' || ch == ']' || ch == '}' || ch == ' ' || ch == '\t' ||
        ch == '\n' || ch == '\r') {
      break;
    }
    cursor++;
  }

  len = (size_t)(cursor - start);
  if (len == 4 && memcmp(start, "true", 4) == 0)
    return cursor;
  if (len == 5 && memcmp(start, "false", 5) == 0)
    return cursor;
  if (len == 4 && memcmp(start, "null", 4) == 0)
    return cursor;
  if (json__is_number(start, cursor))
    return cursor;

  json__error("json: invalid value");
  return NULL;
}

static const char *json__skip_value(const char *cursor, const char *end) {
  cursor = json__skip_whitespace(cursor, end);
  if (cursor >= end) {
    json__error("json: expected value");
    return NULL;
  }

  if (*cursor == '"')
    return json__skip_string(cursor, end);
  if (*cursor == '{')
    return json__skip_compound(cursor, end, '{', '}');
  if (*cursor == '[')
    return json__skip_compound(cursor, end, '[', ']');
  return json__skip_primitive(cursor, end);
}

static JsonSlice json__value_slice(const char *start, const char *end) {
  if (!start || !end || end < start)
    return json_from_parts(NULL, 0);

  if (*start == '"' && end > start + 1 && *(end - 1) == '"')
    return json_from_parts(start + 1, (size_t)((end - start) - 2));

  return json_from_parts(start, (size_t)(end - start));
}

/*
 * Step to the next object member.
 *
 * `first` must be true only for the member immediately after the opening
 * brace; every later member requires a separating comma. The trailing comma is
 * deliberately left unconsumed so the next call can enforce that rule.
 */
static bool json__object_next(const char **cursor_ptr, const char *end,
                              bool first, JsonSlice *key, JsonSlice *value) {
  const char *cursor = *cursor_ptr;
  const char *key_start;
  const char *key_end;
  const char *value_start;
  const char *value_end;

  cursor = json__skip_whitespace(cursor, end);
  if (cursor >= end) {
    json__error("json: unterminated object");
    return false;
  }

  if (*cursor == '}') {
    *cursor_ptr = cursor;
    return false;
  }

  if (!first) {
    if (*cursor != ',') {
      json__error("json: expected ',' or '}' in object");
      return false;
    }
    cursor = json__skip_whitespace(cursor + 1, end);
    if (cursor >= end) {
      json__error("json: unterminated object");
      return false;
    }
    if (*cursor == '}') {
      json__error("json: trailing comma in object");
      return false;
    }
  }

  if (*cursor != '"') {
    json__error("json: expected object key");
    return false;
  }

  key_start = cursor + 1;
  key_end = json__skip_string(cursor, end);
  if (!key_end)
    return false;

  cursor = json__skip_whitespace(key_end, end);
  if (cursor >= end || *cursor != ':') {
    json__error("json: expected ':' after key");
    return false;
  }

  cursor++;
  value_start = json__skip_whitespace(cursor, end);
  value_end = json__skip_value(value_start, end);
  if (!value_end)
    return false;

  if (key)
    *key = json_from_parts(key_start, (size_t)((key_end - 1) - key_start));
  if (value)
    *value = json__value_slice(value_start, value_end);

  *cursor_ptr = value_end;
  return true;
}

/*
 * Step to the next array element, following the same comma rules as
 * json__object_next().
 */
static bool json__array_next(const char **cursor_ptr, const char *end,
                             bool first, JsonSlice *out) {
  const char *cursor = *cursor_ptr;
  const char *value_start;
  const char *value_end;

  cursor = json__skip_whitespace(cursor, end);
  if (cursor >= end) {
    json__error("json: unterminated array");
    return false;
  }

  if (*cursor == ']') {
    *cursor_ptr = cursor;
    return false;
  }

  if (!first) {
    if (*cursor != ',') {
      json__error("json: expected ',' or ']' in array");
      return false;
    }
    cursor = json__skip_whitespace(cursor + 1, end);
    if (cursor >= end) {
      json__error("json: unterminated array");
      return false;
    }
    if (*cursor == ']') {
      json__error("json: trailing comma in array");
      return false;
    }
  }

  value_start = cursor;
  value_end = json__skip_value(value_start, end);
  if (!value_end)
    return false;

  if (out)
    *out = json__value_slice(value_start, value_end);

  *cursor_ptr = value_end;
  return true;
}

JsonSlice json_from_parts(const char *data, size_t len) {
  JsonSlice slice;
  slice.data = data;
  slice.len = len;
  return slice;
}

JsonSlice json_from_cstr(const char *json) {
  if (!json)
    return json_from_parts(NULL, 0);
  return json_from_parts(json, strlen(json));
}

bool json_slice_copy(JsonSlice slice, char *buffer, size_t buffer_size) {
  if (!buffer) {
    json__error("json_slice_copy: buffer is NULL");
    return false;
  }

  if (!slice.data && slice.len != 0) {
    json__error("json_slice_copy: invalid slice");
    return false;
  }

  if (buffer_size <= slice.len) {
    json__error("json_slice_copy: buffer too small");
    return false;
  }

  if (slice.len > 0)
    memcpy(buffer, slice.data, slice.len);
  buffer[slice.len] = '\0';
  return true;
}

#ifndef JSON_NO_STDIO
bool json_slice_to_double(JsonSlice slice, double *out) {
  /* One '.' may expand to a multi-byte locale separator; the slack covers it
     and the bounds checks below cover anything wider still. */
  char buf[JSON_DOUBLE_MAX_TOKEN + 8];
  const struct lconv *lc;
  const char *point;
  size_t point_len;
  size_t i;
  size_t len = 0;
  double value;
  char *parse_end;

  if (!out) {
    json__error("json_slice_to_double: out is NULL");
    return false;
  }

  if (!slice.data || slice.len == 0) {
    json__error("json_slice_to_double: invalid slice");
    return false;
  }

  if (slice.len > JSON_DOUBLE_MAX_TOKEN) {
    json__error("json_slice_to_double: number token too long");
    return false;
  }

  if (!json__is_number(slice.data, slice.data + slice.len)) {
    json__error("json_slice_to_double: not a JSON number");
    return false;
  }

  /* JSON always writes '.', but strtod() only accepts the separator of the
     current locale, which is ',' under de_DE and friends. Translate. */
  lc = localeconv();
  point = (lc && lc->decimal_point && lc->decimal_point[0]) ? lc->decimal_point
                                                            : ".";
  point_len = strlen(point);

  for (i = 0; i < slice.len; i++) {
    if (slice.data[i] == '.') {
      if (len + point_len >= sizeof(buf)) {
        json__error("json_slice_to_double: number token too long");
        return false;
      }
      memcpy(buf + len, point, point_len);
      len += point_len;
    } else {
      if (len + 1 >= sizeof(buf)) {
        json__error("json_slice_to_double: number token too long");
        return false;
      }
      buf[len++] = slice.data[i];
    }
  }
  buf[len] = '\0';

  parse_end = NULL;
  value = strtod(buf, &parse_end);

  if (parse_end != buf + len) {
    json__error("json_slice_to_double: trailing characters in number");
    return false;
  }

  /* Rejects the infinity strtod() returns on overflow. Underflow yields a
     finite zero or denormal and is accepted. */
  if (!(value >= -DBL_MAX && value <= DBL_MAX)) {
    json__error("json_slice_to_double: value out of range for double");
    return false;
  }

  *out = value;
  return true;
}
#endif

static int json__hex_digit(char ch) {
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  if (ch >= 'a' && ch <= 'f')
    return ch - 'a' + 10;
  if (ch >= 'A' && ch <= 'F')
    return ch - 'A' + 10;
  return -1;
}

/* Decode exactly four hex digits; the caller guarantees they are in bounds. */
static bool json__parse_hex4(const char *p, unsigned long *out) {
  unsigned long value = 0;
  int i;

  for (i = 0; i < 4; i++) {
    int digit = json__hex_digit(p[i]);
    if (digit < 0)
      return false;
    value = (value << 4) | (unsigned long)digit;
  }

  *out = value;
  return true;
}

/*
 * Append `codepoint` as UTF-8, reserving one byte for the trailing NUL.
 *
 * `unsigned long` is used rather than `unsigned int` because a codepoint above
 * the basic plane needs 21 bits and `unsigned int` is only guaranteed 16.
 */
static bool json__append_utf8(char *buffer, size_t buffer_size, size_t *pos,
                              unsigned long codepoint) {
  size_t needed;

  if (codepoint < 0x80ul)
    needed = 1;
  else if (codepoint < 0x800ul)
    needed = 2;
  else if (codepoint < 0x10000ul)
    needed = 3;
  else
    needed = 4;

  if (*pos + needed >= buffer_size)
    return false;

  switch (needed) {
  case 1:
    buffer[(*pos)++] = (char)codepoint;
    break;
  case 2:
    buffer[(*pos)++] = (char)(0xC0ul | (codepoint >> 6));
    buffer[(*pos)++] = (char)(0x80ul | (codepoint & 0x3Ful));
    break;
  case 3:
    buffer[(*pos)++] = (char)(0xE0ul | (codepoint >> 12));
    buffer[(*pos)++] = (char)(0x80ul | ((codepoint >> 6) & 0x3Ful));
    buffer[(*pos)++] = (char)(0x80ul | (codepoint & 0x3Ful));
    break;
  default:
    buffer[(*pos)++] = (char)(0xF0ul | (codepoint >> 18));
    buffer[(*pos)++] = (char)(0x80ul | ((codepoint >> 12) & 0x3Ful));
    buffer[(*pos)++] = (char)(0x80ul | ((codepoint >> 6) & 0x3Ful));
    buffer[(*pos)++] = (char)(0x80ul | (codepoint & 0x3Ful));
    break;
  }

  return true;
}

bool json_slice_unescape(JsonSlice slice, char *buffer, size_t buffer_size,
                         size_t *out_len) {
  size_t i = 0;
  size_t pos = 0;

  if (out_len)
    *out_len = 0;

  if (!buffer || buffer_size == 0) {
    json__error("json_slice_unescape: buffer is NULL or empty");
    return false;
  }

  if (!slice.data && slice.len != 0) {
    json__error("json_slice_unescape: invalid slice");
    return false;
  }

  while (i < slice.len) {
    char ch = slice.data[i];
    char decoded;

    if (ch != '\\') {
      if (pos + 1 >= buffer_size) {
        json__error("json_slice_unescape: buffer too small");
        return false;
      }
      buffer[pos++] = ch;
      i++;
      continue;
    }

    if (i + 1 >= slice.len) {
      json__error("json_slice_unescape: trailing backslash");
      return false;
    }

    if (slice.data[i + 1] == 'u') {
      unsigned long codepoint;
      unsigned long high;

      if (i + 6 > slice.len ||
          !json__parse_hex4(slice.data + i + 2, &high)) {
        json__error("json_slice_unescape: invalid \\u escape");
        return false;
      }
      i += 6;

      if (high >= 0xD800ul && high <= 0xDBFFul) {
        unsigned long low;
        if (i + 6 > slice.len || slice.data[i] != '\\' ||
            slice.data[i + 1] != 'u' ||
            !json__parse_hex4(slice.data + i + 2, &low) || low < 0xDC00ul ||
            low > 0xDFFFul) {
          json__error("json_slice_unescape: unpaired high surrogate");
          return false;
        }
        codepoint = 0x10000ul + ((high - 0xD800ul) << 10) + (low - 0xDC00ul);
        i += 6;
      } else if (high >= 0xDC00ul && high <= 0xDFFFul) {
        json__error("json_slice_unescape: unpaired low surrogate");
        return false;
      } else {
        codepoint = high;
      }

      if (!json__append_utf8(buffer, buffer_size, &pos, codepoint)) {
        json__error("json_slice_unescape: buffer too small");
        return false;
      }
      continue;
    }

    switch (slice.data[i + 1]) {
    case '"':
      decoded = '"';
      break;
    case '\\':
      decoded = '\\';
      break;
    case '/':
      decoded = '/';
      break;
    case 'b':
      decoded = '\b';
      break;
    case 'f':
      decoded = '\f';
      break;
    case 'n':
      decoded = '\n';
      break;
    case 'r':
      decoded = '\r';
      break;
    case 't':
      decoded = '\t';
      break;
    default:
      json__error("json_slice_unescape: invalid escape sequence");
      return false;
    }

    if (pos + 1 >= buffer_size) {
      json__error("json_slice_unescape: buffer too small");
      return false;
    }
    buffer[pos++] = decoded;
    i += 2;
  }

  buffer[pos] = '\0';
  if (out_len)
    *out_len = pos;
  return true;
}

bool json_getn(JsonSlice object, const char *key, size_t key_len,
               JsonSlice *out) {
  const char *cursor;
  const char *end;
  JsonSlice current_key;
  JsonSlice current_value;
  bool first;

  if (!out || !key) {
    json__error("json_getn: invalid arguments");
    return false;
  }

  *out = json_from_parts(NULL, 0);
  if (!object.data) {
    json__error("json_getn: object slice is NULL");
    return false;
  }

  cursor = json__skip_whitespace(object.data, object.data + object.len);
  end = object.data + object.len;

  if (cursor >= end || *cursor != '{') {
    json__error("json_getn: input is not an object");
    return false;
  }

  cursor++;
  first = true;
  while (cursor < end) {
    if (!json__object_next(&cursor, end, first, &current_key, &current_value))
      return false;
    first = false;

    if (current_key.len == key_len &&
        strncmp(current_key.data, key, key_len) == 0) {
      *out = current_value;
      return true;
    }
  }

  json__error("json_getn: unterminated object");
  return false;
}

bool json_get(JsonSlice object, const char *key, JsonSlice *out) {
  if (!key) {
    json__error("json_get: invalid arguments");
    return false;
  }
  return json_getn(object, key, strlen(key), out);
}

void json_object_iter_init(JsonSlice object, JsonObjectIter *iter) {
  if (!iter) {
    json__error("json_object_iter_init: iter is NULL");
    return;
  }

  iter->object = object;
  /* NULL marks "not started"; the opening brace is consumed on the first
     next() call. Deriving that state from the cursor position instead would
     misfire whenever the slice begins with whitespace. */
  iter->cursor = NULL;
}

bool json_object_iter_next(JsonObjectIter *iter, JsonSlice *key,
                           JsonSlice *value) {
  const char *cursor;
  const char *end;
  bool first;

  if (!iter || !key || !value) {
    json__error("json_object_iter_next: invalid arguments");
    return false;
  }

  *key = json_from_parts(NULL, 0);
  *value = json_from_parts(NULL, 0);
  if (!iter->object.data) {
    json__error("json_object_iter_next: object slice is NULL");
    return false;
  }

  end = iter->object.data + iter->object.len;
  first = (iter->cursor == NULL);

  if (first) {
    cursor = json__skip_whitespace(iter->object.data, end);
    if (cursor >= end || *cursor != '{') {
      json__error("json_object_iter_next: input is not an object");
      return false;
    }
    cursor++;
  } else {
    cursor = iter->cursor;
  }

  if (!json__object_next(&cursor, end, first, key, value)) {
    iter->cursor = cursor;
    return false;
  }

  iter->cursor = cursor;
  return true;
}

void json_array_iter_init(JsonSlice array, JsonArrayIter *iter) {
  if (!iter) {
    json__error("json_array_iter_init: iter is NULL");
    return;
  }

  iter->array = array;
  /* See json_object_iter_init() for why this is NULL rather than array.data. */
  iter->cursor = NULL;
}

bool json_array_iter_next(JsonArrayIter *iter, JsonSlice *out) {
  const char *cursor;
  const char *end;
  bool first;

  if (!iter || !out) {
    json__error("json_array_iter_next: invalid arguments");
    return false;
  }

  *out = json_from_parts(NULL, 0);
  if (!iter->array.data) {
    json__error("json_array_iter_next: array slice is NULL");
    return false;
  }

  end = iter->array.data + iter->array.len;
  first = (iter->cursor == NULL);

  if (first) {
    cursor = json__skip_whitespace(iter->array.data, end);
    if (cursor >= end || *cursor != '[') {
      json__error("json_array_iter_next: input is not an array");
      return false;
    }
    cursor++;
  } else {
    cursor = iter->cursor;
  }

  if (!json__array_next(&cursor, end, first, out)) {
    iter->cursor = cursor;
    return false;
  }

  iter->cursor = cursor;
  return true;
}

/* ========================================================================= */
/*                       WRITER IMPLEMENTATION                               */
/* ========================================================================= */

/*
 * Return true when `len` more bytes fit at `pos` within `cap`, reserving one
 * byte for the NUL terminator.
 *
 * The addition is checked. A caller-supplied length can be large enough that
 * `pos + len` wraps, and a wrapped sum passes a plain `pos + len < cap` test,
 * which would hand memcpy an enormous size.
 */
static bool json__fits(size_t pos, size_t len, size_t cap) {
  if (len > SIZE_MAX - pos)
    return false;
  return pos + len < cap;
}

static void json__write_bytes(JsonWriter *w, const char *data, size_t len) {
  if (w->overflow || len == 0)
    return;
  if (!data) {
    w->overflow = true;
    return;
  }
  if (!json__fits(w->pos, len, w->cap)) {
    w->overflow = true;
    return;
  }
  memcpy(w->buf + w->pos, data, len);
  w->pos += len;
  w->buf[w->pos] = '\0';
}

static void json__write_char(JsonWriter *w, char ch) {
  if (w->overflow)
    return;
  if (!json__fits(w->pos, 1, w->cap)) {
    w->overflow = true;
    return;
  }
  w->buf[w->pos++] = ch;
  w->buf[w->pos] = '\0';
}

static void json__write_comma_if_needed(JsonWriter *w) {
  if (w->needs_comma & (1u << w->depth)) {
    json__write_char(w, ',');
  }
  w->needs_comma |= (1u << w->depth);
}

static void json__write_escaped_string(JsonWriter *w, const char *str,
                                       size_t len) {
  static const char hex_digits[] = "0123456789abcdef";
  size_t i;

  /* Escaping never shrinks the input, so a source at least as long as the
     whole buffer cannot possibly fit. Rejecting here bounds the read: without
     it an oversized `len` would be walked byte by byte until the buffer
     filled, reading past the end of `str`. */
  if (len >= w->cap) {
    w->overflow = true;
    return;
  }

  json__write_char(w, '"');
  for (i = 0; i < len && !w->overflow; i++) {
    unsigned char ch = (unsigned char)str[i];
    switch (ch) {
    case '"':
      json__write_bytes(w, "\\\"", 2);
      break;
    case '\\':
      json__write_bytes(w, "\\\\", 2);
      break;
    case '\b':
      json__write_bytes(w, "\\b", 2);
      break;
    case '\f':
      json__write_bytes(w, "\\f", 2);
      break;
    case '\n':
      json__write_bytes(w, "\\n", 2);
      break;
    case '\r':
      json__write_bytes(w, "\\r", 2);
      break;
    case '\t':
      json__write_bytes(w, "\\t", 2);
      break;
    default:
      if (ch < 0x20) {
        char esc[6] = {'\\', 'u', '0', '0', 0, 0};
        esc[4] = hex_digits[ch >> 4];
        esc[5] = hex_digits[ch & 0x0f];
        json__write_bytes(w, esc, 6);
      } else {
        json__write_char(w, (char)ch);
      }
      break;
    }
  }
  json__write_char(w, '"');
}

static void json__write_long(JsonWriter *w, unsigned long val, bool negative) {
  char tmp[21];
  int pos = (int)sizeof(tmp);

  if (val == 0) {
    json__write_char(w, '0');
    return;
  }

  while (val > 0) {
    tmp[--pos] = (char)('0' + (val % 10));
    val /= 10;
  }

  if (negative)
    tmp[--pos] = '-';

  json__write_bytes(w, tmp + pos, (size_t)(sizeof(tmp) - (size_t)pos));
}

void json_writer_init(JsonWriter *w, char *buf, size_t cap) {
  if (!w)
    return;
  w->buf = buf;
  w->cap = cap;
  w->pos = 0;
  w->overflow = (buf == NULL || cap == 0);
  w->depth = 0;
  w->needs_comma = 0;
  w->is_array = 0;
  if (buf && cap > 0)
    buf[0] = '\0';
}

size_t json_writer_len(const JsonWriter *w) { return w ? w->pos : 0; }

bool json_writer_ok(const JsonWriter *w) { return w && !w->overflow; }

const char *json_writer_output(const JsonWriter *w) {
  if (!w || w->overflow)
    return NULL;
  return w->buf;
}

void json_write_null(JsonWriter *w) {
  if (!w)
    return;
  json__write_comma_if_needed(w);
  json__write_bytes(w, "null", 4);
}

void json_write_bool(JsonWriter *w, bool value) {
  if (!w)
    return;
  json__write_comma_if_needed(w);
  if (value)
    json__write_bytes(w, "true", 4);
  else
    json__write_bytes(w, "false", 5);
}

void json_write_int(JsonWriter *w, long value) {
  if (!w)
    return;
  json__write_comma_if_needed(w);
  if (value < 0)
    json__write_long(w, (unsigned long)(-(value + 1)) + 1, true);
  else
    json__write_long(w, (unsigned long)value, false);
}

void json_write_uint(JsonWriter *w, unsigned long value) {
  if (!w)
    return;
  json__write_comma_if_needed(w);
  json__write_long(w, value, false);
}

#ifndef JSON_NO_STDIO
/*
 * Print `value` as the shortest decimal string that strtod() maps back to the
 * exact same double, then normalize the locale's decimal separator to '.'.
 */
static void json__write_double(JsonWriter *w, double value) {
  char tmp[48];
  char out[48];
  size_t len = 0;
  size_t out_len = 0;
  size_t i;
  bool at_point = false;
  int prec;

  /* Catches NaN (every comparison against it is false) and both infinities in
     a single test, without pulling in <math.h>. */
  if (!(value >= -DBL_MAX && value <= DBL_MAX)) {
    json__write_bytes(w, "null", 4);
    return;
  }

  /* 17 significant digits always round-trip a double, so this loop always
     terminates. Starting at 1 rather than 15 is what makes 5e-324 print as
     "5e-324" instead of "4.94065645841247e-324": %g only strips trailing
     zeroes, so a high starting precision can round-trip exactly while still
     being far from the shortest spelling. Typical values (1.5, 51.5074) exit
     after a handful of iterations. */
  for (prec = 1; prec <= 17; prec++) {
    int written = snprintf(tmp, sizeof(tmp), "%.*g", prec, value);
    if (written <= 0 || (size_t)written >= sizeof(tmp))
      continue;
    len = (size_t)written;
    if (strtod(tmp, NULL) == value)
      break;
  }

  /* Unreachable for a finite double, but never emit a truncated number. */
  if (len == 0) {
    json__write_bytes(w, "null", 4);
    return;
  }

  /* %g emits the locale's decimal separator, which may be ',' or in principle
     several bytes. A finite %g result contains exactly one such run, so
     collapsing every non-JSON-number byte run to a single '.' is enough. */
  for (i = 0; i < len; i++) {
    char ch = tmp[i];
    if ((ch >= '0' && ch <= '9') || ch == '-' || ch == '+' || ch == 'e' ||
        ch == 'E') {
      out[out_len++] = ch;
      at_point = false;
    } else if (!at_point) {
      out[out_len++] = '.';
      at_point = true;
    }
  }

  json__write_bytes(w, out, out_len);
}

void json_write_double(JsonWriter *w, double value) {
  if (!w)
    return;
  json__write_comma_if_needed(w);
  json__write_double(w, value);
}

void json_write_double_kv(JsonWriter *w, const char *key, double value) {
  if (!w || !key)
    return;
  json_write_key(w, key);
  json_write_double(w, value);
}
#endif

void json_write_str(JsonWriter *w, const char *str) {
  if (!w)
    return;
  json__write_comma_if_needed(w);
  if (!str) {
    json__write_bytes(w, "null", 4);
    return;
  }
  json__write_escaped_string(w, str, strlen(str));
}

void json_write_str_kv(JsonWriter *w, const char *key, const char *value) {
  if (!w || !key)
    return;
  json_write_key(w, key);
  json_write_str(w, value);
}

void json_write_null_kv(JsonWriter *w, const char *key) {
  if (!w || !key)
    return;
  json_write_key(w, key);
  json_write_null(w);
}

void json_write_bool_kv(JsonWriter *w, const char *key, bool value) {
  if (!w || !key)
    return;
  json_write_key(w, key);
  json_write_bool(w, value);
}

void json_write_int_kv(JsonWriter *w, const char *key, long value) {
  if (!w || !key)
    return;
  json_write_key(w, key);
  json_write_int(w, value);
}

void json_write_uint_kv(JsonWriter *w, const char *key, unsigned long value) {
  if (!w || !key)
    return;
  json_write_key(w, key);
  json_write_uint(w, value);
}

void json_write_strn(JsonWriter *w, const char *str, size_t len) {
  if (!w)
    return;
  json__write_comma_if_needed(w);
  if (!str) {
    json__write_bytes(w, "null", 4);
    return;
  }
  json__write_escaped_string(w, str, len);
}

void json_write_strn_kv(JsonWriter *w, const char *key, const char *value,
                        size_t len) {
  if (!w || !key)
    return;
  json_write_key(w, key);
  json_write_strn(w, value, len);
}

void json_write_raw(JsonWriter *w, const char *raw, size_t len) {
  if (!w)
    return;
  json__write_comma_if_needed(w);
  json__write_bytes(w, raw, len);
}

void json_write_raw_kv(JsonWriter *w, const char *key, const char *raw,
                       size_t len) {
  if (!w || !key)
    return;
  json_write_key(w, key);
  json_write_raw(w, raw, len);
}

/*
 * Enter a nested container, or flag overflow if that would exceed the depth
 * the comma/type bitmasks can represent. Without this guard `1u << w->depth`
 * is undefined behaviour past 31 and the masks silently alias depth 0.
 */
static bool json__writer_push(JsonWriter *w, bool is_array) {
  if (w->overflow)
    return false;
  if (w->depth >= JSON_WRITER_MAX_DEPTH) {
    json__error("json_writer: maximum nesting depth exceeded");
    w->overflow = true;
    return false;
  }
  w->depth++;
  w->needs_comma &= ~(1u << w->depth);
  if (is_array)
    w->is_array |= (1u << w->depth);
  else
    w->is_array &= ~(1u << w->depth);
  return true;
}

/* Leave a container, verifying it was opened with the matching delimiter. */
static bool json__writer_pop(JsonWriter *w, bool is_array) {
  if (w->overflow)
    return false;
  if (w->depth == 0) {
    json__error("json_writer: unbalanced container end");
    w->overflow = true;
    return false;
  }
  if (((w->is_array >> w->depth) & 1u) != (is_array ? 1u : 0u)) {
    json__error("json_writer: mismatched container end");
    w->overflow = true;
    return false;
  }
  w->needs_comma &= ~(1u << w->depth);
  w->depth--;
  return true;
}

void json_write_object_begin(JsonWriter *w) {
  if (!w)
    return;
  json__write_comma_if_needed(w);
  json__write_char(w, '{');
  json__writer_push(w, false);
}

void json_write_object_begin_k(JsonWriter *w, const char *key) {
  if (!w || !key)
    return;
  json_write_key(w, key);
  json_write_object_begin(w);
}

void json_write_object_end(JsonWriter *w) {
  if (!w)
    return;
  if (!json__writer_pop(w, false))
    return;
  json__write_char(w, '}');
}

void json_write_key(JsonWriter *w, const char *key) {
  if (!w || !key)
    return;
  json__write_comma_if_needed(w);
  json__write_escaped_string(w, key, strlen(key));
  json__write_char(w, ':');
  w->needs_comma &= ~(1u << w->depth);
}

void json_write_array_begin(JsonWriter *w) {
  if (!w)
    return;
  json__write_comma_if_needed(w);
  json__write_char(w, '[');
  json__writer_push(w, true);
}

void json_write_array_begin_k(JsonWriter *w, const char *key) {
  if (!w || !key)
    return;
  json_write_key(w, key);
  json_write_array_begin(w);
}

void json_write_array_end(JsonWriter *w) {
  if (!w)
    return;
  if (!json__writer_pop(w, true))
    return;
  json__write_char(w, ']');
}

#ifdef __cplusplus
}
#endif

#endif /* LIBJSON_IMPLEMENTATION */