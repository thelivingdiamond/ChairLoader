#pragma once
#include <boost/core/noncopyable.hpp>

class ArkSimpleAttentionObject : boost::noncopyable_::noncopyable
{
public:
  unsigned int DoGetEntityId() = 0;
  virtual Vec3_tpl<float> *DoGetSimpleAttentionObjectWorldPos(Vec3_tpl<float> *result) = 0;


  };
