#include "debug_draw_font.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace Piccolo
{
    void DebugDrawFont::loadFont()
    {
        std::string str = g_runtime_global_context.m_config_manager->getEditorFontPath().string();
        const char* fontFilePath = str.c_str();
        FILE* fontFile = fopen(fontFilePath, "rb");
        if (fontFile == NULL)
        {
            std::runtime_error("debug draw cannot open font.ttf");
        }
        fseek(fontFile, 0, SEEK_END);
        uint64_t size = ftell(fontFile);
        fseek(fontFile, 0, SEEK_SET);

        stbtt_fontinfo fontInfo;
        unsigned char* fontBuffer = (unsigned char*)calloc(size, sizeof(unsigned char));
        fread(fontBuffer, size, 1, fontFile);
        fclose(fontFile);
        
        if (!stbtt_InitFont(&fontInfo, fontBuffer, 0))
        {
            std::runtime_error("debug draw stb init font failed\n");
        }

        unsigned char* bitmap = (unsigned char*)calloc(m_bitmap_w * m_bitmap_h, sizeof(unsigned char));

        float pixels = m_singleCharacterHeight - 2;
        float scale = stbtt_ScaleForPixelHeight(&fontInfo, pixels);

        int c_x1, c_y1, c_x2, c_y2;
        int ascent = 0;
        int descent = 0;
        int lineGap = 0;
        stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
        ascent = roundf(ascent * scale);
        descent = roundf(descent * scale);
        

        int x = 0;
        for (unsigned char character = m_range_l; character <= m_range_r; character++)
        {
            int advanceWidth = 0;
            int leftSideBearing = 0;
            stbtt_GetCodepointHMetrics(&fontInfo, (unsigned char)character, &advanceWidth, &leftSideBearing);

            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(&fontInfo, character, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
            
            int y = ascent + c_y1 - 2;
            int byteOffset = roundf(leftSideBearing * scale) + (character - m_range_l) % m_numOfCharacterInOneLine * m_singleCharacterWidth + ((character - m_range_l)/ m_numOfCharacterInOneLine * m_singleCharacterHeight + y) * m_bitmap_w;
            
            stbtt_MakeCodepointBitmap(&fontInfo, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, m_bitmap_w, scale, scale, character);

            x += roundf(advanceWidth * scale);

            int kern;
            kern = stbtt_GetCodepointKernAdvance(&fontInfo, character, (unsigned char)(character + 1));
            x += roundf(kern * scale);

        }
        std::vector<float> imageData(m_bitmap_w * m_bitmap_h);
        for (int i = 0; i < m_bitmap_w * m_bitmap_h; i++)
        {
            imageData[i] = static_cast<float>(*(bitmap + i))/255.0f;
        }
        
        std::shared_ptr<RHI> rhi = g_runtime_global_context.m_render_system->getRHI();
        rhi->createGlobalImage(m_font_image, m_font_imageView, m_allocation, m_bitmap_w, m_bitmap_h, imageData.data(), RHIFormat::RHI_FORMAT_R32_SFLOAT);

        free(fontBuffer);
        free(bitmap);
    }

    void DebugDrawFont::getCharacterTextureRect(const unsigned char character, float &x1, float &y1, float &x2, float &y2)
    {
        if (character >= m_range_l && character <= m_range_r)
        {
            x1 = (character - m_range_l) % m_numOfCharacterInOneLine * m_singleCharacterWidth * 1.0f / m_bitmap_w;
            x2 = ((character - m_range_l) % m_numOfCharacterInOneLine * m_singleCharacterWidth + m_singleCharacterWidth)* 1.0f / m_bitmap_w;
            y1 = (character - m_range_l) / m_numOfCharacterInOneLine * m_singleCharacterHeight * 1.0f / m_bitmap_h;
            y2 = ((character - m_range_l) / m_numOfCharacterInOneLine * m_singleCharacterHeight + m_singleCharacterHeight)* 1.0f / m_bitmap_h;
            
        }
        else
        {
            x1 = x2 = y1 = y2 = 0;
        }
    }

    void DebugDrawFont::inialize()
    {
        loadFont();
    }

    void DebugDrawFont::destroy()
    {
        std::shared_ptr<RHI> rhi = g_runtime_global_context.m_render_system->getRHI();
        rhi->freeMemory(m_font_imageMemory);
        rhi->destroyImageView(m_font_imageView);
        rhi->destroyImage(m_font_image);
    }

    RHIImageView* DebugDrawFont::getImageView() const
    {
        return m_font_imageView;
    }
}