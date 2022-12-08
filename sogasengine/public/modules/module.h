#pragma once

namespace Sogas
{
    class IModule
    {
    public:
        IModule(const std::string name) : name(name) {};

        const std::string& GetName() { return name; }
        bool IsActive() { return bIsActive; }

    protected:
        virtual bool Start() { return true; }
        virtual void Stop() {};
        virtual void Update(f32 /*dt*/) {}
        virtual void Render() {};
        virtual void RenderDebug() {};
        virtual void RenderInMenu() {};
        virtual void RenderUI() {};
        virtual void RenderUIDebug() {};

    private:
        bool DoStart()
        {
            SASSERT( bIsActive == false );
            if(bIsActive) return false;

            const bool ok = Start();
            if(ok)
                bIsActive = true;
            return ok;
        }

        void DoStop()
        {
            SASSERT(bIsActive);
            if(!bIsActive) return;
                Stop();
            bIsActive = false;
        }

        friend class CModuleManager;

        std::string name;
        bool bIsActive = false;
    };

    using VModules = std::vector<IModule*>;
    using GameState = VModules;

} // Sogas
