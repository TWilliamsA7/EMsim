#ifndef __Vec3_H__
#define __Vec3_H__

#include <cmath>
#include <string>

template <typename t> class Vec3 {
    public:
        t x, y, z;
        
        // Class Constructors
        Vec3() {x=0; y=0; z=0;}
        Vec3(t _x, t _y, t _z) {x = _x; y = _y; z = _z;}

        // Operators
        inline Vec3<t> operator + (const Vec3<t> &V) const { return Vec3<t>(x + V.x, y + V.y, z + V.z); }
        inline Vec3<t> operator - (const Vec3<t> &V) const { return Vec3<t>(x - V.x, y - V.y, z - V.z); }
        inline Vec3<t> operator * (const t &s) const {return Vec3<t> (s * x, s * y, s * z); }

        // Class Methods
        Vec3<t> cross(Vec3<t> &V) { return Vec3<t>(this->y*V.z - this->z*V.y, -this->x*V.z + this->z*V.x, this->x*V.y-this->y*V.x); }
        
        t dot(Vec3<t> &V) { return (this->x*V.x + this->y*V.y + this->z*V.z); }

        t magnitude(){ return sqrt(this->x * this->x + this->y * this->y + this->z * this->z); }

        Vec3<t> normalize() { return (*this * (float) (1 / this->magnitude())); }

        std::string toString(){ return "("+std::to_string(this->x)+", "+std::to_string(this->y)+", "+std::to_string(this->z)+")"; }

        // Cycle to the right
        Vec3<t> cycle() { return Vec3<t>(this->z, this->x, this->y); }
        Vec3<t> negate(bool x, bool y, bool z) { return Vec3<t>(
            (x) ? -this->x : this->x, (y) ? -this->y : this->y, (z) ? -this->z : this->z 
        );}

    private:
};

typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

#endif // __Vec3_H__