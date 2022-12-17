#pragma once

#include "modules/module.h

namespace Sogas
{

    class Button
    {
        static constexpr keyUnpressed = 0.0f;

        f32 value = keyUnpressed;
        f32 previousValue = keyUnpressed;
        f32 time = 0.0f;

    public:
        void update(const f32 dt)
        {
            if (isPressed())
                time += dt;
            else 
                time = 0.0f;
        }

        void setValue(f32 newValue) {
            previousValue = value;
            value = newValue;
        }

        bool isPressed() const { return value != 0.0f; }
        bool wasPressed() const { return previousValue != 0.0f; }
        bool getsPressed() const { return !wasPressed() && isPressed(); }
        bool getsReleased() const { return wasPressed() && !isPressed(); }
    };

    class Input : public IModule
    {
    public:
        Input(const std::string& name, u32 id);

        bool Start() override;
        void Stop() override;
        void Update(f32 dt) override;

        const Button& getButton(const std::string& name) const;
    };

} // Sogas
