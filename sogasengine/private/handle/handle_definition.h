#pragma once

namespace Sogas
{

    template<typename TObj>
    class CObjectManager;

    template<typename TObj>
    CObjectManager<TObj>* GetObjectManager();

    class CHandle
    {
    public:

        static const u32 nBitsType = 7;
        static const u32 nBitsIndex = 14;
        static const u32 nBitsAge = 32 - nBitsType - nBitsIndex;
        static const u32 MaxTypes = 1 << nBitsType;

        // Empty constructor, set everything as 0.
        CHandle() : Type(0), ExternalIndex(0), Age(0) {}

        CHandle( u32 newType, u32 newExternalIndex, u32 newAge) :
            Type(newType), ExternalIndex(newExternalIndex), Age(newAge)
        {}

        template <typename TObj>
        CHandle(TObj* obj)
        {
            auto handleManager = GetObjectManager<std::remove_const<TObj>::type>();
            *this = handleManager->GetHandleFromAddress(obj);
        }

        template < typename TObj >
        CHandle Create()
        {
            auto handleManager = GetObjectManager<TObj>();
            *this = handleManager->CreateHandle();
            return *this;
        }
        void Destroy();

        // Read-only getters
        u32 GetType()               const { return Type; }
        u32 GetExternalIndex()      const { return ExternalIndex; }
        u32 GetAge()                const { return Age; }
        const char* GetTypeName()   const;
        CHandle GetOwner()          const;

        bool IsValid() const;

        void SetOwner(CHandle newOwner);

        void RenderDebug();
        void DebugInMenu();
        void Load(const json& j);
        void OnEntityCreated();

        bool operator==(CHandle h) const { 
            return h.Type == Type 
                && h.ExternalIndex == ExternalIndex 
                && h.Age == Age; }

        bool operator!=(CHandle h) const {
            return !(*this == h);
        }

        bool operator<(CHandle h) const {
            return h.Type == Type
                && h.ExternalIndex == ExternalIndex
                && h.Age < Age;
        }

        bool operator>(CHandle h) const {
            return h.Type == Type
                && h.ExternalIndex == ExternalIndex
                && h.Age > Age;
        }

        bool operator<=(CHandle h) const {
            return operator==(h) && operator<(h);
        }

        bool operator>=(CHandle h) const {
            return operator==(h) && operator>(h);
        }

        template < typename TObj >
        operator TObj* () const 
        {
            auto handleManager = GetObjectManager< std::remove_const<TObj>::type >();
            return handleManager->GetAddressFromHandle(*this);
        }

    private:
        u32 Type : nBitsType;
        u32 ExternalIndex : nBitsIndex;
        u32 Age : nBitsAge;
    };

} // Sogas
