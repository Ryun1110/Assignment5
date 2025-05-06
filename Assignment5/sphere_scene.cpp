#include <vector>
#include <array>
#include <cmath>
#include <cfloat>
#include <fstream>
#include <iostream>
#include <algorithm>         

#ifdef _WIN32
#define NOMINMAX           
#include <windows.h>
#include <Shellapi.h>
#endif

struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };
struct Mat4 {
    float m[4][4]{};
    static Mat4 identity() {
        Mat4 I; for (int i = 0; i < 4; ++i) I.m[i][i] = 1; return I;
    }
    Vec4 operator*(const Vec4& v) const {
        Vec4 o;
        o.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w;
        o.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w;
        o.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w;
        o.w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w;
        return o;
    }
    Mat4 operator*(const Mat4& b) const {
        Mat4 r{};
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                for (int k = 0; k < 4; ++k)
                    r.m[i][j] += m[i][k] * b.m[k][j];
        return r;
    }
};

Mat4 translate(float tx, float ty, float tz) {
    Mat4 T = Mat4::identity();
    T.m[0][3] = tx; T.m[1][3] = ty; T.m[2][3] = tz; return T;
}
Mat4 scale(float sx, float sy, float sz) {
    Mat4 S = Mat4::identity();
    S.m[0][0] = sx; S.m[1][1] = sy; S.m[2][2] = sz; return S;
}
Mat4 perspective(float l, float r, float b, float t, float n, float f) {
    Mat4 P{};
    P.m[0][0] = 2 * n / (r - l);
    P.m[1][1] = 2 * n / (t - b);
    P.m[0][2] = (r + l) / (r - l);
    P.m[1][2] = (t + b) / (t - b);
    P.m[2][2] = -(f + n) / (f - n);
    P.m[2][3] = -2 * f * n / (f - n);
    P.m[3][2] = -1;
    return P;
}
Mat4 viewport(int nx, int ny) {
    Mat4 V = Mat4::identity();
    V.m[0][0] = nx / 2.0f;  V.m[0][3] = (nx - 1) / 2.0f;
    V.m[1][1] = -ny / 2.0f;  V.m[1][3] = (ny - 1) / 2.0f;   // y 뒤집기
    V.m[2][2] = 0.5f;      V.m[2][3] = 0.5f;
    return V;
}

int               gNumVertices = 0, gNumTriangles = 0;
std::vector<Vec3> gVertexBuffer;
int* gIndexBuffer = nullptr;

void create_scene() {
    const int W = 32, H = 16;
    const float PI = 3.14159265358979323846f;
    gNumVertices = (H - 2) * W + 2;
    gNumTriangles = (H - 2) * (W - 1) * 2;
    gVertexBuffer.resize(gNumVertices);
    gIndexBuffer = new int[3 * gNumTriangles];

    int id = 0;
    for (int j = 1; j < H - 1; ++j)
        for (int i = 0; i < W; ++i, ++id) {
            float theta = float(j) / (H - 1) * PI;
            float phi = float(i) / (W - 1) * 2 * PI;
            gVertexBuffer[id] = {
                std::sin(theta) * std::cos(phi),
                std::cos(theta),
                -std::sin(theta) * std::sin(phi) };
        }
    gVertexBuffer[id++] = { 0, 1,0 };
    gVertexBuffer[id++] = { 0,-1,0 };

    id = 0;
    for (int j = 0; j < H - 3; ++j)
        for (int i = 0; i < W - 1; ++i) {
            gIndexBuffer[id++] = j * W + i;
            gIndexBuffer[id++] = (j + 1) * W + i + 1;
            gIndexBuffer[id++] = j * W + i + 1;
            gIndexBuffer[id++] = j * W + i;
            gIndexBuffer[id++] = (j + 1) * W + i;
            gIndexBuffer[id++] = (j + 1) * W + i + 1;
        }
    for (int i = 0; i < W - 1; ++i) {
        gIndexBuffer[id++] = (H - 2) * W;      gIndexBuffer[id++] = i;                gIndexBuffer[id++] = i + 1;
        gIndexBuffer[id++] = (H - 2) * W + 1;    gIndexBuffer[id++] = (H - 3) * W + i + 1;      gIndexBuffer[id++] = (H - 3) * W + i;
    }
}

float edge(const Vec3& a, const Vec3& b, const Vec3& c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}
bool inTri(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c) {
    float w0 = edge(b, c, p), w1 = edge(c, a, p), w2 = edge(a, b, p);
    return (w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0);
}

void saveBMP(const char* fname, int W, int H,
    const std::vector<std::array<unsigned char, 3>>& pix) {
#ifdef _WIN32
    int rowPitch = ((W * 3 + 3) & ~3);
    int dataSize = rowPitch * H;

    BITMAPFILEHEADER fh{};
    BITMAPINFOHEADER ih{};
    fh.bfType = 0x4D42;                         // 'BM'
    fh.bfOffBits = sizeof(fh) + sizeof(ih);
    fh.bfSize = fh.bfOffBits + dataSize;

    ih.biSize = sizeof(ih);
    ih.biWidth = W; ih.biHeight = H;
    ih.biPlanes = 1; ih.biBitCount = 24;
    ih.biCompression = BI_RGB;

    std::ofstream ofs(fname, std::ios::binary);
    ofs.write((char*)&fh, sizeof(fh));
    ofs.write((char*)&ih, sizeof(ih));

    std::vector<unsigned char> row(rowPitch);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            const auto& c = pix[(H - 1 - y) * W + x];    
            row[x * 3 + 0] = c[2]; row[x * 3 + 1] = c[1]; row[x * 3 + 2] = c[0];
        }
        ofs.write((char*)row.data(), rowPitch);
    }
#else
    (void)fname; (void)W; (void)H; (void)pix; 
#endif
}

int main() {
    constexpr int NX = 512, NY = 512;
    create_scene();

    Mat4 M = translate(0, 0, -7) * scale(2, 2, 2);          // T * S
    Mat4 V = Mat4::identity();
    Mat4 P = perspective(-0.1f, 0.1f, -0.1f, 0.1f,-0.1f, -1000.f);
    Mat4 C = viewport(NX, NY) * P * V * M;

    std::vector<std::array<unsigned char, 3>> color(NX * NY, { 0,0,0 });
    std::vector<float> depth(NX * NY, FLT_MAX);

    for (int t = 0; t < gNumTriangles; ++t) {
        int a = gIndexBuffer[3 * t], b = gIndexBuffer[3 * t + 1], c = gIndexBuffer[3 * t + 2];
        Vec4 P4[3] = {
            C * Vec4{gVertexBuffer[a].x,gVertexBuffer[a].y,gVertexBuffer[a].z,1},
            C * Vec4{gVertexBuffer[b].x,gVertexBuffer[b].y,gVertexBuffer[b].z,1},
            C * Vec4{gVertexBuffer[c].x,gVertexBuffer[c].y,gVertexBuffer[c].z,1} };
        Vec3 P2[3];
        for (int i = 0; i < 3; ++i)
            P2[i] = { P4[i].x / P4[i].w, P4[i].y / P4[i].w, P4[i].z / P4[i].w };

        int xmin = std::max(0, std::min({ int(P2[0].x),int(P2[1].x),int(P2[2].x) }));
        int xmax = std::min(NX - 1, std::max({ int(P2[0].x),int(P2[1].x),int(P2[2].x) }));
        int ymin = std::max(0, std::min({ int(P2[0].y),int(P2[1].y),int(P2[2].y) }));
        int ymax = std::min(NY - 1, std::max({ int(P2[0].y),int(P2[1].y),int(P2[2].y) }));

        for (int y = ymin; y <= ymax; ++y)
            for (int x = xmin; x <= xmax; ++x) {
                Vec3 p{ float(x) + 0.5f,float(y) + 0.5f,0 };
                if (!inTri(p, P2[0], P2[1], P2[2])) continue;
                float z = (P2[0].z + P2[1].z + P2[2].z) / 3;
                int idx = y * NX + x;
                if (z < depth[idx]) {
                    depth[idx] = z;
                    color[idx] = { 255,255,255 };
                }
            }
    }

    std::ofstream ppm("output.ppm", std::ios::binary);
    ppm << "P6\n" << NX << " " << NY << "\n255\n";
    for (auto& c : color) ppm.write((char*)c.data(), 3);

    saveBMP("output.bmp", NX, NY, color);
#ifdef _WIN32
    ShellExecuteA(nullptr, "open", "output.bmp", nullptr, nullptr, SW_SHOWNORMAL);
#endif
    return 0;
}
