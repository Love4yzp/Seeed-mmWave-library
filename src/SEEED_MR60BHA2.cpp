#include "SEEED_MR60BHA2.h"

/**
 * @brief Handle different types of heart and breath data.
 *
 * This function processes different types of heart and breath data based on
 * the type identifier. It first deletes any existing objects to avoid memory
 * leaks.
 *
 * @param _type The type identifier of the data.
 * @param data The pointer to the data buffer.
 * @param data_len The length of the data buffer.
 * @return true if the data is handled successfully.
 * @return false if there is an error in handling the data.
 */
bool SEEED_MR60BHA2::handleType(uint16_t _type, const uint8_t* data,
                                size_t data_len) {
  TypeHeartBreath type = static_cast<TypeHeartBreath>(_type);
  switch (type) {
    case TypeHeartBreath::TypeHeartBreathPhase: {
      _heart_breath.total_phase  = extractFloat(data);
      _heart_breath.breath_phase = extractFloat(data + sizeof(float));
      _heart_breath.heart_phase  = extractFloat(data + 2 * sizeof(float));
      _isHeartBreathPhaseValid   = true;
      if (_onHeartBreathPhases)
        _onHeartBreathPhases(_heart_breath.total_phase,
                             _heart_breath.breath_phase,
                             _heart_breath.heart_phase);
      break;
    }
    case TypeHeartBreath::TypeBreathRate: {
      _breath_rate       = extractFloat(data);
      _isBreathRateValid = true;
      if (_onBreathRate) _onBreathRate(_breath_rate);
      break;
    }
    case TypeHeartBreath::TypeHeartRate: {
      _heart_rate       = extractFloat(data);
      _isHeartRateValid = true;
      if (_onHeartRate) _onHeartRate(_heart_rate);
      break;
    }
    case TypeHeartBreath::TypeHeartBreathDistance: {
      _rangeFlag       = extractU32(data);
      _range           = extractFloat(data + sizeof(uint32_t));
      _isDistanceValid = true;
      if (_onDistance && _rangeFlag) _onDistance(_range);
      break;
    }
    case TypeHeartBreath::ReportHumanDetection: {
      _isHumanDetected = data[0];
      _isHumanDetectionValid = true;
      if (_onPresence) _onPresence(_isHumanDetected);
      break;
    }
    case TypeHeartBreath::Report3DPointCloudDetection: {
      size_t target_num = extractU32(data);  // Extract target quantity
      data += sizeof(uint32_t);

      std::vector<TargetN> received_targets; // Used to store parsed target data
      received_targets.reserve(target_num);

      for(size_t i = 0; i < target_num; i++)
      {
        TargetN target;
        target.x_point = extractFloat(data);
        data += sizeof(float);

        target.y_point = extractFloat(data);
        data += sizeof(float);

        target.dop_index = extractU32(data);
        data += sizeof(int32_t);

        target.cluster_index = extractU32(data);
        data += sizeof(int32_t);

        received_targets.push_back(target); // Add the resolved target to the container
      }

      // Store the received target data in the PeopleCounting object
      _people_counting_point_cloud.targets = std::move(received_targets);
      _isPeopleCountingPointCloudValid = true;
      if (_onPointCloud) _onPointCloud(_people_counting_point_cloud);

      break;
    }
    case TypeHeartBreath::Report3DPointCloudTargetInfo: {
      size_t target_num = extractU32(data);  // Extract target quantity
      data += sizeof(uint32_t);

      std::vector<TargetN> received_targets; // Used to store parsed target data
      received_targets.reserve(target_num);

      for(size_t i = 0; i < target_num; i++)
      {
        TargetN target;
        target.x_point = extractFloat(data);
        data += sizeof(float);

        target.y_point = extractFloat(data);
        data += sizeof(float);

        target.dop_index = extractU32(data);
        data += sizeof(int32_t);

        target.cluster_index = extractU32(data);
        data += sizeof(int32_t);

        received_targets.push_back(target); // Add the resolved target to the container
      }

      // Store the received target data in the PeopleCounting object
      _people_counting_target_info.targets = std::move(received_targets);
      _isPeopleCountingTargetInfoValid = true;
      if (_onTargetInfo) _onTargetInfo(_people_counting_target_info);

      break;
    }
    case TypeHeartBreath::ReportFirmware: {
      _firmware_info.value = extractU32(data);
      _isFirmwareInfoValid = true;
      if (_onFirmwareInfo) _onFirmwareInfo(_firmware_info);
      break;
    }
    default:
      return false;  // Unhandled type
  }
  return true;
}

bool SEEED_MR60BHA2::getHeartBreathPhases(float& total_phase,
                                          float& breath_phase,
                                          float& heart_phase) {
  if (!_isHeartBreathPhaseValid)
    return false;
  _isHeartBreathPhaseValid = false;

  total_phase  = _heart_breath.total_phase;
  breath_phase = _heart_breath.breath_phase;
  heart_phase  = _heart_breath.heart_phase;
  return true;
}

bool SEEED_MR60BHA2::getBreathRate(float& rate) {
  if (!_isBreathRateValid)
    return false;
  _isBreathRateValid = false;
  rate               = _breath_rate;
  return true;
}

bool SEEED_MR60BHA2::getHeartRate(float& rate) {
  if (!_isHeartRateValid)
    return false;
  _isHeartRateValid = false;
  rate              = _heart_rate;
  return true;
}

bool SEEED_MR60BHA2::getDistance(float& distance) {
  if (!_isDistanceValid || !_rangeFlag)
    return false;
  _isDistanceValid = false;
  distance         = _range;
  return true;
}

bool SEEED_MR60BHA2::getPeopleCountingPointCloud(PeopleCounting& point_cloud) {
  if (!_isPeopleCountingPointCloudValid)
    return false;
  _isPeopleCountingPointCloudValid = false;
  point_cloud = std::move(_people_counting_point_cloud);
  return true;
}

bool SEEED_MR60BHA2::getPeopleCountingTargetInfo(PeopleCounting& target_info) {
  if (!_isPeopleCountingTargetInfoValid)
    return false;
  _isPeopleCountingTargetInfoValid = false;
  target_info = std::move(_people_counting_target_info);
  return true;
}

bool SEEED_MR60BHA2::isHumanDetected() {
  if (!_isHumanDetectionValid)
    return false;
  _isHumanDetectionValid = false;
  return _isHumanDetected;
}

bool SEEED_MR60BHA2::getFirmwareInfo(FirmwareInfo& firmware_info) {
  if (!_isFirmwareInfoValid)
    return false;
  _isFirmwareInfoValid = false;
  firmware_info = std::move(_firmware_info);
  return true;
}

using seeed::mmwave::Status;

Status SEEED_MR60BHA2::readHeartBreathPhases(float& total, float& breath, float& heart) {
  return getHeartBreathPhases(total, breath, heart) ? Status::Ok : Status::NoData;
}

Status SEEED_MR60BHA2::readBreathRate(float& rate) {
  return getBreathRate(rate) ? Status::Ok : Status::NoData;
}

Status SEEED_MR60BHA2::readHeartRate(float& rate) {
  return getHeartRate(rate) ? Status::Ok : Status::NoData;
}

Status SEEED_MR60BHA2::readDistance(float& distance) {
  return getDistance(distance) ? Status::Ok : Status::NoData;
}

Status SEEED_MR60BHA2::readPresence(bool& present) {
  if (!_isHumanDetectionValid) return Status::NoData;
  _isHumanDetectionValid = false;
  present = _isHumanDetected;
  return Status::Ok;
}

Status SEEED_MR60BHA2::readPointCloud(PeopleCounting& point_cloud) {
  return getPeopleCountingPointCloud(point_cloud) ? Status::Ok : Status::NoData;
}

Status SEEED_MR60BHA2::readTargetInfo(PeopleCounting& target_info) {
  return getPeopleCountingTargetInfo(target_info) ? Status::Ok : Status::NoData;
}

Status SEEED_MR60BHA2::readFirmwareInfo(FirmwareInfo& firmware_info) {
  return getFirmwareInfo(firmware_info) ? Status::Ok : Status::NoData;
}
