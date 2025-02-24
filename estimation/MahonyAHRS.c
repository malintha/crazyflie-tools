//=====================================================================================================
// MahonyAHRS.c
//=====================================================================================================
//
// Madgwick's implementation of Mayhony's AHRS algorithm.
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
//
// Date			Author			Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
// 02/22/2015	Benoit Landry	Simplified for crazyflie experiment, plus python api
//
//=====================================================================================================

#include "MahonyAHRS.h"
#include <math.h>
#include <Python.h>

#define twoKp	(2.0f * 0.5f)	// 2 * proportional gain
#define twoKi	(2.0f * 0.0f)	// 2 * integral gain

float invSqrt(float x);

static PyObject *MahonyAHRS_MahonyAHRSupdateIMU(PyObject *self, PyObject *args) {
	float gx, gy, gz;
	float ax, ay, az;
	float dt;
	float q0, q1, q2, q3;
	float integralFBx, integralFBy, integralFBz;

	if (!PyArg_ParseTuple(args, "ffffffffffffff", &gx, &gy, &gz, &ax, &ay, &az, &dt,
												  &q0, &q1, &q2, &q3, 
												  &integralFBx, &integralFBy, &integralFBz)) {
		return NULL;
   	}

   	float ret[7];
   	MahonyAHRSupdateIMU(gx,gy,gz,ax,ay,az,dt,q0,q1,q2,q3,integralFBx,integralFBy,integralFBz,ret);
   	return Py_BuildValue("fffffff", ret[0], ret[1], ret[2], ret[3], ret[4], ret[5], ret[6]);
}

static PyMethodDef MahonyAHRS_methods[] = {
   { "MahonyAHRSupdateIMU", (PyCFunction)MahonyAHRS_MahonyAHRSupdateIMU, METH_VARARGS, NULL },
   { NULL, NULL, 0, NULL }
};

PyMODINIT_FUNC initMahonyAHRS() {
   Py_InitModule("MahonyAHRS", MahonyAHRS_methods);
}

void MahonyAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az, float dt,
	float q0, float q1, float q2, float q3, float integralFBx, float integralFBy, float integralFBz, 
	float* ret) {
	
	float recipNorm;
	float halfvx, halfvy, halfvz;
	float halfex, halfey, halfez;
	float qa, qb, qc;

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;        

		// Estimated direction of gravity and vector perpendicular to magnetic flux
		halfvx = q1 * q3 - q0 * q2;
		halfvy = q0 * q1 + q2 * q3;
		halfvz = q0 * q0 - 0.5f + q3 * q3;
	
		// Error is sum of cross product between estimated and measured direction of gravity
		halfex = (ay * halfvz - az * halfvy);
		halfey = (az * halfvx - ax * halfvz);
		halfez = (ax * halfvy - ay * halfvx);

		// Compute and apply integral feedback if enabled
		if(twoKi > 0.0f) {
			integralFBx += twoKi * halfex * dt;	// integral error scaled by Ki
			integralFBy += twoKi * halfey * dt;
			integralFBz += twoKi * halfez * dt;
			gx += integralFBx;	// apply integral feedback
			gy += integralFBy;
			gz += integralFBz;
		}
		else {
			integralFBx = 0.0f;	// prevent integral windup
			integralFBy = 0.0f;
			integralFBz = 0.0f;
		}

		// Apply proportional feedback
		gx += twoKp * halfex;
		gy += twoKp * halfey;
		gz += twoKp * halfez;
	}
	
	// Integrate rate of change of quaternion
	gx *= (0.5f * dt);		// pre-multiply common factors
	gy *= (0.5f * dt);
	gz *= (0.5f * dt);
	qa = q0;
	qb = q1;
	qc = q2;
	q0 += (-qb * gx - qc * gy - q3 * gz);
	q1 += (qa * gx + qc * gz - q3 * gy);
	q2 += (qa * gy - qb * gz + q3 * gx);
	q3 += (qa * gz + qb * gy - qc * gx); 
	
	// Normalise quaternion
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;

	ret[0] = q0;
	ret[1] = q1;
	ret[2] = q2;
	ret[3] = q3;
	ret[4] = integralFBx;
	ret[5] = integralFBy;
	ret[6] = integralFBz;
}

// Fast inverse square-root
// See: http://en.wikipedia.org/wiki/Fast_inverse_square_root
float invSqrt(float x) {
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}