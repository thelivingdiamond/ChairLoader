#pragma once

class StreamSpace {};
class StreamLine {};

template <class OStream>
inline OStream& operator<< (OStream& s, const StreamSpace& v) { return s; }
template <class OStream>
inline OStream& operator<< (OStream& s, const StreamLine& v) { return s; }

template <class IStream>
inline IStream& operator>> (IStream& s, const StreamSpace& v) { return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const StreamLine& v) { return s; }


template <class OStream>
inline OStream& operator<< (OStream& s, const Vect2i& v) { s << v.x << StreamSpace() << v.y; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Vect2i& v) { s >> v.x >> StreamSpace() >> v.y; return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const Vect2f& v) { s << v.x << StreamSpace() << v.y; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Vect2f& v) { s >> v.x >> StreamSpace() >> v.y; return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const Vect2s& v) { s << v.x << StreamSpace() << v.y; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Vect2s& v) { s >> v.x >> StreamSpace() >> v.y; return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const Vect3f& v) { s << v.x << StreamSpace() << v.y << StreamSpace() << v.z; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Vect3f& v) { s >> v.x >> StreamSpace() >> v.y >> StreamSpace() >> v.z; return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const Vect3d& v) { s << v.x << StreamSpace() << v.y << StreamSpace() << v.z; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Vect3d& v) { s >> v.x >> StreamSpace() >> v.y >> StreamSpace() >> v.z; return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const Vect4f& v) { s << v.x << StreamSpace() << v.y << StreamSpace() << v.z << StreamSpace() << v.w; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Vect4f& v) { s >> v.x >> StreamSpace() >> v.y >> StreamSpace() >> v.z >> StreamSpace() >> v.w; return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const Mat3f& m) { s << m.xrow() << StreamLine() << m.yrow() << StreamSpace() << m.zrow(); return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Mat3f& m) { s >> m.xrow() >> StreamLine() >> m.yrow() >> StreamLine() >> m.zrow(); return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const Mat3d& m) { s << m.xrow() << StreamLine() << m.yrow() << StreamSpace() << m.zrow(); return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Mat3d& m) { s >> m.xrow() >> StreamLine() >> m.yrow() >> StreamLine() >> m.zrow(); return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const QuatF& q) { s << q.s_ << StreamSpace() << q.x_ << StreamSpace() << q.y_ << StreamSpace() << q.z_; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const QuatF& v) { s >> q.s_ >> StreamSpace() >> q.x_ >> StreamSpace() >> q.y_ >> StreamSpace() >> q.z_; return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const QuatD& q) { s << q.s_ << StreamSpace() << q.x_ << StreamSpace() << q.y_ << StreamSpace() << q.z_; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const QuatD& v) { s >> q.s_ >> StreamSpace() >> q.x_ >> StreamSpace() >> q.y_ >> StreamSpace() >> q.z_; return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const Se3f& se3) { s << se3.q << StreamSpace() << se3.d; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Se3f& se3) { s >> se3.q >> StreamSpace() >> se3.d; return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const Se3d& se3) { s << se3.q << StreamSpace() << se3.d; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Se3d& se3) { s >> se3.q >> StreamSpace() >> se3.d; return s; }


template <class OStream>
inline OStream& operator<< (OStream& s, const Color3c& c) { s << c.r << StreamSpace() << c.g << StreamSpace() << c.b; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Color3c& c) { s >> c.r >> StreamSpace() >> c.g >> StreamSpace() >> c.b; return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const Color4c& c) { s << c.r << StreamSpace() << c.g << StreamSpace() << c.b << StreamSpace() << c.a; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Color4c& c) { s >> c.r >> StreamSpace() >> c.g >> StreamSpace() >> c.b << StreamSpace() << c.a; return s; }

template <class OStream>
inline OStream& operator<< (OStream& s, const Color4f& c) { s << c.r << StreamSpace() << c.g << StreamSpace() << c.b << StreamSpace() << c.a; return s; }
template <class IStream>
inline IStream& operator>> (IStream& s, const Color4f& c) { s >> c.r >> StreamSpace() >> c.g >> StreamSpace() >> c.b << StreamSpace() << c.a; return s; }
