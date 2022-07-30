#pragma once

template <typename A, typename B, int size> class CSaltBufferArray {
    struct SSaltBufferElement// Size=0x4 (Id=1944476)
    {
        A m_Salt;// Offset=0x0 Size=0x2
        B m_NextIndex;// Offset=0x2 Size=0x2
    };
    SSaltBufferElement m_Buffer[size];// Offset=0x0 Size=0x3fff4
private:
    A m_FreeListStartIndex;// Offset=0x3fff4 Size=0x2
    B m_maxUsed;// Offset=0x3fff6 Size=0x2
};