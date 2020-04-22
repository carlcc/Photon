
# Photon client & server communication design

## 0. Terms

### 0.1 Channel

A `Channel` is a logical connection or a logical stream.

### 0.2 Abstract Connection

A `Abstract Connection` is one or more (logical) connections shares same `Base Time` for timestaping.

**NOTE**: A `Abstract Connection` may be a single connection, but may also be several connection.

### 0.3 Base Time

If two (logical) connection share a same `Base Time`, then if their payloads have same physical world timestamp, their payloads have same in-message timestamp.

## 1. Goals

For simplicity we use a reliable, stream-styple transport layer for our application layer protocol. (e.g. TCP, QUIC, kcp, etc.)

1. Multiple channel in a connection.
2. Synchronizing different channel should be easy.
3. Security

## 2. Protocol design overview

### 2.1 Synchronizing different channel should be easy.

**All command streams and media streams share one `Abstract Connection`.**

By ensuring this, the endpoint kowns how to synchronize each channel's message if necessary, for an example, to synchronize audio and video stream.

### 2.2 Security

Optional TLS


## 3. Protocol detailed design

### 3.0 Some more detailed design you must know before continuing.

#### 3.0.1 Dynamic Unsigned Integer Encoding

For these integer values that we allow it to be a large number, but it is very likely to be a very small number, we introduce a method named `Dynamic Unsigned Integer Encoding`, denoted as $DUI[N]$ where $N$ is the maximum bytes that can be used.

The decoding method is described below (with C++ like pseudo code), the encoding method is the opposite process:

```C++
template <uint N>
DynamicUnsignedIntegerDecoding(uint8_t *buffer)
{
    result = 0;
    for (i = 0; i < N; ++i) {
        if (i == N - 1) {
            result <<= 8;
            result |= buffer[i];
        } else {
            result <<= 7;
            result |= buffer[i] & 0x7F;
            if (buffer[i] < 0x80) {
                break;
            }
        }
    }
    return result;
}
```

The range that $DUI[N]$ is able to encode is shown below:
| N | range |
|---|---|
|1|[0, 255]|
|2|[0, 32767]|
|3|[0, 4194303]|
|N|[0, $2^{7N+1} - 1$]|

#### 3.0.2 Dynamic Signed Integer Encoding

Similar to `Dynamic Unsigned Integer Encoding`, for signed integers that very likely to have small absolute value, we introduce `Dynamic Signed Integer Encoding` to represent them.


The decoding method is described below (with C++ like pseudo code), the encoding method is the opposite process:

```C++
template <uint N>
DynamicSignedIntegerDecoding(uint8_t *buffer)
{
    result = 0;
    actual_bytes = 0;
    for (i = 0; i < N; ++i) {
        ++actual_bytes;
        if (i == N - 1) {
            result <<= 8;
            result |= buffer[i];
        } else {
            result <<= 7;
            result |= buffer[i] & 0x7F;
            if (buffer[i] < 0x80) {
                break;
            }
        }
    }
    actual_bits_plus1 = actual_bytes * 7 + 2;
    if (buffer[0] & 0x40) {
        result -= 1 << actual_bits_plus1
    }
    return result;
}
```

The range that $DSI[N]$ is able to encode is shown below:
| N | range |
|---|---|
|1|[-128, 127]|
|2|[-16384, 16383]|
|3|[-2097152, 2097151]|
|N|[$2^{7N}$, $2^{7N} - 1$]|

### 3.1 The physical connections transport the data with `Chunk`s, each chunk should have the following field:

| Field | Encoding | Note |
|---|---| --- |
|Channel ID| DUI[2] | |
|Chunk ID| DUI[4] | |
| Chunk Size | DUI[3] | Bytes |
| Chunk Data (Payload) | raw | ${ChunkSize} bytes |

**NOTE**: Channel ID `0` is reserved for protocol usage, other IDs can be used by use.

### 3.2 The message header is described below:

| Field | Encoding | Note |
| --- | --- | --- |
| Message ID | DUI[3] | I think Uint16 may be not large enough. Although this field is not very likely to be a small number, for DUI[3] is large enough, and we can save 1 or 2 bytes in some cases |
| Timestamp | DUI[4] | Milliseconds |
| Reserved | 3 bit | |
| Message Type | 5 bit | |
| Message Length | DUI[4] | Bytes |

**Message types**
| Enum | Description | Note |
| --- | --- | --- |
| 0   | Control Message | Control Message is used to set the configuration of this channel |
| 1   | Remote Method Invoke | a.k.a. Remote Process Call |
| 2   | Video Message | This message is a video frame or related parameters |
| 3   | Audio Message | This message is an audio message |


### 3.3 Summary

The protocol layout as below:

A Chunk:
```
low address                                                                                                         high adress
+------------+------------+-----------+-----------------------------+----------------+----------+------------+----------------+
| Channel ID | Message ID | Timestamp | Importance and Message type | Message Length | Chunk ID | Chunk Size |   Chunk Data   |
+------------+------------+-----------+-----------------------------+----------------+----------+------------+----------------+
   1~2 B         1~3B         1~4B            1B = 3b + 5b                1~4 B         1~4 B       1~3 B      ${ChunkSize} B
```

The stream:
```
The transport layer stream:
low address                                                                                                          high adress
+--------+--------+--------+--------+--------+--------+--------+--------+
| Chunk1 | Chunk2 | Chunk3 | Chunk4 | Chunk5 | Chunk6 | Chunk7 | Chunk8 |
+--------+--------+--------+--------+--------+--------+--------+--------+
    ^        ^        ^        ^        ^        ^        ^        ^
    |        |        |        |        |        |        |        |
    |        |        |        |        |        |        |        |
    |        |        |        |        |        |        |        |
    |        |        |        |        |        |        |        |
    +-----------------+--------------------------+-----------------+----- Channel 1
             |                 |        |                 |
             |                 |        |                 |
             |                 |        |                 |
             |                 |        |                 |
             +-----------------+--------+-----------------+-------------- Channel2


--------------------------------------------------------------------------------------------------
The logical(channel) stream:

Channel 1: | Chunk1 | Chunk3 | Chunk6 | Chunk8 |
Channel 2: | Chunk2 | Chunk4 | Chunk5 | Chunk7 |


--------------------------------------------------------------------------------------------------
Messages are lays in the logical stream one by one, e.g.

Channel 1: | Chunk1 | Chunk3 | Chunk6 | Chunk8 |
                         |
                         | The playload stream
                         V
low address                      high adress
+-----------+-----------+-----------+-----+
| Message 1 | Message 2 | Message 3 | ... |
+-----------+-----------+-----------+-----+

```


## 4. Messages in detail

### 4.1 Control Message

Control message is implemented as RMI

#### 4.1.1 Handshake HELLO

#### 4.1.2 Handshake HELLO1

#### 4.1.3 Create channel

#### 4.1.4 Configure chunk size for channel (Default chunk size is 127)

### 4.2 Remote Method Invoke(RMI) Message

#### 4.2.0 RMI basic types

##### 4.2.0.0 The basis

All types are stored after a byte indicating it's type

| Field | Encoding | Note |
| --- | --- | --- |
| `Object Type` | 1 byte | |
| Type determined fields | - | Refer to the table below |


**The `Object types` are**
| enum | type |
|--- | --- |
|1| Byte Array|
|2| String|
|3| Integer|
|4| Array|
|5| Map|
|6| int8_t|
|7| uint8_t|
|8| int16_t|
|9| uint16_t|
|10| int32_t|
|11| uint32_t|
|12| int64_t|
|13| uint64_t|
|255|Void|

##### 4.2.0.1 Byte Array

| Field | Encoding | Note |
| --- | --- | --- |
| Length | DUI[4] | |
| Data | raw | |

##### 4.2.0.2 String

| Field | Encoding | Note |
| --- | --- | --- |
| Length In bytes | DUI[4] | |
| Data | raw | UTF8 encoding |

##### 4.2.0.3 Integer

| Type(Note: This is **NOT** `Field`) | Encoding | Note |
| --- | --- | --- |
| int8_t | raw | |
| uint8_t | raw | a.k.a. byte |
| int16_t | raw | Big endian |
| uint16_t | raw | Big endian |
| int32_t | raw | Big endian |
| uint32_t | raw | Big endian |
| int64_t | raw | Big endian |
| uint64_t | raw | Big endian |

##### 4.2.0.4 Array

| Field | Encoding | Note |
| --- | --- | --- |
| Length | DUI[4] | |
| Objects | raw | embedding is allowed |


##### 4.2.0.5 K-V Array (Map)

| Field | Encoding | Note |
| --- | --- | --- |
| Length | DUI[4] | Denote the Value as $N$ | 
| Key1 | String | See 4.2.0.2 |
| Value1 | Object | See 4.2.0.0 | 
| ... | ... | ... |
| Key$N$ | String | See 4.2.0.2 |
| Value$N$ | Object | See 4.2.0.0 | 


#### 4.2.1 The RMI Message request format

| Field | Encoding | Note |
| --- | --- | --- |
|Return Type|`Object Type` enum||
|Method Full Name| String | With package name. e.g. use `com.example.foo` instead of `foo` unless you want to invoke the `foo` function in default package|
|Arguments| K-V Array | See 4.2.0.5 |


#### 4.2.2 The RMI Message response format

| Field | Encoding | Note |
| --- | --- | --- |
| The MessageID of Request | DUI[3] | see 3.2 |
| Invoke Result | 1 Byte | The following table is the enumartion of Invoke Result |
| `Invoke Result` determined data | - | - |


**`Invoke Result` enumerations**
| Enum | Description | Note |
| --- | --- | --- |
| 0 | Invoke succeed | |
| 1 | Return type mismatch |
| 2 | Paramter number or type mismatch |
| 3 | Exception occured |

In the case `Invoke Result` == 0:
`Invoke Result` determined data should be the object (see 4.2.0.0) returned or empty if the return type is `Void`.

In the case `Invoke Result` == 1 or `Invoke Result` == 2:
`Invoke Result` should be empty.

In the case `Invoke Result` == 2:
`Invoke Result` should be a string describing the exception.

### 4.3 Video Message

Video messages have the following layout:

| Field | Type | Note |
|--- |---| ---|
|`compression Algorithm`(CA) | 1 Byte | An enumeration of compression algorithms|
| Payload | - | CA determined data |

**`compression Algorithm`** enumerations:

| Enum | Description | Note |
| --- | --- | --- |
| 0 | H.264 | |
| Other values are not defined by now |  |


**The payload**
If the `CA` field is H.264(Value == 0), the payload should be NALU.


**A note about the compression algorithms**:

Some other video compression algorithm candidates are VP8, VP9, H265 and AV1, and I had tested some (VP8, VP9, AV1) of which:

1. VP8's quality is not so good while it produces same compression ratio
2. VP9's quality is good, but it's a little slow while compressing high definition (720P) pictures.
3. Although AV1 have already made a lot of significant performance optimization, but any way, it's still too slow for realtime media stream encoding.
4. H265, I haven't test it yet.

So my conclusion is: H264 seems to be the best choice to transport realtime videos by now.

#### 4.3.1 RMIs to configure video parameters

TODO

### 4.4 Audio Message

Audio messages have the following layout:

| Field | Type | Note |
|--- |---| ---|
| Compression Algorithm(CA) | 1 Byte | Compression Algorithm |
| Payload | raw | |

**`Compression Algorithm`** enumerations:
| Enum | Description | Note |
| --- | --- | --- |
| 0 | OPUS | |
| Other values are not defined by now |  |

Audio configurations such as `sample rate`, `channels count`, `sample format` and `samples per message(frame size)` can be configured using RMI on the specified channel.

#### 4.4.1 RMIs to configure audio message

```C++
package audio;
// If failed, this function should raise an exception
void SetSampleRate(uint32_t sampleRate);

// If failed, this function should raise an exception
void SetChannelCount(uint8_t channels);

enum class SampleFormat : uint8_t {
    Uint16,
    Int16,
};
// If failed, this function should raise an exception
void SetSampleFormat(SampleFormat fmt);

// If failed, this function should raise an exception
void SetFrameSize(uint16_t milliseconds);
```
