# ActivityFramework binary serialization format

This specification describes AFWS format revision 1. All offsets and lengths are
in 8-bit bytes. All multibyte fields, including the header and counts, use
big-endian byte order. Fields are consecutive, without alignment padding.

## File header

Every file starts with exactly 16 bytes, written field by field rather than as a
C++ structure.

| Offset | Bytes | Field | Encoding |
| --- | --- | --- | --- |
| 0 | 4 | Magic | `41 46 57 53` (ASCII `AFWS`) |
| 4 | 2 | Format revision | Unsigned 16-bit integer, currently `1` |
| 6 | 2 | Reserved flags | Unsigned 16-bit integer, must be `0` |
| 8 | 8 | Application schema version | Unsigned 64-bit integer, at least `1` |

The payload begins at offset 16. There is no checksum, object type identifier,
payload length, or end marker. Applications must agree on the top-level sequence
of values and their types.

## Payload encoding

| Value | Representation |
| --- | --- |
| Fixed-width integers | Their declared width; signed values use two's complement |
| `bool` | One byte: `00` or `01`; other values are rejected |
| `float`, `double` | IEEE 754 binary32/binary64 bit patterns, respectively |
| Enum | Encoding of its underlying integer type; enumerator validity is not checked |
| Supported STL container or adaptor | Unsigned 64-bit element count, then encoded elements |
| `std::string` | Unsigned 64-bit byte count, then exactly those bytes, without a terminating null |
| `std::array<T, N>` | Unsigned 64-bit count equal to `N`, then elements |
| C array `T[N]` | Exactly `N` elements, without a count |
| Pair | First, then second, without extra framing |
| Map | Count of entries, then key/value pairs |
| Reflected object | Eligible properties in reflection metadata order, without names or framing |
| Non-null raw pointer | Pointee value; no address, identity, or ownership information |

Containers use iteration order. Queues use front-to-back order; stacks use
top-to-bottom order (reversed during reconstruction); priority queues use pop
order. Unordered containers do not promise deterministic byte sequences.

Strings preserve embedded null bytes and perform no character encoding conversion.
Reading a string replaces its previous contents; a zero count clears it. Strings
use the same count checks and partial-read behavior as resizable sequence containers.

Counts are checked against the local `size_t` range and the destination's
`max_size()` before allocation or insertion. Array extents must match exactly.
These are representability checks, not an application memory quota. Allocation
can still fail for a representable count. Existing insertion/replacement and
partial-read semantics are unchanged.

For portable application schemas, use `std::intN_t`/`std::uintN_t` fields and
explicit enum underlying types. Directly serialized `size_t`, `long`, `wchar_t`,
and other platform-dependent scalar types retain their native width; the format
does not normalize them. Character encoding is an application responsibility.
Implementations require 8-bit bytes, ordinary little/big-endian storage, and
IEEE 754 32-bit float/64-bit double. Pointer-based schemas require valid existing
destination storage and cannot represent nullable values or cyclic graphs.

## Versions and compatibility

The format revision describes these byte-level encoding rules. The schema
version describes application properties and is independent of the revision.

- For writers, the constructor's `version` argument selects the schema version
  emitted. For readers, it is the **maximum schema version understood by the
  application**, defaulting to `1`. `version()` returns the actual file version
  after successful reader construction.
- Readers accept only revision 1, zero flags, and schema versions from 1 through
  the supplied maximum. Unknown magic, truncated headers, unsupported revisions,
  flags, and schemas throw `std::ios_base::failure` before payload extraction.
- A property participates when its `TA_PROPERTY(n)` version is at most the file
  schema version. Skipped properties retain the destination's existing value.
  `TA_DEFAULT_PROPERTY` starts at version 1.
- To extend a schema compatibly, give new properties a higher version and preserve
  the order, types, meaning, and version annotations of existing properties.
  Preserve the eligible property sequence for every older version, including
  inherited metadata. The reader's maximum version is a caller declaration;
  the serializer does not verify that the registered schema is complete.
- An older reader cannot skip unknown fields. Removing, reordering, or changing
  the encoding of existing fields requires application migration or a separate
  schema/decoder; increasing a version number alone is insufficient.
- Native-header files produced before AFWS revision 1 are incompatible. There is
  no automatic legacy fallback: read them using the previous implementation on a
  compatible platform, then rewrite the decoded data using the new writer.

For example, a reader constructed with maximum schema version 2 can read version
1 or 2 files; one constructed with the default maximum of 1 rejects version 2.
The former behavior of ignoring the reader constructor's version argument is
intentionally removed.

## Completion and errors

Call `writer.close()` before reporting a successful save. It checks flushing and
file closure; destructors perform best-effort cleanup without reporting errors.
`flush()` drains both buffering layers but does not guarantee physical-disk
durability. Read/write/format failures throw `std::ios_base::failure`; decoding
is not transactional and already-decoded values may remain modified.
