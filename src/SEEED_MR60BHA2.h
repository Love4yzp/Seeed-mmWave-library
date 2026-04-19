/**
 * @file SEEED_MR60BHA2.h
 * @date  02 July 2024
 * @author Spencer Yan
 *
 * @note Description of the file
 *
 * @copyright © 2024, Seeed Studio
 *
 * @attention MR60BHA2 module is used to monitor breath and heart rate
 */

#ifndef SEEED_MR60BHA2_H
#define SEEED_MR60BHA2_H

#include <functional>

#include "SeeedmmWave.h"

#define MAX_TARGET_NUM    3

#define RANGE_STEP 17.28f

enum class TypeHeartBreath : uint16_t {
  TypeHeartBreathPhase    = 0x0A13,
  TypeBreathRate          = 0x0A14,
  TypeHeartRate           = 0x0A15,
  TypeHeartBreathDistance = 0x0A16,
  Report3DPointCloudDetection   = 0x0A08,
  Report3DPointCloudTargetInfo = 0x0A04,
  ReportHumanDetection       = 0x0F09,
  ReportFirmware          = 0xFFFF,
};

typedef struct HeartBreath {
  float total_phase;
  float breath_phase;
  float heart_phase;
} HeartBreath;

typedef struct TargetN {
  float x_point;
  float y_point;
  int32_t dop_index;
  int32_t cluster_index;
} TargetN;

typedef struct FirmwareVersion {
  uint8_t project_name;
  uint8_t major_version;
  uint8_t sub_version;
  uint8_t modified_version;
} FirmwareVersion;

typedef union FirmwareInfo {
  FirmwareVersion firmware_verson;
  uint32_t value;
};

typedef struct PeopleCounting {
  std::vector<TargetN> targets;
} PeopleCounting;

class SEEED_MR60BHA2 : public SeeedmmWave {
 private:
  /* HeartBreath */
  HeartBreath _heart_breath = {0};

  /* BreathRate */
  float _breath_rate;

  /* HeartRate */
  float _heart_rate;

  /* HeartBreathDistance */
  uint32_t _rangeFlag;
  float _range;

  /* HumanDetection */
  bool _isHumanDetected;             // 0 : no one            1 : There is someone
  bool _isHumanDetectionValid;

  /* PeopleCounting PointCloud */
  PeopleCounting _people_counting_point_cloud;
  bool _isPeopleCountingPointCloudValid;
 
  /* PeopleCounting TargetInfo */
  PeopleCounting _people_counting_target_info;
  bool _isPeopleCountingTargetInfoValid;


  FirmwareInfo _firmware_info;
  bool _isFirmwareInfoValid     = false;

  bool _isHeartBreathPhaseValid = false;
  bool _isBreathRateValid       = false;
  bool _isHeartRateValid        = false;
  bool _isDistanceValid         = false;

 public:
  SEEED_MR60BHA2() {}

  virtual ~SEEED_MR60BHA2() {}

  bool handleType(uint16_t _type, const uint8_t* data,
                  size_t data_len) override;

  bool getHeartBreathPhases(float& total_phase, float& breath_phase,
                            float& heart_phase);
  bool getBreathRate(float& rate);
  bool getHeartRate(float& rate);
  bool getDistance(float& distance);
  bool getPeopleCountingPointCloud(PeopleCounting& point_cloud);
  bool getPeopleCountingTargetInfo(PeopleCounting& target_info);
  bool isHumanDetected();
  bool getFirmwareInfo(FirmwareInfo& firmware_info);

  // --- v2 event subscriptions (additive) ------------------------------
  //
  // Each callback fires from within update() whenever a frame of the
  // corresponding type parses successfully. Handlers run on the same task
  // that calls update(); do not block, do not call update() from inside.
  //
  using HeartBreathPhasesCallback = std::function<void(float, float, float)>;
  using FloatCallback             = std::function<void(float)>;
  using BoolCallback              = std::function<void(bool)>;
  using PeopleCountingCallback    = std::function<void(const PeopleCounting&)>;
  using FirmwareInfoCallback      = std::function<void(const FirmwareInfo&)>;

  void onHeartBreathPhases(HeartBreathPhasesCallback cb) { _onHeartBreathPhases = std::move(cb); }
  void onBreathRate(FloatCallback cb)                    { _onBreathRate        = std::move(cb); }
  void onHeartRate(FloatCallback cb)                     { _onHeartRate         = std::move(cb); }
  void onDistance(FloatCallback cb)                      { _onDistance          = std::move(cb); }
  void onPresence(BoolCallback cb)                       { _onPresence          = std::move(cb); }
  void onPointCloud(PeopleCountingCallback cb)           { _onPointCloud        = std::move(cb); }
  void onTargetInfo(PeopleCountingCallback cb)           { _onTargetInfo        = std::move(cb); }
  void onFirmwareInfo(FirmwareInfoCallback cb)           { _onFirmwareInfo      = std::move(cb); }

 private:
  HeartBreathPhasesCallback _onHeartBreathPhases;
  FloatCallback             _onBreathRate;
  FloatCallback             _onHeartRate;
  FloatCallback             _onDistance;
  BoolCallback              _onPresence;
  PeopleCountingCallback    _onPointCloud;
  PeopleCountingCallback    _onTargetInfo;
  FirmwareInfoCallback      _onFirmwareInfo;
};

#endif /*SEEED_MR60BHA2_H*/