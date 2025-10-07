struct RGB { float r, g, b; };

inline RGB hsv_to_rgb(float h, float s, float v) {
    h = h - floor(h);               
    float hf = h * 6.0f;
    float i  = floor(hf);
    float f  = hf - i;

    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    // Masks instead of branches
    float m0 = (i == 0.0f);
    float m1 = (i == 1.0f);
    float m2 = (i == 2.0f);
    float m3 = (i == 3.0f);
    float m4 = (i == 4.0f);
    float m5 = 1.0f - (m0+m1+m2+m3+m4);

    RGB out;
    out.r = m0*v + m1*q + m2*p + m3*p + m4*t + m5*v;
    out.g = m0*t + m1*v + m2*v + m3*q + m4*p + m5*p;
    out.b = m0*p + m1*p + m2*t + m3*v + m4*v + m5*q;
    return out;
}


inline float clampf(float x, float low, float high) {
    return max(low, min(high, x));
}

// (a+ib) * (c+id)
inline void complex_mul(float a, float b, float c, float d,
                        float &realPart, float &imagPart) {
    realPart = a*c - b*d;
    imagPart = a*d + b*c;
}


// (a+ib) / (c+id)
inline void complex_div(float a, float b, float c, float d,
                        float &realPart, float &imagPart) {
    float denom = c*c + d*d + 1e-30f;
    realPart = (a*c + b*d) / denom;
    imagPart = (b*c - a*d) / denom;
}


// (z^k), integer power by repeated squaring
inline void complex_pow(float zr, float zi, int k,
                            float &realPart, float &imagPart) {
    realPart = 1.0f; imagPart = 0.0f;
    float base_r = zr, base_i = zi;
    int exp = k;
    while (exp > 0) {
        if (exp & 1) {
            float tr, ti;
            complex_mul(realPart, imagPart, base_r, base_i, tr, ti);
            realPart = tr; imagPart = ti;
        }
        float tr, ti;
        complex_mul(base_r, base_i, base_r, base_i, tr, ti);
        base_r = tr; base_i = ti;
        exp >>= 1;
    }
}
export void render(
    uniform int width,
    uniform int height,
    uniform int rootCount,
    uniform int maxIterations,
    uniform uint32 * uniform outRGBA
) {
    uniform float epsilon = 1e-12f;

   // View window in complex plane
    uniform float xmin = -1.6f, xmax = 1.6f;
    uniform float ymin = -1.6f, ymax = 1.6f;

    uniform int   totalPixels = width * height;
    uniform float invWidth = 1.0f / (float)width;
    uniform float invHeight = 1.0f / (float)height;
    uniform float xrange = xmax - xmin;
    uniform float yrange = ymax - ymin;

    foreach (index = 0 ... totalPixels) {
        // Pixel coords 
        // index = (y * width) + x
        // x = index - (y * width)
        float fIndex = (float)index;
        int pixelCoordY = (int)floor(fIndex * invWidth);
        int pixelCoordX = index - (pixelCoordY * width);

        // Normalized UV
        float u = ((float)pixelCoordX + 0.5f) * invWidth;
        float v = ((float)pixelCoordY + 0.5f) * invHeight;

        // Complex plane coordinates 
        float zr = xmin + u * xrange;
        float zi = ymin + v * yrange;

        // Newton iteration
        int iter = 0;
        for (iter = 0; iter < maxIterations; ++iter) {
            float realPart, imagPart; 

            complex_pow(zr, zi, rootCount, realPart, imagPart);
            // z^k - 1
            realPart = realPart - 1.0f;
            float squaredMagnitude = realPart*realPart + imagPart*imagPart;
            // Checking for convergence
            if (squaredMagnitude < epsilon) break;

            // Derivative
            float drealPart, dimagPart;
            complex_pow(zr, zi, rootCount - 1, drealPart, dimagPart);
            drealPart = (float)rootCount * drealPart;
            dimagPart = (float)rootCount * dimagPart;

            float stepReal, stepImag;
            complex_div(realPart, imagPart, drealPart, dimagPart, stepReal, stepImag);

            // Step
            zr -= stepReal;
            zi -= stepImag;

            // If goes infinity
            if ((zr*zr + zi*zi) > 1e16f) break;
        }

        // Determine root index from angle
        float angle = atan2(zi, zr);
        float rootf = (angle / (2.0f * 3.14159265358979323846f)) * (float)rootCount;
        int rootIndex = (int)floor(rootf + 0.5f);

        // Negative angles
        if (rootIndex < 0) {
            rootIndex += rootCount;
        } else if (rootIndex >= rootCount){
            rootIndex -= rootCount;
        }

        // Color: hue by root, value by speed of convergence
        float hue = (rootCount > 0) ? ((float)rootIndex / (float)rootCount) : 0.0f;
        float sat = 1.0f;
        float val = (iter < maxIterations) ? (1.0f - (float)iter / (float)maxIterations) : 0.0f;

        RGB rgb = hsv_to_rgb(hue, sat, val);

        int32 R = (int32)clampf(rgb.r * 255.0f + 0.5f, 0.0f, 255.0f);
        int32 G = (int32)clampf(rgb.g * 255.0f + 0.5f, 0.0f, 255.0f);
        int32 B = (int32)clampf(rgb.b * 255.0f + 0.5f, 0.0f, 255.0f);
        int32 A = 255;

        // Pack into uint32 
        uint32 pixel = (uint32)R | ((uint32)G << 8) | ((uint32)B << 16) | ((uint32)A << 24);

        outRGBA[index] = pixel;
    }
}