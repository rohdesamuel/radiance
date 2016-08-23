#ifndef VECTOR_MATH__H
#define VECTOR_MATH__H

class Vec2 {
public:
  Vec2();
  Vec2(float x, float y);
  Vec2(const Vec2 & start, const Vec2 & end);

  void normalize();
  Vec2 normal() const;

  void operator+=(const Vec2& other);
  Vec2 operator+(const Vec2& other) const;

  void operator-=(const Vec2& other);
  Vec2 operator-(const Vec2& other) const;

  // Divide by scalar.
  void operator/=(float rhs);
  Vec2 operator/(float rhs) const;

  // Multiply by scalar.
  void operator*=(float rhs);
  Vec2 operator*(float rhs) const;

  // Dot product.
  float dot(const Vec2& other) const;

  float length() const;

  union {
    float x;
    float u;
  };

  union {
    float y;
    float v;
  };
};

class Vec3 {
public:
  Vec3();
  Vec3(float x, float y, float z);
  Vec3(const Vec3 & start, const Vec3 & end);

  void normalize();
  Vec3 normal() const;

  void operator+=(const Vec3& other);
  Vec3 operator+(const Vec3& other) const;

  void operator-=(const Vec3& other);
  Vec3 operator-(const Vec3& other) const;

  // Divide by scalar.
  void operator/=(float rhs);
  Vec3 operator/(float rhs) const;

  // Multiply by scalar.
  void operator*=(float rhs);
  Vec3 operator*(float rhs) const;

  // Dot product.
  float dot(const Vec3& other) const;

  // Cross product and store in self.
  void cross(const Vec3& other);

  // Cross product and give result.
  static Vec3 cross(const Vec3& u, const Vec3& v);

  float length();

  union {
    float x;
    float r;
  };

  union {
    float y;
    float g;
  };

  union {
    float z;
    float b;
  };
};

class Vec4 {
public:
  Vec4();
  Vec4(float x, float y, float z, float w);
  Vec4(const Vec4 & start, const Vec4 & end);

  void normalize();
  Vec4 normal() const;

  void operator+=(const Vec4& other);
  Vec4 operator+(const Vec4& other) const;

  void operator-=(const Vec4& other);
  Vec4 operator-(const Vec4& other) const;

  // Divide by scalar.
  void operator/=(float rhs);
  Vec4 operator/(float rhs) const;

  // Multiply by scalar.
  void operator*=(float rhs);
  Vec4 operator*(float rhs) const;

  // Dot product.
  float dot(const Vec4& other);

  float length();

  union {
    float x;
    float r;
    float s;
    float u;
  };

  union {
    float y;
    float g;
    float t;
    float v;
  };

  union {
    float z;
    float b;
    float p;
  };

  union {
    float w;
    float a;
    float q;
  };
};

#endif /*VECTOR_MATH__H*/
