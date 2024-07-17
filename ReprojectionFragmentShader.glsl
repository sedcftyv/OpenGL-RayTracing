#version 430 core
in vec2 TexCoords;

uniform int screenWidth;
uniform int screenHeight;
uniform mat4 historyWToS;
uniform mat4 nowWToS;
uniform mat4 historySToW;
uniform mat4 nowSToW;
uniform bool flag;

layout(location = 0) out vec4 FragColor;
//layout(location = 1) out vec4 screen;
//layout(location = 2) out vec4 histscreen;
//layout(location = 3) out int nowID;
//layout(location = 4) out int hisID;
//layout(location = 5) out int histID;

uniform sampler2D screenTexture;
uniform sampler2D screenPosition;
uniform isampler2D screenID;
uniform sampler2D historyTexture;
uniform isampler2D historyID;
//uniform sampler2D historyPosition;

struct Camera {
    vec3 camPos;
    vec3 front;
    vec3 right;
    vec3 up;
    float halfH;
    float halfW;
    vec3 leftbottom;
    int LoopNum;
};

uniform struct Camera camera;

void Reprojection() {

    float m_alpha = 0.2f;
    float m_colorBoxK = 1.0f;

    int height = screenHeight;
    int width = screenWidth;
    int y = int(round(height * TexCoords.y));
    int x = int(round(width * TexCoords.x));


    vec3 this_position = texture(screenPosition, TexCoords).rgb;
    int this_id = int(texture(screenID, TexCoords).r);

    //histID = -2;
    //nowID = this_id;
    //hisID = int(texture(historyID, TexCoords).r);
    if (this_id == -1) 
        return;

    vec4 other_screenPosition = historyWToS * vec4(this_position, 1.f);

    other_screenPosition /= other_screenPosition.w;

    //screen = vec4(TexCoords.x, TexCoords.y, 0, 1);
    //histscreen = vec4(other_screenPosition.x, other_screenPosition.y, 0, 1);

    int other_x = int(other_screenPosition.x * width);
    int other_y = int(other_screenPosition.y * height);

    if (other_x < 0 || other_x > width - 1 || other_y < 0 || other_y > height - 1)
        return;

    //histID = texture(historyID, vec2(other_screenPosition)).r;

    if (int(texture(historyID, vec2(other_screenPosition)).r) != this_id)
        return;

    vec3 hist = texture(historyTexture, vec2(other_screenPosition)).rgb;

    int kernelRadius = 3;
    int x_start = max(0, x - kernelRadius);
    int x_end = min(width - 1, x + kernelRadius);
    int y_start = max(0, y - kernelRadius);
    int y_end = min(height - 1, y + kernelRadius);

    vec3 mu = vec3(0.f);
    vec3 sigma = vec3(0.f);
    vec3 col = texture(screenTexture, TexCoords).rgb;
    for (int m = x_start; m <= x_end; m++) {
        for (int n = y_start; n <= y_end; n++) {
            vec3 tmp = texture(screenTexture, vec2(m / float(width), n / float(height))).rgb;
            mu += tmp;
            sigma += (col - tmp) * (col - tmp);
        }
    }

    int count = kernelRadius * 2 + 1;
    count *= count;

    mu /= count;
    sigma = sqrt(sigma / count);
    hist = clamp(hist, mu - sigma * m_colorBoxK, mu + sigma * m_colorBoxK);
    FragColor = vec4(mix(hist, col, m_alpha), 1);

}

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    //vec3 hist = texture(historyTexture, TexCoords).rgb;
    //FragColor = vec4(mix(hist, col, 0.2), 1);
    FragColor = vec4(col, 1);

    if (flag)
    {
        //FragColor = vec4(1, 0, 0, 1);
        Reprojection();
    }
}

