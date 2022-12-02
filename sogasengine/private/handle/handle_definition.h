#pragma once

namespace Sogas
{
    class CHandle
    {
    public:

        static const u32 nBitsType = 7;
        static const u32 nBitsIndex = 14;
        static const u32 nBitsAge = 32 - nBitsType - nBitsIndex;
        static const u32 MaxTypes = 1 << nBitsType;

        // Empty constructor, set everything as 0.
        CHandle() : type(0), externalIndex(0), age(0) {}

        CHandle( u32 newType, u32 newExternalIndex, u32 newAge) :
            type(newType), externalIndex(newExternalIndex), age(newAge)
        {}

        // Read-only getters
        u32 GetType()               const { return type; }
        u32 GetExternalIndex()      const { return externalIndex; }
        u32 GetAge()                const { return age; }
        const char* GetTypeName()   const;

        bool IsValid() const;

        bool operator==(CHandle h) const { 
            return h.type == type 
                && h.externalIndex == externalIndex 
                && h.age == age; }

        bool operator!=(CHandle h) const {
            return !(*this == h);
        }

        bool operator<(CHandle h) const {
            return h.type == type
                && h.externalIndex == externalIndex
                && h.age < age;
        }

        bool operator>(CHandle h) const {
            return h.type == type
                && h.externalIndex == externalIndex
                && h.age > age;
        }

        bool operator<=(CHandle h) const {
            return operator==(h) && operator<(h);
        }

        bool operator>=(CHandle h) const {
            return operator==(h) && operator>(h);
        }

    private:
        u32 type : nBitsType;
        u32 externalIndex : nBitsIndex;
        u32 age : nBitsAge;
    };

} // Sogas
