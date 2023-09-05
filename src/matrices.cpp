#include <Eigen/Dense>
#include <cmath>
#include "matrices.hpp"
#include "uav_params.hpp"

using namespace Eigen;
using namespace std;

Vector<double, 6> Matrices::quaterionsToRPY(Vector<double, 7> y)
{
    Vector<double, 6> Y_RPY;
    Y_RPY.head<3>() = y.head<3>();
    const Vector4d& e = y.tail<4>();
    Y_RPY(3) = atan2(2*(e(0)*e(1)+e(2)*e(3)),e(0)*e(0)-e(1)*e(1)-e(2)*e(2)+e(3)*e(3));
    Y_RPY(4) = asin(2*(e(0)*e(2)-e(1)*e(3)));
    Y_RPY(5) = atan2(2*(e(0)*e(3)+e(1)*e(2)),e(0)*e(0)+e(1)*e(1)-e(2)*e(2)-e(3)*e(3));
    return Y_RPY;
}

Vector<double, 7> Matrices::RPYtoQuaterion(Vector<double, 6> y)
{
    Vector<double, 7> Y_quat;
    Vector4d quat;
    double halfRoll = y(3) * 0.5;
    double halfPitch = y(4) * 0.5;
    double halfYaw = y(5) * 0.5;
    double sinHalfRoll = std::sin(halfRoll);
    double cosHalfRoll = std::cos(halfRoll);
    double sinHalfPitch = std::sin(halfPitch);
    double cosHalfPitch = std::cos(halfPitch);
    double sinHalfYaw = std::sin(halfYaw);
    double cosHalfYaw = std::cos(halfYaw);

    quat(0) = cosHalfRoll * cosHalfPitch * cosHalfYaw + sinHalfRoll * sinHalfPitch * sinHalfYaw; // w
    quat(1) = sinHalfRoll * cosHalfPitch * cosHalfYaw - cosHalfRoll * sinHalfPitch * sinHalfYaw; // x
    quat(2) = cosHalfRoll * sinHalfPitch * cosHalfYaw + sinHalfRoll * cosHalfPitch * sinHalfYaw; // y
    quat(3) = cosHalfRoll * cosHalfPitch * sinHalfYaw - sinHalfRoll * sinHalfPitch * cosHalfYaw; // z

    Y_quat << y.head<3>(), quat;
    return Y_quat;
}

Matrix4d Matrices::OM_conj(Vector<double, 6> x)
{
    Matrix4d om_conj = Matrix4d::Zero();
    double P = x(3);
    double Q = x(4);
    double R = x(5);
    om_conj <<  0,  P,  Q,  R,
               -P,  0, -R,  Q,
               -Q,  R,  0, -P,
               -R, -Q,  P,  0;

    return -0.5 *om_conj;
}

Matrix<double, 6, 6> Matrices::massMatrix()
{
    UAVparams* params = UAVparams::getSingleton();
    //mass matrix
    Matrix<double, 6, 6> massMatrix;
    massMatrix.setZero();
    massMatrix(0,0) = params->m;
    massMatrix(1,1) = params->m;
    massMatrix(2,2) = params->m;
    massMatrix(3,3) = params->Ix;
    massMatrix(4,4) = params->Iy;
    massMatrix(5,5) = params->Iz;
    massMatrix(3,4) = -params->Ixy;
    massMatrix(4,3) = -params->Ixy;
    massMatrix(3,5) = -params->Ixz;
    massMatrix(5,3) = -params->Ixz;
    massMatrix(4,5) = -params->Iyz;
    massMatrix(5,4) = -params->Iyz;
    return massMatrix;
}

Matrix<double, 6, 6> Matrices::gyroMatrix(Vector<double, 6> x)
{
    Matrix<double,6,6> gyro;
    gyro.setZero();
    gyro(1,0) =  x(5);
    gyro(2,0) = -x(4);
    gyro(4,0) =  x(2);
    gyro(5,0) = -x(1);
    gyro(0,1) = -x(5);
    gyro(2,1) =  x(3);
    gyro(3,1) = -x(2);
    gyro(5,1) =  x(0);
    gyro(0,2) =  x(4);
    gyro(1,2) = -x(3);
    gyro(3,2) =  x(1);
    gyro(4,2) = -x(0);
    gyro(4,3) =  x(5);
    gyro(5,3) = -x(4);
    gyro(3,4) = -x(5);
    gyro(5,4) =  x(3);
    gyro(3,5) =  x(4);
    gyro(4,5) = -x(3);
    return gyro;
}

Matrix<double,6,6> Matrices::TMatrix(Vector<double,6>  y)
{
    Matrix3d Tv, Tom;
    Matrix<double,6,6> res;
    res.setZero();
    double fi = y(3);
    double theta = y(4);
    double psi = y(5);

    Tv  << cos(theta)*cos(psi), sin(fi)*sin(theta)*cos(psi) - cos(fi)*sin(psi), cos(fi)*sin(theta)*cos(psi) + sin(fi)*sin(psi),
           cos(theta)*sin(psi), sin(fi)*sin(theta)*sin(psi) + cos(fi)*cos(psi), cos(fi)*sin(theta)*sin(psi) - sin(fi)*cos(psi),
           -sin(theta)        , sin(fi)*cos(theta)                            , cos(fi)*cos(theta);
    
    Tom << 1, sin(fi)*tan(theta), cos(fi)*tan(theta),
           0, cos(fi)           , -sin(fi),
           0, sin(fi)/cos(theta), cos(fi)/cos(theta);
    res.block<3,3>(0,0) = Tv;
    res.block<3,3>(3,3) = Tom;
    return res;
}

Matrix<double, 3, 3> Matrices::R_nb(const Vector<double,6>&  y)
{
    double fi = y(3);
    double theta = y(4);
    double psi = y(5);
    Matrix<double, 3, 3> r_nb;
    r_nb << cos(theta)*cos(psi),                            cos(theta)*sin(psi),                           -sin(theta),
            sin(fi)*sin(theta)*cos(psi) - cos(fi)*sin(psi), sin(fi)*sin(theta)*sin(psi) + cos(fi)*cos(psi), sin(fi)*cos(theta),
            cos(fi)*sin(theta)*cos(psi) + sin(fi)*sin(psi), cos(fi)*sin(theta)*sin(psi) - sin(fi)*cos(psi), cos(fi)*cos(theta);
    return r_nb;
}

Matrix<double, 3, 3> Matrices::R_nb(const Vector<double,7>&  y)
{
    const Vector4d& e = y.tail<4>();
    Matrix<double, 3, 3> r_nb;
    r_nb << e(0)*e(0)+e(1)*e(1)-e(2)*e(2)-e(3)*e(3)  , 2*(e(1)*e(2)+e(0)*e(3))                   , 2*(e(1)*e(3)-e(0)*e(2)),
            2*(e(1)*e(2)-e(0)*e(3))                  , e(0)*e(0)-e(1)*e(1)+e(2)*e(2)-e(3)*e(3)   , 2*(e(2)*e(3)+e(0)*e(1)),
            2*(e(1)*e(3)+e(0)*e(2))                  , 2*(e(2)*e(3)-e(0)*e(1))                   , e(0)*e(0)-e(1)*e(1)-e(2)*e(2)+e(3)*e(3);
    return r_nb;
}
