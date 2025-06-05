#ifndef SOLDUINO_SERIALIZER_H
#define SOLDUINO_SERIALIZER_H

#include <stdint.h>
#include <stddef.h>

// Maximum buffer sizes
#define SOLDUINO_MAX_SERIALIZED_SIZE 1024

#ifdef __cplusplus
extern "C" {
#endif

// Basic type serialization
bool borsh_serialize_u8(uint8_t* buffer, size_t buffer_size, size_t* offset, uint8_t value);
bool borsh_serialize_u32(uint8_t* buffer, size_t buffer_size, size_t* offset, uint32_t value);
bool borsh_serialize_u64(uint8_t* buffer, size_t buffer_size, size_t* offset, uint64_t value);
bool borsh_serialize_bytes(uint8_t* buffer, size_t buffer_size, size_t* offset, const uint8_t* bytes, size_t len);
bool borsh_serialize_string(uint8_t* buffer, size_t buffer_size, size_t* offset, const char* str);

// Basic type deserialization
bool borsh_deserialize_u8(const uint8_t* buffer, size_t buffer_size, size_t* offset, uint8_t* value);
bool borsh_deserialize_u32(const uint8_t* buffer, size_t buffer_size, size_t* offset, uint32_t* value);
bool borsh_deserialize_u64(const uint8_t* buffer, size_t buffer_size, size_t* offset, uint64_t* value);
bool borsh_deserialize_bytes(const uint8_t* buffer, size_t buffer_size, size_t* offset, uint8_t* out, size_t out_size);
bool borsh_deserialize_string(const uint8_t* buffer, size_t buffer_size, size_t* offset, char* out, size_t out_size);

#ifdef __cplusplus
}
#endif

#endif // SOLDUINO_SERIALIZER_H 