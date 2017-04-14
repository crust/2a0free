#pragma once

#include "generator.h"

struct Pulse : Generator_with_envelope
{
    virtual ~Pulse();

    virtual void reg1_write(uint8_t value) override;
    virtual void reg3_write(uint8_t value) override;
    virtual void on_half_frame() override;
    virtual void update() override;
    virtual double sample() const override;

    void adjust_period();
    
    uint16_t t{ 0 };
    uint8_t shift{ 0 };
    uint16_t _sweep_divider{ 0 };
    uint16_t _target{ 0 };
    bool _silenced{ false };
    bool _sweep_reload{ false };

};
