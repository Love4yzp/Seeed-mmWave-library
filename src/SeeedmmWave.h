#ifndef SEEEDMMWAVE_H
#define SEEEDMMWAVE_H

#include <Arduino.h>
#include <stdint.h>

#ifdef ESP_ARDUINO_VERSION_MAJOR
#  if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
// Code for version 3.x
#  else
// Code for version 2.x
#  endif
#else
// Code for version 1.x
#endif

#ifndef ESP32
#  error "Seeed_Arduino_mmWave targets Seeed XIAO ESP32 series with Seeed mmWave modules."
#endif

#include <functional>
#include <memory>
#include <queue>

#include "Status.h"

#ifndef _MMWAVE_DEBUG
  #define _MMWAVE_DEBUG 0
#endif

#ifndef _UART_BAUD
#  define _UART_BAUD 115200
#else
#  warning "Notice: the uart baud of mmWave serial should be 115200"
#endif

#define FRAME_BUFFER_SIZE 512
#define SOF_BYTE          0x01

// Frame structure sizes
#define SIZE_SOF        1
#define SIZE_ID         2
#define SIZE_LEN        2
#define SIZE_TYPE       2
#define SIZE_HEAD_CKSUM 1
#define SIZE_FRAME_HEADER                                                      \
  (SIZE_SOF + SIZE_ID + SIZE_LEN + SIZE_TYPE + SIZE_HEAD_CKSUM)
#define SIZE_DATA_CKSUM 1

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define SEEED_WAVE_IS_BIG_ENDIAN 1
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define SEEED_WAVE_IS_BIG_ENDIAN 0
#else
#  warning "Unable to determine the size end system"
#endif

#ifndef MMWAVE_QUEUE_CAPACITY
#  define MMWAVE_QUEUE_CAPACITY 120
#endif
// Back-compat alias; prefer MMWAVE_QUEUE_CAPACITY in new code.
#define MMWaveMaxQueueSize MMWAVE_QUEUE_CAPACITY

class SeeedmmWave {
 public:
  struct Stats {
    uint32_t framesRx            = 0;  // Frames that parsed successfully.
    uint32_t checksumErrors      = 0;  // Header or data checksum failures.
    uint32_t droppedByQueueFull  = 0;  // Oldest frame discarded because queue filled.
    uint32_t bytesRx             = 0;  // Total bytes pulled from the UART.
    seeed::mmwave::Status lastError = seeed::mmwave::Status::Ok;
  };

  using FrameCallback = std::function<void(uint16_t type, const uint8_t* data, size_t len)>;
  using ErrorCallback = std::function<void(seeed::mmwave::Status status)>;

 private:
  HardwareSerial* _serial;
  uint32_t _baud;
  uint32_t _wait_delay;

  std::queue<std::vector<uint8_t>> byteQueue;

  Stats _stats{};
  FrameCallback _onFrame;
  ErrorCallback _onError;

 protected:
  // Fire frame-level callback + increment framesRx. Called by processFrame
  // after a successful handleType dispatch; subclasses normally do not need
  // to call this directly.
  void fireFrame(uint16_t type, const uint8_t* data, size_t len);
  void fireError(seeed::mmwave::Status s);
  size_t expectedFrameLength(const std::vector<uint8_t>& buffer);
  uint8_t calculateChecksum(const uint8_t* data, size_t len);
  bool validateChecksum(const uint8_t* data, size_t len,
                        uint8_t expected_checksum);
  float extractFloat(const uint8_t* bytes) const;
  uint32_t extractU32(const uint8_t* bytes) const;
  void floatToBytes(float value, uint8_t* bytes);
  void uint32ToBytes(uint32_t value, uint8_t* bytes);

  bool processFrame(const uint8_t* frame_bytes, size_t len,
                    uint16_t data_type = 0xFFFF);
  /**
   * @brief Handle different types of data frames.
   *
   * This is a pure virtual function that must be overridden by derived classes
   * to handle specific types of data frames. The function processes the data
   * based on the type and data length, and performs necessary actions.
   *
   * @param _type The type of the data frame.
   * @param data The pointer to the data buffer.
   * @param data_len The length of the data buffer.
   * @return true if the data is processed successfully, false otherwise.
   */
  virtual bool handleType(uint16_t _type, const uint8_t* data,
                          size_t data_len) = 0;

  std::vector<uint8_t> packetFrame(uint16_t type, const uint8_t* data = nullptr,
                                   size_t len = 0);
  bool sendFrame(const std::vector<uint8_t>& frame);

  // Frame-pump internals. Subclasses use fetchType() for command-response
  // round-trips. User sketches should call the public update() instead.
  void fetch(uint32_t timeout = 1000);
  bool fetchType(uint16_t data_type = 0xFFFF, uint32_t timeout = 1000);
  bool processQueuedFrames(uint16_t data_type = 0xFFFF,
                           uint32_t timeout   = 1000);

 public:
  SeeedmmWave() {}
  virtual ~SeeedmmWave() {
    if (_serial) {
      _serial->end();
      _serial = nullptr;
    }
  }

  void begin(HardwareSerial* serial, uint32_t baud = _UART_BAUD,
             uint32_t wait_delay = 1, int rst = -1);
  int available();
  int read(void);
  int read(char* data, int length);
  size_t write(const uint8_t* buffer, size_t size);
  size_t write(const char* buffer, size_t size);

  /**
   * @brief Send a frame of data.
   *
   * @attention This function constructs and sends a frame of data, including
   * the frame header, data, and checksums.
   *
   * @param type The type of the frame.
   * @param data The data to include in the frame. Defaults to nullptr.
   * @param len The length of the data. Defaults to 0.
   * @retval True if the frame is sent successfully
   * @retval False otherwise.
   */
  bool update(uint32_t timeout = 100);
  bool send(uint16_t type, const uint8_t* data = nullptr, size_t data_len = 0);

  // --- v2 event subscriptions (additive; polling getters still work) ----

  // Fires on every frame that passes both checksums and is dispatched to the
  // device subclass. Useful for debug/sniffing or for handling custom frame
  // types the subclass does not know about.
  void onFrame(FrameCallback cb) { _onFrame = std::move(cb); }

  // Fires when a frame is dropped due to a decode failure or queue overflow.
  void onError(ErrorCallback cb) { _onError = std::move(cb); }

  // Snapshot of the running counters since begin().
  const Stats& stats() const { return _stats; }
  void resetStats() { _stats = Stats{}; }
};

void printHexBuff(const std::vector<uint8_t>& buffer);

#endif  // SEEEDMMWAVE_H
