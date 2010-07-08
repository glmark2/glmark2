#include "matrix.h"

/** 
 * Multiply arrays representing 4x4 matrices in column-major order.
 * 
 * The result is stored in the first matrix.
 *
 * @param m the first matrix
 * @param n the second matrix
 */
static void multiply(GLfloat *m, const GLfloat *n)
{
   GLfloat tmp[16];
   const GLfloat *row, *column;
   div_t d;
   int i, j;

   for (i = 0; i < 16; i++) {
      tmp[i] = 0;
      d = div(i, 4);
      row = n + d.quot * 4;
      column = m + d.rem;
      for (j = 0; j < 4; j++)
         tmp[i] += row[j] * column[j * 4];
   }
   memcpy(m, &tmp, sizeof tmp);
}

/** 
 * Multiply this matrix with another.
 *
 * @param pM the matrix to multiply with.
 * 
 * @return reference to this matrix (multiplied)
 */
Matrix4f &Matrix4f::operator*=(const Matrix4f &pM)
{
    multiply(m, pM.m);

    return *this;
}

/** 
 * Rotates a matrix.
 * 
 * @param angle the angle to rotate
 * @param x the x component of the rotation axis
 * @param y the y component of the rotation axis
 * @param z the z component of the rotation axis
 * 
 * @return reference to the matrix
 */
Matrix4f &Matrix4f::rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   double s, c;

   sincos(angle, &s, &c);

   GLfloat r[16] = {
      x * x * (1 - c) + c,     y * x * (1 - c) + z * s, x * z * (1 - c) - y * s, 0,
      x * y * (1 - c) - z * s, y * y * (1 - c) + c,     y * z * (1 - c) + x * s, 0, 
      x * z * (1 - c) + y * s, y * z * (1 - c) - x * s, z * z * (1 - c) + c,     0,
      0, 0, 0, 1
   };

   multiply(m, r);

   return *this;
}

/** 
 * Translates a matrix.
 * 
 * @param x the x component of the translation
 * @param y the y component of the translation
 * @param z the z component of the translation
 * 
 * @return reference to the matrix
 */
Matrix4f &Matrix4f::translate(GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat t[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  x, y, z, 1 };

   multiply(m, t);

   return *this;
}

/** 
 * Transposes a matrix.
 * 
 * @return reference to the matrix
 */
Matrix4f &Matrix4f::transpose()
{
   GLfloat t[16] = {
      m[0], m[4], m[8],  m[12],
      m[1], m[5], m[9],  m[13],
      m[2], m[6], m[10], m[14],
      m[3], m[7], m[11], m[15]};

   memcpy(m, t, sizeof(m));

   return *this;
}

/**
 * Makes this matrix an identity matrix.
 *
 * @return reference to the matrix
 */
Matrix4f &Matrix4f::identity()
{
   m[0] = 1.0; m[4] = 0.0; m[8] = 0.0;  m[12] = 0.0;
   m[1] = 0.0; m[5] = 1.0; m[9] = 0.0;  m[13] = 0.0;
   m[2] = 0.0; m[6] = 0.0; m[10] = 1.0; m[14] = 0.0;
   m[3] = 0.0; m[7] = 0.0; m[11] = 0.0; m[15] = 1.0;

   return *this;
}

/**
 * Makes this matrix a perspective projection matrix.
 *
 * @param fovy field of view angle in degrees in the y direction
 * @param aspect aspect ratio of view
 * @param zNear the distance from the viewer to the near clipping plane
 * @param zFar the distance from the viewer to the far clipping plane
 *
 * @return reference to the matrix
 */
Matrix4f &Matrix4f::perspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
    GLfloat sine, cotangent, deltaZ;
    GLfloat radians = fovy / 2 * M_PI / 180;

    deltaZ = zFar - zNear;
    sine = sin(radians);

    if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
       return *this;
    }

    cotangent = cos(radians) / sine;

    identity();
    m[0] = cotangent / aspect;
    m[5] = cotangent;
    m[10] = -(zFar + zNear) / deltaZ;
    m[11] = -1;
    m[14] = -2 * zNear * zFar / deltaZ;
    m[15] = 0;

    return *this;
}

/**
 * Inverts this matrix.
 *
 * This method can currently handle only pure translation-rotation matrices.
 *
 * @return reference to the matrix
 */
Matrix4f &Matrix4f::invert()
{
   // If the bottom row is [0, 0, 0, 1] this is a pure translation-rotation
   // transformation matrix and we can optimize the matrix inversion.
   // Read http://www.gamedev.net/community/forums/topic.asp?topic_id=425118
   // for an explanation.
   if (m[3] == 0.0 &&  m[7] == 0.0 && m[11] == 0.0 && m[15] == 1.0) {
      // Extract and invert the translation part 't'. The inverse of a
      // translation matrix can be calculated by negating the translation
      // coordinates.
      Matrix4f t(1.0, 1.0, 1.0);
      t.m[12] = -m[12]; t.m[13] = -m[13]; t.m[14] = -m[14];

      // Invert the rotation part 'r'. The inverse of a rotation matrix is
      // equal to its transpose.
      m[12] = m[13] = m[14] = 0;
      this->transpose();

      // inv(m) = inv(r) * inv(t)
      *this *= t;
   }
   else {
      // Don't care about the general case for now
   }

   return *this;
}

/** 
 * Creates an empty matrix.
 *
 * All matrix components are 0.0 expect the lower right
 * which is 1.0.
 */
Matrix4f::Matrix4f()
{
    memset(m, 0, sizeof(m));
    m[15] = 1.0;
}

/** 
 * Copy constructor.
 * 
 * @param mat the matrix to copy the contents of.
 */
Matrix4f::Matrix4f(Matrix4f &mat)
{
   memcpy(m, mat.m, sizeof(m));
}

/** 
 * Creates a matrix with specified values in the diagonal.
 * 
 * The lower right value is initialized to 1.0.
 *
 * @param x the x component of the diagonal
 * @param y the y component of the diagonal
 * @param z the z component of the diagonal
 */
Matrix4f::Matrix4f(GLfloat x, GLfloat y, GLfloat z)
{
    memset(m, 0, sizeof(m));
    m[0] = x;
    m[5] = y;
    m[10] = z;
    m[15] = 1.0;
}

/** 
 * Displays a matrix.
 *
 * @param str string to display before matrix
 */
void Matrix4f::display(const char *str)
{
   int r;
   if (str != NULL)
      printf("%s\n", str);
   for(r = 0; r < 4; r++)
      printf("%f %f %f %f\n", m[r], m[r + 4], m[r + 8], m[r + 12]);
}
