
# Photon client & server communication design

## 0. Terms

### 0.1 Channel

A `Channel` is a logical connection. A channel is a stream.

All the `Message` of a `Channel` have a unique `Message ID`.

**NOTE**: The `Channel` here may be reliable or unreliable, i.e., the a certain unimportant `Message` can be ignored.

### 0.2 Message

A `Message` is a unit of communication, which represents a certain message or a certain information, it is relatively an efficient information unit that can be used to accomplish a small piece of high level logic. E.g., a video frame is a message, while a `Chunk` of a video frame is not, since high level logic has no idea about how to process a `Chunk`.

### 0.3 Chunk

A `Chunk` is a small piece of `Message`. A `Message` may be divided into one or more `Chunks`.

In the case we use a single transport (such as TCP) connection for more than one channel, for those large `Message` we may want to split the `Message` into serveral chunks so that sending this `Message` would not make the messages in other `Channel`s be blocked for so long.

### 0.4 Abstract Connection

A `Abstract Connection` is one or more connections shares same `Base Time` for timestaping.

**NOTE**: A `Abstract Connection` may be a single connection, but may also be several connection.

### 0.5 Base Time

If two connection share a same `Base Time`, then if their payloads have same physical world timestamp, their payloads have same in-message timestamp.

### 0.6

## 1. Goals

1. Synchronizing different channel should be easy.

2. The communication protocol can be base on a reliable or unreliable protocol without changing high level logic.


3. QoS feed back

4. Security

## 2. Protocol design overview

### 2.1 Synchronizing different channel should be easy.

**All command streams and media streams share one `Abstract Connection`.**

By ensuring this, the endpoint kowns how to synchronize each channel's message if necessary, for an example, to synchronize audio and video stream.

### 2.2 The communication protocol can be base on a reliable or unreliable protocol without changing high level logic.

...


### 2.3 QoS feed back

### 2.4 Security

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
|Message Header| - | |
|Chunk ID| DUI[4] | |
| Chunk Size | DUI[3] | Bytes |
| Chunk Data | raw | ${ChunkSize} bytes |

**NOTE**: Channel ID `0` is reserved for protocol usage, other IDs can be used by use.

### 3.2 The message header is described below:

| Field | Encoding | Note |
| --- | --- | --- |
| Message ID | DUI[3] | I think Uint16 may be not large enough. Although this field is not very likely to be a small number, for DUI[3] is large enough, and we can save 1 or 2 bytes in some cases |
| Timestamp | DUI[4] | Milliseconds |
| Importance | 3 bit | |
| Message Type | 5 bit | |
| Message Length | DUI[4] | Bytes |



**Importance levels**
| Level | Describe | Note |
|---|---| --- |
| 0 | ingoreable | This message is ignoreable if the receiver didn't get it |
| 1 | important | This message is important, the receiver should better wait for it |
| 2 | very important | This message is very important, the receiver must reply an ACK for this message. If the sender does not get the ACK, the sender will send this message again. Before complete this message, all the sender's activity should be paused. |
| other | reserved | |

**Message types**
| Enum | Describe | Note |
| --- | --- | --- |
| 0   | Control Message | Control Message is used to set the configuration of this channel |
| 1   | Remote Method Invoke | a.k.a. Remote Process Call |
| 2   | Video Message | This message is a video frame or related parameters |
| 3   | Audio Message | This message is an audio message |
| 5   | QoS feedback  |  |


### 3.3 Summary

The protocol layout as following:

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



```


## 4. Messages in detail

### 4.1 Control Message



