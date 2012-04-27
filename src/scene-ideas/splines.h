#ifndef SPLINES_H_
#define SPLINES_H_

#include <vector>
#include "vec.h"

class Spline
{
public:
    Spline() :
        paramData_(0) {}
    ~Spline()
    {
        delete [] paramData_;
    }
    void addControlPoint(const LibMatrix::vec3& point) { controlData_.push_back(point); }
    void getCurrentVec(float currentTime, LibMatrix::vec3& v) const;
    void calcParams();

private:
    std::vector<LibMatrix::vec3> controlData_;
    typedef LibMatrix::vec3 param[4];
    param* paramData_;
};

class ViewFromSpline : public Spline
{
public:
    ViewFromSpline();
    ~ViewFromSpline() {}
};

class ViewToSpline : public Spline
{
public:
    ViewToSpline();
    ~ViewToSpline() {}
};

class LightPositionSpline : public Spline
{
public:
    LightPositionSpline();
    ~LightPositionSpline() {}
};

class LogoPositionSpline : public Spline
{
public:
    LogoPositionSpline();
    ~LogoPositionSpline() {}
};

class LogoRotationSpline : public Spline
{
public:
    LogoRotationSpline();
    ~LogoRotationSpline() {}
};

#endif // SPLINES_H_
