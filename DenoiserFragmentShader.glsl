#version 430 core
in vec2 TexCoords;

uniform int screenWidth;
uniform int screenHeight;

layout(location = 0) out vec4 FragColor;

uniform sampler2D screenTexture;
uniform sampler2D screenNormal;
uniform sampler2D screenPosition;

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

float hi[25] = { 1.f / 16.f, 1.f / 16.f, 1.f / 16.f, 1.f / 16.f, 1.f / 16.f,
                 1.f / 16.f, 1.f / 4.f, 1.f / 4.f, 1.f / 4.f, 1.f / 16.f,
                 1.f / 16.f, 1.f / 4.f, 3.f / 8.f, 1.f / 4.f, 1.f / 16.f,
                 1.f / 16.f, 1.f / 4.f, 1.f / 4.f, 1.f / 4.f, 1.f / 16.f,
                 1.f / 16.f, 1.f / 16.f, 1.f / 16.f, 1.f / 16.f, 1.f / 16.f };


int bit_log2(int val)
{
    int ret = 0;
    while (bool(val >>= 1))
        ret++;
    return ret;
}

void FilterWavelet()
{

    float m_sigmaPlane = 0.1f;
    float m_sigmaColor = 0.6f;
    float m_sigmaNormal = 0.1f;
    float m_sigmaCoord = 32.0f;

    int height = screenHeight;
    int width = screenWidth;
    int kernelRadius = 32;
    int y = int(round(height * TexCoords.y));
    int x = int(round(width * TexCoords.x));

    // std::cout << "filter::����ÿ������" << std::endl;
    // TODO: Joint bilateral filter
    // filteredImage(x, y) = frameInfo.m_beauty(x, y);
    // ȷ���˲��˷�Χ
    //int x_start = max(0, x - kernelRadius);
    //int x_end = min(x + kernelRadius, width - 1);
    //int y_start = max(0, y - kernelRadius);
    //int y_end = min(y + kernelRadius, height - 1);

    // ȷ���ĸ�����
    vec3 this_beauty = texture(screenTexture, TexCoords).rgb;
    vec3 this_normal = texture(screenNormal, TexCoords).rgb;
    vec3 this_position = texture(screenPosition, TexCoords).rgb;

    // ����������
    vec3 this_result = vec3(0.0f);
    // ��һ��ϵ��
    float weightSum = 0.0f;

    int times = bit_log2(kernelRadius * 2 / 4);
    //times = 3;
    //times = 3;
    for (int i = 0; i <= times; ++i)
    {
        int interval = (1 << i);
        //interval = int(pow(2, times));

        int x_start = max(0, x - 2 * interval);
        int x_end = min(x + 2 * interval, width - 1);
        int y_start = max(0, y - 2 * interval);
        int y_end = min(y + 2 * interval, height - 1);
        for (int _y = y_start; _y <= y_end; _y += interval) {
            for (int _x = x_start; _x <= x_end; _x += interval) {
                // std::cout << "filter::�˲���ʼ" << std::endl;
                // �������ص��ĸ�����
                vec2 _TexCoords = vec2(_x / float(width), _y / float(height));
                vec3 other_beauty = texture(screenTexture, _TexCoords).rgb;
                vec3 other_noraml = texture(screenNormal, _TexCoords).rgb;
                vec3 other_position = texture(screenPosition, _TexCoords).rgb;

                // ����Ȩ��
                // ��������
                vec3 _position = this_position - other_position;
                float lengthPos = distance(this_position, other_position);
                float weightPos = lengthPos * lengthPos / (2 * m_sigmaCoord * m_sigmaCoord);
                //float weightPos = lengthPos / (2.0f * m_sigmaCoord * m_sigmaCoord);


                // ��ɫ����
                float lengthCol = distance(this_beauty, other_beauty);
                float weightCol = lengthCol * lengthCol / (2 * m_sigmaColor * m_sigmaColor);
                //float weightCol = lengthCol / (2.0f * m_sigmaColor * m_sigmaColor);

                // ����������
                float normal_cos = dot(this_normal, other_noraml);
                float D_normal = acos(normal_cos);
                float weightNor = D_normal * D_normal / (2.0f * m_sigmaNormal * m_sigmaNormal);

                // �������
                vec3 _dirction;
                if (lengthPos > 0.f) {
                    // ��ֹnoramlizeһ��0�������±���
                    _dirction = normalize(_position);
                }
                //vec3 _dirction = Normalize(this_position - other_position);
                float D_plane = dot(this_normal, _dirction);
                float weightPla = D_plane * D_plane / (2.0f * m_sigmaPlane * m_sigmaPlane);

                // Ȩ�ؼ���
                //if (i != 0)
                //    continue;
                float weight = exp(-weightPos - weightCol - weightNor - weightPla);
                this_result += other_beauty * weight;
                weightSum += weight;

            }
        }
    }
    // std::cout << "weightSum:" << weightSum << std::endl;
    if (weightSum < 0.000001) {
        // ��ʼȨ��̫С��float���Ȳ�������õ�0
        FragColor = vec4(this_beauty, 1);
        // weightSum = 0.001f;
    }
    else {
        // ��ɸ�ֵ
        FragColor = vec4(this_result / weightSum, 1);
        //FragColor = vec4(this_result / weightSum, 1);
    }
}

void main() 
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(col, 1.0);
    //return;
    FilterWavelet();
}

