#ifndef __SBM_H__
#define __SBM_H__

#include <GLES2/gl2.h>
#include <cstdio>
#include <cstring>

typedef struct SBM_HEADER_t
{
    unsigned int magic;
    unsigned int size;
    char name[64];
    unsigned int num_attribs;
    unsigned int num_frames;
    unsigned int num_vertices;
    unsigned int num_indices;
    unsigned int index_type;
} SBM_HEADER;

typedef struct SBM_ATTRIB_HEADER_t
{
    char name[64];
    unsigned int type;
    unsigned int components;
    unsigned int flags;
} SBM_ATTRIB_HEADER;

typedef struct SBM_FRAME_HEADER_t
{
    unsigned int first;
    unsigned int count;
    unsigned int flags;
} SBM_FRAME_HEADER;

typedef struct SBM_VEC4F_t
{
    float x;
    float y;
    float z;
    float w;
} SBM_VEC4F;

class SBObject
{
public:
    SBObject(void)
        : m_vao(0),
          m_attribute_buffer(0),
          m_index_buffer(0),
          m_attrib(0),
          m_frame(0)
    {
    }

    virtual ~SBObject(void)
    {
        Free();
    }

    bool LoadFromSBM(const char * filename)
    {
        FILE * f = NULL;

        f = fopen(filename, "rb");
        if(f == NULL)
        {
            return false;
        }

        fseek(f, 0, SEEK_END);
        size_t filesize = ftell(f);
        fseek(f, 0, SEEK_SET);

        m_data = new unsigned char [filesize];
        size_t readsize = fread(m_data, 1, filesize, f);
        if(readsize != filesize)
        {
            delete[] m_data;
            m_data = NULL;
            fclose(f);
            return false;
        }
        fclose(f);

        SBM_HEADER * header = (SBM_HEADER *)m_data;
        m_raw_data = m_data + sizeof(SBM_HEADER) + header->num_attribs * sizeof(SBM_ATTRIB_HEADER) + header->num_frames * sizeof(SBM_FRAME_HEADER);
        SBM_ATTRIB_HEADER * attrib_header = (SBM_ATTRIB_HEADER *)(m_data + sizeof(SBM_HEADER));
        SBM_FRAME_HEADER * frame_header = (SBM_FRAME_HEADER *)(m_data + sizeof(SBM_HEADER) + header->num_attribs * sizeof(SBM_ATTRIB_HEADER));

        memcpy(&m_header, header, sizeof(SBM_HEADER));
        m_attrib = new SBM_ATTRIB_HEADER[header->num_attribs];
        memcpy(m_attrib, attrib_header, header->num_attribs * sizeof(SBM_ATTRIB_HEADER));
        m_frame = new SBM_FRAME_HEADER[header->num_frames];
        memcpy(m_frame, frame_header, header->num_frames * sizeof(SBM_FRAME_HEADER));

        return true;
    }

    bool Free(void)
    {
        m_index_buffer = 0;
        m_attribute_buffer = 0;
        m_vao = 0;

        delete [] m_attrib;
        m_attrib = NULL;

        delete [] m_frame;
        m_frame = NULL;
        
        delete [] m_data;

        return true;
    }

    unsigned int GetAttributeCount(void) const
    {
        return m_header.num_attribs;
    }

    const char * GetAttributeName(unsigned int index) const
    {
        return index < m_header.num_attribs ? m_attrib[index].name : 0;
    }

    unsigned int GetAttribComponents(unsigned int index) const
    {
        return index < m_header.num_attribs ? m_attrib[index].components : 0;
    }

    unsigned char* GetVertexData()
    {
        return m_raw_data;
    }

    unsigned int GetNumVertices() const
    {
        return m_header.num_vertices;
    }

    unsigned int GetFirstFrameVertex(unsigned int frame) const
    {
        return (frame < m_header.num_frames) ? m_frame[frame].first : 0;
    }

    unsigned int GetFrameVertexCount(unsigned int frame) const
    {
        return (frame < m_header.num_frames) ? m_frame[frame].count : 0;
    }

protected:
    GLuint m_vao;
    GLuint m_attribute_buffer;
    GLuint m_index_buffer;

    GLuint m_num_attribs;
    GLuint m_num_verticies;
    
    GLuint m_vertexIndex;
    GLuint m_normalIndex;
    GLuint m_texCoord0Index;

    SBM_HEADER m_header;
    SBM_ATTRIB_HEADER * m_attrib;
    SBM_FRAME_HEADER * m_frame;

    unsigned char * m_data;
    unsigned char * m_raw_data;
};

#endif /* __SBM_H__ */
