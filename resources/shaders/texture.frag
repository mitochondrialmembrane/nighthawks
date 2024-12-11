#version 330 core
in vec2 out_uv;
uniform sampler2D tex;

uniform bool isInverting;
uniform bool isSharpening;

uniform float pixelW;
uniform float pixelH;

out vec4 fragColor;

vec3 sampleColor(vec2 offset) {
    vec2 coord = out_uv + offset * vec2(pixelW, pixelH);
    return texture(tex, coord).rgb;
}

void getSectorVarianceAndAverageColor(vec2 offset, int boxSize, out vec3 avgColor, out float variance) {
    vec3 colorSum = vec3(0.0);
    vec3 squaredColorSum = vec3(0.0);
    float sampleCount = 0.0;
=
    for (int y = 0; y < boxSize; y++) {
        for (int x = 0; x < boxSize; x++) {
            vec2 sampleOffset = offset + vec2(float(x), float(y));
            vec3 color = sampleColor(sampleOffset);
            colorSum += color;
            squaredColorSum += color * color;
            sampleCount += 1.0;
        }
    }

    // Calculate average color and variance
    avgColor = colorSum / sampleCount;
    vec3 varianceRes = (squaredColorSum / sampleCount) - (avgColor * avgColor);
    variance = dot(varianceRes, vec3(0.299, 0.587, 0.114));
}

void main() {
    vec3 boxAvgColors[4];
    float boxVariances[4];
    int kernelSize = 10;

    getSectorVarianceAndAverageColor(vec2(-kernelSize, -kernelSize), kernelSize, boxAvgColors[0], boxVariances[0]);
    getSectorVarianceAndAverageColor(vec2(0, -kernelSize), kernelSize, boxAvgColors[1], boxVariances[1]);
    getSectorVarianceAndAverageColor(vec2(-kernelSize, 0), kernelSize, boxAvgColors[2], boxVariances[2]);
    getSectorVarianceAndAverageColor(vec2(0, 0), kernelSize, boxAvgColors[3], boxVariances[3]);

    float minVariance = boxVariances[0];
    vec3 finalColor = boxAvgColors[0];

    for (int i = 1; i < 4; i++) {
        if (boxVariances[i] < minVariance) {
            minVariance = boxVariances[i];
            finalColor = boxAvgColors[i];
        }
    }

    fragColor = vec4(finalColor, 1.0);
}




/**
vec4 calculateMean(vec2 start, int boxSize) {
    vec4 mean = vec4(0,0,0,0);

    for (int i = 0; i < boxSize; i++) {
        for (int j = 0; j < boxSize; j++) {
            mean += texture(tex, start + vec2(i * pixelW, j * pixelH));
        }
    }
    for (int i = 0; i < 4; i++) {
        mean[i] /= boxSize * boxSize;
    }
    return mean;
}

float calculateVariance(vec2 start, int boxSize, vec4 mean) {
    vec4 variance = vec4(0,0,0,0);

    for (int i = 0; i < boxSize; i++) {
        for (int j = 0; j < boxSize; j++) {
            vec4 x = texture(tex, start + vec2(i * pixelW, j * pixelH)) - mean;
            variance += x * x;
        }
    }
    for (int i = 0; i < 4; i++) {
        variance[i] /= boxSize * boxSize;
    }
    return dot(variance, vec4(0.299, 0.587, 0.114, 0));
}

void main()
{
    if (isSharpening) {
        int boxSize = 10;
        float lowestVariance = 1.0f / 0.0f;

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                vec2 start = out_uv + vec2(-boxSize * i, -boxSize * j);
                vec4 mean = calculateMean(start, boxSize);
                float variance = calculateVariance(start, boxSize, mean);
                if (variance < lowestVariance) {
                    fragColor = mean;
                    lowestVariance = variance;
                }
            }
        }
    }
    else {
        fragColor = texture(tex, out_uv);
    }


    if (isInverting) {
        for (int i = 0; i < 3; i++) {
            fragColor[i] = 1 - fragColor[i];
        }
    }
}**/
