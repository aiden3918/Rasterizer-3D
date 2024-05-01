#pragma once

#ifndef definitions_hpp
#define defintions_hpp

#include <fstream>
#include <iostream>
#include <sstream>

typedef std::vector<std::vector<double>> vecXYZ;

struct vec3D {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

struct triangle {
    vec3D vertices[3];
};

struct mesh {
    std::vector<triangle> triangles;

    bool LoadFromObjectFile(std::string sFilename) {
        std::ifstream file(sFilename);
        if (!file.is_open()) {
            std::cout << "Unable to load file: " << sFilename << std::endl;
            return false;
        }
        std::cout << sFilename << " loaded" << std::endl;

        // store the unassigned vertices
        std::vector<vec3D> vertices;

        // iterate as long as not at end of file
        while (!file.eof()) {
            char fileline[128];
            file.getline(fileline, 128);

            std::stringstream stream;
            // read current file line
            stream << fileline;

            char junk;

            // get all of the vertex points in the obj text file
            if (fileline[0] == 'v') {
                vec3D vertex;
                // write each "element"(?) of the stringstream into each location
                stream >> junk >> vertex.x >> vertex.y >> vertex.z;
                vertices.push_back(vertex);
            }

            // assign those vertex points to triangles in the obj text file
            if (fileline[0] == 'f') {
                int f[3];
                stream >> junk >> f[0] >> f[1] >> f[2];
                // indices in object file start at 1
                triangles.push_back({ vertices[f[0] - 1], vertices[f[1] - 1], vertices[f[2] - 1] });
            }

        }

        return true;
    }
};

struct matrix4x4 {
    float matrix[4][4] = { 0 };
};

struct line {
    vec3D point[2];
};

struct graph {
    std::vector<line> lines;
};

struct function {
    std::vector<vec3D> points;
};

// matrix multiplication
vec3D Matrix_MultiplyVector(matrix4x4& m, vec3D& i) {
    vec3D o;
    o.x = (i.x * m.matrix[0][0]) + (i.y * m.matrix[1][0]) + (i.z * m.matrix[2][0]) + (i.w * m.matrix[3][0]);
    o.y = (i.x * m.matrix[0][1]) + (i.y * m.matrix[1][1]) + (i.z * m.matrix[2][1]) + (i.w * m.matrix[3][1]);
    o.z = (i.x * m.matrix[0][2]) + (i.y * m.matrix[1][2]) + (i.z * m.matrix[2][2]) + (i.w * m.matrix[3][2]);
    o.w = (i.x * m.matrix[0][3]) + (i.y * m.matrix[1][3]) + (i.z * m.matrix[2][3]) + (i.w * m.matrix[3][3]);

    return o;
}

matrix4x4 Matrix_MakeIdentity()
{
    matrix4x4 matrix;
    matrix.matrix[0][0] = 1.0f;
    matrix.matrix[1][1] = 1.0f;
    matrix.matrix[2][2] = 1.0f;
    matrix.matrix[3][3] = 1.0f;
    return matrix;
}

matrix4x4 Matrix_MakeRotationX(float fAngleRad)
{
    matrix4x4 matrix;
    matrix.matrix[0][0] = 1.0f;
    matrix.matrix[1][1] = cosf(fAngleRad);
    matrix.matrix[1][2] = sinf(fAngleRad);
    matrix.matrix[2][1] = -sinf(fAngleRad);
    matrix.matrix[2][2] = cosf(fAngleRad);
    matrix.matrix[3][3] = 1.0f;
    return matrix;
}

matrix4x4 Matrix_MakeRotationY(float fAngleRad)
{
    matrix4x4 matrix;
    matrix.matrix[0][0] = cosf(fAngleRad);
    matrix.matrix[0][2] = sinf(fAngleRad);
    matrix.matrix[2][0] = -sinf(fAngleRad);
    matrix.matrix[1][1] = 1.0f;
    matrix.matrix[2][2] = cosf(fAngleRad);
    matrix.matrix[3][3] = 1.0f;
    return matrix;
}

matrix4x4 Matrix_MakeRotationZ(float fAngleRad)
{
    matrix4x4 matrix;
    matrix.matrix[0][0] = cosf(fAngleRad);
    matrix.matrix[0][1] = sinf(fAngleRad);
    matrix.matrix[1][0] = -sinf(fAngleRad);
    matrix.matrix[1][1] = cosf(fAngleRad);
    matrix.matrix[2][2] = 1.0f;
    matrix.matrix[3][3] = 1.0f;
    return matrix;
}

matrix4x4 Matrix_MakeTranslation(float x, float y, float z)
{
    matrix4x4 matrix;
    matrix.matrix[0][0] = 1.0f;
    matrix.matrix[1][1] = 1.0f;
    matrix.matrix[2][2] = 1.0f;
    matrix.matrix[3][3] = 1.0f;
    matrix.matrix[3][0] = x;
    matrix.matrix[3][1] = y;
    matrix.matrix[3][2] = z;
    return matrix;
}

matrix4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
    float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
    matrix4x4 matrix;
    matrix.matrix[0][0] = fAspectRatio * fFovRad;
    matrix.matrix[1][1] = fFovRad;
    matrix.matrix[2][2] = fFar / (fFar - fNear);
    matrix.matrix[3][2] = (-fFar * fNear) / (fFar - fNear);
    matrix.matrix[2][3] = 1.0f;
    matrix.matrix[3][3] = 0.0f;
    return matrix;
}

matrix4x4 Matrix_MultiplyMatrix(matrix4x4& m1, matrix4x4& m2)
{
    matrix4x4 matrix;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            matrix.matrix[r][c] = m1.matrix[r][0] * m2.matrix[0][c] + m1.matrix[r][1] * m2.matrix[1][c] + m1.matrix[r][2] * m2.matrix[2][c] + m1.matrix[r][3] * m2.matrix[3][c];
    return matrix;
}

vec3D Vector_Add(vec3D& v1, vec3D& v2)
{
    return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

vec3D Vector_Sub(vec3D& v1, vec3D& v2)
{
    return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

vec3D Vector_Mul(vec3D& v1, float k)
{
    return { v1.x * k, v1.y * k, v1.z * k };
}

vec3D Vector_Div(vec3D& v1, float k)
{
    if (k == 0.0f) return { 0.0f, 0.0f, 0.0f };
    return { v1.x / k, v1.y / k, v1.z / k };
}

float Vector_DotProduct(vec3D& v1, vec3D& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float Vector_Length(vec3D& v)
{
    return sqrtf(Vector_DotProduct(v, v));
}

vec3D Vector_Normalise(vec3D& v)
{
    float l = Vector_Length(v);
    return { v.x / l, v.y / l, v.z / l };
}

vec3D Vector_CrossProduct(vec3D& v1, vec3D& v2)
{
    vec3D v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    return v;
}

// position of object (where it should be), "forward vector", up vector
matrix4x4 Matrix_PointAt(vec3D& pos, vec3D& target, vec3D& up) {
    // calculate new forward direction (C)
    vec3D newForward = Vector_Sub(target, pos);
    newForward = Vector_Normalise(newForward);

    // calculate new up direction (B)
    vec3D a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
    // DEBUG: messing around with this rn because y is upside down (x axis points to the negative direction, so its good)
    vec3D newUp = Vector_Sub(a, up);
    newUp = Vector_Normalise(newUp);

    // calculate new right direction (A)
    // uses cross product because x axis is technically normal to y and z axis
    vec3D newRight = Vector_CrossProduct(newForward, newUp);

    matrix4x4 pointAtMatrix;
    pointAtMatrix.matrix[0][0] = newRight.x;    pointAtMatrix.matrix[0][1] = newRight.y;    pointAtMatrix.matrix[0][2] = newRight.z;    pointAtMatrix.matrix[0][3] = 0.0f;
    pointAtMatrix.matrix[1][0] = newUp.x;       pointAtMatrix.matrix[1][1] = newUp.y;       pointAtMatrix.matrix[1][2] = newUp.z;       pointAtMatrix.matrix[1][3] = 0.0f;
    pointAtMatrix.matrix[2][0] = newForward.x;  pointAtMatrix.matrix[2][1] = newForward.y;  pointAtMatrix.matrix[2][2] = newForward.z;  pointAtMatrix.matrix[2][3] = 0.0f;
    pointAtMatrix.matrix[3][0] = pos.x;         pointAtMatrix.matrix[3][1] = pos.y;         pointAtMatrix.matrix[3][2] = pos.z;         pointAtMatrix.matrix[3][3] = 1.0f;

    return pointAtMatrix;
}

matrix4x4 Matrix_QuickInverse(matrix4x4& m) // Only for Rotation/Translation Matrices
{
    matrix4x4 matrix;
    matrix.matrix[0][0] = m.matrix[0][0]; matrix.matrix[0][1] = m.matrix[1][0]; matrix.matrix[0][2] = m.matrix[2][0]; matrix.matrix[0][3] = 0.0f;
    matrix.matrix[1][0] = m.matrix[0][1]; matrix.matrix[1][1] = m.matrix[1][1]; matrix.matrix[1][2] = m.matrix[2][1]; matrix.matrix[1][3] = 0.0f;
    matrix.matrix[2][0] = m.matrix[0][2]; matrix.matrix[2][1] = m.matrix[1][2]; matrix.matrix[2][2] = m.matrix[2][2]; matrix.matrix[2][3] = 0.0f;
    matrix.matrix[3][0] = -(m.matrix[3][0] * matrix.matrix[0][0] + m.matrix[3][1] * matrix.matrix[1][0] + m.matrix[3][2] * matrix.matrix[2][0]);
    matrix.matrix[3][1] = -(m.matrix[3][0] * matrix.matrix[0][1] + m.matrix[3][1] * matrix.matrix[1][1] + m.matrix[3][2] * matrix.matrix[2][1]);
    matrix.matrix[3][2] = -(m.matrix[3][0] * matrix.matrix[0][2] + m.matrix[3][1] * matrix.matrix[1][2] + m.matrix[3][2] * matrix.matrix[2][2]);
    matrix.matrix[3][3] = 1.0f;
    return matrix;
}

// plane_p = plane's point, plane_n = plane's normal
// return the point P where a line intersects with a plane
// NxPx + NyPy + NzPz - P * N = 0
vec3D Vector_IntersectPlane(vec3D& plane_p, vec3D& plane_n, vec3D& lineStart, vec3D& lineEnd)
{
    plane_n = Vector_Normalise(plane_n);
    float plane_d = -Vector_DotProduct(plane_n, plane_p);
    float ad = Vector_DotProduct(lineStart, plane_n);
    float bd = Vector_DotProduct(lineEnd, plane_n);
    float t = (-plane_d - ad) / (bd - ad);
    vec3D lineStartToEnd = Vector_Sub(lineEnd, lineStart);
    vec3D lineToIntersect = Vector_Mul(lineStartToEnd, t);
    return Vector_Add(lineStart, lineToIntersect);
}

// returns how many triangles will be produced
// note: signed distance (displacement) is used to determine if triangle needs clipping first (and how much),
//       then Vector_IntersectPlane is used to find where that triangle should be clipped in world space
int Triangle_ClipAgainstPlane(vec3D plane_p, vec3D plane_n, triangle& inputTri, triangle& outputTri1, triangle& outputTri2)
{
    // make sure plane normal is normal
    plane_n = Vector_Normalise(plane_n);

    auto distance = [&](vec3D& point)
        {
            vec3D n = Vector_Normalise(point);
            return (plane_n.x * point.x + plane_n.y * point.y + plane_n.z * point.z - Vector_DotProduct(plane_n, plane_p));
        };

    // Create two temporary storage arrays to classify points either side of plane
    // If distance sign is positive, point lies on "inside" of plane
    vec3D* inside_points[3];  int nInsidePointCount = 0;
    vec3D* outside_points[3]; int nOutsidePointCount = 0;

    // Get signed distance of each point in triangle to plane
    float d0 = distance(inputTri.vertices[0]);
    float d1 = distance(inputTri.vertices[1]);
    float d2 = distance(inputTri.vertices[2]);

    // if distance is positive, then point is inside plane
    if (d0 >= 0) { inside_points[nInsidePointCount++] = &inputTri.vertices[0]; }
    else { outside_points[nOutsidePointCount++] = &inputTri.vertices[0]; }
    if (d1 >= 0) { inside_points[nInsidePointCount++] = &inputTri.vertices[1]; }
    else { outside_points[nOutsidePointCount++] = &inputTri.vertices[1]; }
    if (d2 >= 0) { inside_points[nInsidePointCount++] = &inputTri.vertices[2]; }
    else { outside_points[nOutsidePointCount++] = &inputTri.vertices[2]; }

    if (nInsidePointCount == 0) {
        return 0;
    }

    if (nInsidePointCount == 3) {
        outputTri1 = inputTri;
        return 1;
    }

    if (nInsidePointCount == 1 && nOutsidePointCount == 2) {
        // Triangle should be clipped. As two points lie outside
        // the plane, the triangle simply becomes a smaller triangle

        // The inside point is valid, so keep that...
        outputTri1.vertices[0] = *inside_points[0];

        // but the two new points are at the locations where the 
        // original sides of the triangle (lines) intersect with the plane
        outputTri1.vertices[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
        outputTri1.vertices[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

        return 1; // Return the newly formed single triangle
    }

    if (nInsidePointCount == 2 && nOutsidePointCount == 1) {
        // The first triangle consists of the two inside points and a new
        // point determined by the location where one side of the triangle
        // intersects with the plane
        outputTri1.vertices[0] = *inside_points[0];
        outputTri1.vertices[1] = *inside_points[1];
        outputTri1.vertices[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

        // The second triangle is composed of one of he inside points, a
        // new point determined by the intersection of the other side of the 
        // triangle and the plane, and the newly created point above
        outputTri2.vertices[0] = *inside_points[1];
        outputTri2.vertices[1] = outputTri1.vertices[2];
        outputTri2.vertices[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

        return 2;
    }

}

// courtesy of Bill the Lizard - Stack Overflow
// https://stackoverflow.com/users/1288/bill-the-lizard
bool isFloat(std::string myString) {
    std::istringstream iss(myString);
    float f;
    iss >> std::noskipws >> f; // noskipws considers leading whitespace invalid
    // Check the entire string was consumed and if either failbit or badbit is set
    return iss.eof() && !iss.fail();
}

#endif // !definitions_hpp
