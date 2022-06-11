#pragma once
class ArkEntityGrabber
{
  float m_breakDistanceSq;
  float m_lastDesiredLerpVelocity;
  float m_timeUnderDesiredLerpVelocity;
  float m_totalDistanceForLerp;
  float m_lerpSpeed;
  float m_lerpObstructedSpeed;
  float m_lerpAcceleration;
  float m_lerpObstructedAcceleration;
  float m_maxTimeAllowedUnderDesiredLerpVelocity;
  bool m_bConstrained;
};
