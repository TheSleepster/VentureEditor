/* ========================================================================
   $File: venture_buffer.cpp $
   $Date: Thu, 27 Mar 25: 06:44PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

struct venture_text_buffer
{
    string_u8 BufferName;
    string_u8 Filepath;
    string_u8 BufferData;
    uint32    CursorBufferPos;

    bool8     IsDirty;
    bool8     IsActiveBuffer;
};

struct editor_state
{
    venture_text_buffer *OpenBuffers;
};
