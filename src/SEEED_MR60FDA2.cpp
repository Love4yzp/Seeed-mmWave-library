/**
 * @file SEEED_MR60FDA2.cpp
 * @date  02 July 2024

 * @author Spencer Yan
 *
 * @note Description of the file
 *
 * @copyright © 2024, Seeed Studio
 */

#include "SEEED_MR60FDA2.h"

/**
 * @brief Radar initialization.
 *
 * @details This function initializes the radar with specific settings.
 *
 * @attention The following parameters should be set:
 * - High: 2.2
 * - Threshold: 0.5
 * - Sensitivity: 3
 * - Rect_XL: 0.5
 * - Rect_XR: 0.5
 * - Rect_ZF: 0.5
 * - Rect_ZB: 0.5
 */
bool SEEED_MR60FDA2::resetSetting(void) {
  uint16_t type = static_cast<uint16_t>(TypeFallDetection::RadarInitSetting);
  if (this->send(type, nullptr, 0)) {
    return true;
  }
  return false;
}

/**
 * @brief Set the radar installation height
 *
 * @param height 1 to 5 m(not include 1.0m)
 * @retval true Set height successfully
 * @retval false Failed to set altitude
 *
 */
bool SEEED_MR60FDA2::setInstallationHeight(const float height) {
  uint16_t type = static_cast<uint16_t>(TypeFallDetection::InstallationHeight);

  uint8_t data[sizeof(float)] = {0};
  floatToBytes(height, data);
  if (this->send(type, data, sizeof(data))) {
    return this->fetchType(type, 1000);
  }
  return false;
}

/**
 * @brief Set fall threshold
 *
 * @param threshold
 *
 * @note The default fall threshold of the radar is 0.6 m.
 */
bool SEEED_MR60FDA2::setThreshold(const float threshold) {
  uint16_t type = static_cast<uint16_t>(TypeFallDetection::FallThreshold);

  uint8_t data[sizeof(float)];
  floatToBytes(threshold, data);
  if (this->send(type, data, sizeof(data))) {
    if (fetchType(type, 1000))  // Timeout of 5000 ms to fetch the response
    {
      return _isThresholdValid;
    }
  }
  return false;
}

/**
 * @brief Set the fall sensitivity
 *
 * @param _sensitivity  range 3 to 10
 * @retval true Set successfully
 * @retval false Failed to set
 *
 * @note the initial value is 3, which represents an average of 3 frames of
 * data.
 */
bool SEEED_MR60FDA2::setSensitivity(const uint32_t _sensitivity) {
  uint16_t type = static_cast<uint16_t>(TypeFallDetection::FallSensitivity);

  uint8_t data[sizeof(uint32_t)];
  uint32ToBytes(_sensitivity, data);
  if (this->send(type, data, sizeof(data))) {
    if (fetchType(type, 1000))  // Timeout of 5000 ms to fetch the response
    {
      return _isSensitivityValid;
    }
  }
  return false;
}

/**
 * @brief Set the Alam Area object
 *
 * @param rect_XL
 * @param rect_XR
 * @param rect_ZF
 * @param rect_ZB
 * @retval true Set successfully
 * @retval false Failed to set
 */
bool SEEED_MR60FDA2::setAlamArea(const float rect_XL, const float rect_XR,
                                 const float rect_ZF, const float rect_ZB) {
  uint8_t data[sizeof(float) * 4];
  floatToBytes(rect_XL, data);
  floatToBytes(rect_XR, data + sizeof(float));
  floatToBytes(rect_ZF, data + 2 * sizeof(float));
  floatToBytes(rect_ZB, data + 3 * sizeof(float));
  uint16_t type = static_cast<uint16_t>(TypeFallDetection::AlarmParameters);
  if (this->send(type, data, sizeof(data))) {
    if (fetchType(type, 1000)) {
      return _isAlarmAreaValid;
    }
  }
  return false;
}

/**
 * @brief get the basic Radar parameters
 *
 * @param height
 * @param threshold
 * @param sensitivity
 * @retval true acquisition is successful
 * @retval false failed to obtain
 */
bool SEEED_MR60FDA2::getRadarParameters(float& height, float& threshold,
                                        uint32_t& sensitivity) {
  this->getRadarParameters();
  if (_parametersValid) {
    height           = _height;
    threshold        = _thershold;
    sensitivity      = _sensitivity;
    _parametersValid = false;
    return true;
  }
  return false;
}

/**
 * @brief get whole Radar parameteres
 *
 * @param height
 * @param threshold
 * @param sensitivity
 * @param rect_XL
 * @param rect_XR
 * @param rect_ZF
 * @param rect_ZB
 * @retval true acquisition is successful
 * @retval false failed to obtain
 */
bool SEEED_MR60FDA2::getRadarParameters(float& height, float& threshold,
                                        uint32_t& sensitivity, float& rect_XL,
                                        float& rect_XR, float& rect_ZF,
                                        float& rect_ZB) {
  this->getRadarParameters();
  if (_parametersValid) {
    height           = _height;
    threshold        = _thershold;
    sensitivity      = _sensitivity;
    rect_XL          = _rect_XL;
    rect_XR          = _rect_XR;
    rect_ZF          = _rect_ZF;
    rect_ZB          = _rect_ZB;
    _parametersValid = false;
    return true;
  }
  return false;
}

/**
 * @brief Trigger Rader to return the parameters
 *
 * @retval true acquisition is successful
 * @retval false failed to obtain
 */
bool SEEED_MR60FDA2::getRadarParameters() {
  uint16_t type = static_cast<uint16_t>(TypeFallDetection::RadarParameters);
  if (this->send(type, nullptr, 0)) {
    return fetchType(type, 1000);
  }
  return false;
}

// Only supports one-way data transmission mode
bool SEEED_MR60FDA2::setUserLog(bool flag) {
  uint16_t type = static_cast<uint16_t>(TypeFallDetection::UserLogInfo);

  uint8_t data[sizeof(uint32_t)] = {0};
  uint32ToBytes(flag, data);
  std::vector<uint8_t> frame = packetFrame(type, data, sizeof(data));
  if (this->send(type, data, sizeof(data))) {
    return true;
  }
  return false;
}

/**
 * @brief Determine whether a fall has been detected.
 *
 * @param fall_status A reference to a boolean variable where the fall status
 * will be stored.
 *                    - `true` indicates a fall has been detected.
 *                    - `false` indicates no fall has been detected.
 *
 * @retval true The function executed successfully and the fall status has been
 * updated.
 * @retval false The function failed to execute.
 */
bool SEEED_MR60FDA2::getFall(bool &is_fall) {
  // if (!_isFallValid)
  //   return false;
  // _isFallValid = false;
  is_fall = _isFall;
  return is_fall;
}
bool SEEED_MR60FDA2::getFall() {
  return _isFall;
}

/**
 * @brief Determine whether a human is present.
 *
 * @param human_status A reference to a boolean variable where the human
 * presence status will be stored.
 *                     - `false` indicates no human is present.
 *                     - `true` indicates a human is present.
 *
 * @retval true A human is detected.
 * @retval false No human is detected.
 */
bool SEEED_MR60FDA2::getHuman(bool &is_human) {
   if (!_isHumanValid)
     return false;
  _isHumanValid = false;
  is_human = _isHuman;
  return is_human;
}
bool SEEED_MR60FDA2::getHuman() {
  if (!_isHumanValid)
     return false;
  _isHumanValid = false;
  return _isHuman;
}

/**
 * @brief Handle different types of fall detection data.
 *
 * This function processes different types of fall detection data based on the
 * type identifier.
 *
 * @param _type The type identifier of the data.
 * @param data The pointer to the data buffer.
 * @param data_len The length of the data buffer.
 * @retval true if the data is handled successfully.
 * @retval false if there is an error in handling the data.
 */
bool SEEED_MR60FDA2::handleType(uint16_t _type, const uint8_t* data,
                                size_t data_len) {
  TypeFallDetection type = static_cast<TypeFallDetection>(_type);
  switch (type) {
    case TypeFallDetection::ReportFallDetection:
      _isFall      = *(const bool*)data;
      _isFallValid = true;
      break;
    case TypeFallDetection::ReportUnmannedDetection:
      _isHuman      = *(const uint8_t*)data;
      _isHumanValid = true;
      break;
    case TypeFallDetection::InstallationHeight: {
      if (data_len != 1)
        return false;
      _isHeightValid = *(const uint8_t*)data;
      break;
    }
    case TypeFallDetection::RadarParameters: {
      if (data_len <= 1)
        return false;
      _height          = extractFloat(data);
      _thershold       = extractFloat(data + sizeof(float));
      _sensitivity     = extractU32(data + 2 * sizeof(float));
      _rect_XL         = extractFloat(data + 3 * sizeof(float));
      _rect_XR         = extractFloat(data + 4 * sizeof(float));
      _rect_ZF         = extractFloat(data + 5 * sizeof(float));
      _rect_ZB         = extractFloat(data + 6 * sizeof(float));
      _parametersValid = true;
      break;
    }
    case TypeFallDetection::FallThreshold: {  // set fall threshold result
      if (data_len != 1)
        return false;
      _isThresholdValid = *(const uint8_t*)data;
      break;
    }
    case TypeFallDetection::AlarmParameters: {
      _isAlarmAreaValid = *(const uint8_t*)data;
      break;
    }
    case TypeFallDetection::FallSensitivity:
      _isSensitivityValid = *(const uint8_t*)data;
      break;

      
    case TypeFallDetection::Report3DPointCloudDetection: {
        int32_t target_num = (int32_t)extractI32(data);  // Extract target quantity
        data += sizeof(uint32_t);

        std::vector<TargetN> received_targets; // Used to store parsed target data
        received_targets.reserve(target_num);
  
        for(size_t i = 0; i < target_num; i++)
        {
          TargetN target;
          target.cluster_index = extractI32(data);
          data += sizeof(int32_t);

          target.x_point = extractFloat(data);
          data += sizeof(float);
  
          target.y_point = extractFloat(data);
          data += sizeof(float);

          target.z_point = extractFloat(data);
          data += sizeof(float);
  
          target.dop_index = extractFloat(data);
          data += sizeof(float);
  
          
  
          received_targets.push_back(target); // Add the resolved target to the container
        }
  
        // Store the received target data in the PeopleCounting object
        _people_counting_point_cloud.targets = std::move(received_targets);
        _isPeopleCountingPointCloudValid = true;
  
        break;
      }

      case TypeFallDetection::Report3DPointCloudTartgetInfo: {
        int32_t target_num = (int32_t)extractI32(data);  // Extract target quantity
        data += sizeof(uint32_t);

        std::vector<TargetN> received_targets; // Used to store parsed target data
        received_targets.reserve(target_num);
  
        for(size_t i = 0; i < target_num; i++)
        {
          TargetN target;
          target.cluster_index = extractI32(data);
          data += sizeof(int32_t);

          target.x_point = extractFloat(data);
          data += sizeof(float);
  
          target.y_point = extractFloat(data);
          data += sizeof(float);
  
          target.z_point = extractFloat(data);
          data += sizeof(float);

          target.dop_index = extractFloat(data);
          data += sizeof(float);
  
          
  
          received_targets.push_back(target); // Add the resolved target to the container
        }
  
        // Store the received target data in the PeopleCounting object
        _people_counting_target_info.targets = std::move(received_targets);
        _isPeopleCountingTartgetInfoValid = true;
  
        break;
      }
    default:
      return false;
  }
  return true;
}

bool SEEED_MR60FDA2::getPeopleCountingPointCloud(PeopleCounting& point_cloud) {

  if (!_isPeopleCountingPointCloudValid)
    return false;
  _isPeopleCountingPointCloudValid = false;
  point_cloud = std::move(_people_counting_point_cloud);
  return true;
}

bool SEEED_MR60FDA2::getPeopleCountingTartgetInfo(PeopleCounting& target_info) {
  
  if (!_isPeopleCountingTartgetInfoValid)
    return false;
  _isPeopleCountingTartgetInfoValid = false;
  target_info = std::move(_people_counting_target_info);
  return true;
}





bool SEEED_MR60FDA2::getFallInternal() {
  if (!_isFallValid)
    return false;
  _isFallValid = false;
  return _isFall;
}