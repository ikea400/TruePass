#pragma once
#include <opaque++.h>

#include <optional>
#include <string>

#include "../utils/BinArray.h"
#include "../utils/BinVector.h"
#include "../utils/uuid.h"
#include "device_bound_dto.h"

namespace ikea400 {
struct StartRegisterRequest {
  std::string username;
  std::string email;
  BinArray<opaque::kRegisterRequestSize> registrationStart;
};

struct StartRegisterResponse {
  BinArray<opaque::kRegisterResponseSize> registrationResponse;
};

struct FinishRegisterRequest {
  BinArray<opaque::kRegisterRecordSize> registrationRecord;
  BinVector<63, 128> protectedAccountKey;
};

struct StartLoginRequest {
  std::string identifier;
  BinArray<opaque::kStartLoginRequestSize> loginRequest;
  std::optional<dto::DevicePublicKey> devicePublicKey;
};

struct StartLoginResponse {
  BinArray<opaque::kStartLoginResponseSize> loginResponse;
  std::optional<dto::DeviceBoundChallenge> deviceChallenge;
};

struct FinishLoginRequest {
  BinArray<opaque::kFinishLoginRequestSize> loginRequest;
  std::optional<dto::DeviceBoundProof> deviceProof;
};

struct FinishLoginResponse {
  BinVector<63, 128> protectedAccountKey;
  uuid userId;
};

using GetChallengeResponse = dto::DeviceBoundChallenge;

struct RefreshRequest {
  std::optional<dto::DeviceBoundProof> deviceProof;
};

}  // namespace ikea400