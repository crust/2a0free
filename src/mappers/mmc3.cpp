#include "mmc3.h"
#include <iostream>

const int PRG_PAGE_SIZE = 0x2000;
const int CHR_PAGE_SIZE = 0x400;

MMC3::MMC3(IBus *bus)
    : _bus(bus)
{
    for (int i = 0; i < 8; ++i)
    {
        std::cout << "MMC3 R" << i << ": " << (int)_reg8001[i] << "\n";
    }
}

MMC3::~MMC3() {}

void MMC3::write_chr(uint8_t value, uint16_t addr)
{
    // TODO (?)
}

uint8_t MMC3::read_prg(uint16_t addr) const
{
    if (addr < 0x6000)
    {
        // ??
        std::cerr << "MMC3 read from " << std::hex << addr << "\n";
        throw 1;
    }
    else if (addr < 0x8000)
    {
        return 0;
        //return _ram[addr % 0x2000];
    }
    else
    {
        addr -= 0x8000;
        return mirrored_prg(addr, 0x2000);
    }
}

uint8_t MMC3::read_chr(uint16_t addr) const
{

    if ((addr & 0x1000) && !_edge)
    {
        _edge = true;

        if (_IRQ_counter == 0)
        {

            _IRQ_counter = _IRQ_reload;
        }
        else if (--_IRQ_counter == 0 && _IRQ_enabled)
        {

            _bus->pull_IRQ();
            _IRQ_pending = true;
        }
    }
    else if (!(addr & 0x1000))
    {
        _edge = false;
    }

    if (addr > 0x1fff)
    {
        return 0;
    }

    return mirrored_chr(addr, 0x400);
}

void MMC3::on_set_prg(uint8_t count)
{
    _prg_bank[0] = 0;
    _prg_bank[1] = 0;
    _prg_bank[2] = 0;
    _prg_bank[3] = _prg->size() - PRG_PAGE_SIZE;
}

size_t MMC3::get_chr_size_for_count(uint8_t count) const
{
    if (!count)
    {
        count = 0x2000 / CHR_PAGE_SIZE;
    }
    return count * 0x2000;
}

void MMC3::on_set_chr(uint8_t count)
{
    for (size_t i = 0; i < 8; ++i)
    {
        _chr_bank[i] = 0;
    }

    write_prg(0, 0x8000);
    write_prg(0, 0x8001);
    write_prg(0, 0xa000);
    write_prg(0, 0xa001);
    write_prg(0, 0xc000);
    write_prg(0, 0xc001);
    write_prg(0, 0xe000);
    write_prg(0, 0xe001);

    setup();
}

void MMC3::write_prg(uint8_t value, uint16_t addr)
{
    if (addr < 0x6000)
    {
        // ??
        std::cerr << "MMC3 write to " << std::hex << addr << " (" << (int)value << ")\n";
        return;
    }
    else if (addr < 0x8000)
    {

        //_ram[addr % 0x2000] = value;
    }
    else
    {
        switch (addr & 0xe001)
        {
        case 0x8000:
            _reg8000 = value;
            break;

        case 0x8001:
            _reg8001.at(_address) = value;
            if (_address <= 1)
            {
                _reg8001.at(_address) &= 0xfe;
            }
            else if (_address >= 6)
            {
                _reg8001.at(_address) &= ~0xc0;
            }
            break;

        case 0xa000:
            _mirroring = value & 1;
            set_mirroring(
                _mirroring ? HORIZONTAL : VERTICAL);
            break;

        case 0xa001:
            _regA001 = value;
            break;

        case 0xc000:
            _IRQ_reload = value;
            break;

        case 0xc001:
            _IRQ_counter = 0;
            break;

        case 0xe000:
            _IRQ_enabled = false;
            _IRQ_pending = false;
            break;

        case 0xe001:
            _bus->release_IRQ();
            if (!_IRQ_enabled)
            {
                _IRQ_enabled = true;
            }
            break;
        }
    }
    setup();
}

void MMC3::setup()
{
    std::array<uint8_t, 8> &R = _reg8001;
    if (!_prg_mode_1)
    {
        _prg_bank[0] = R[6] * PRG_PAGE_SIZE;
        _prg_bank[1] = R[7] * PRG_PAGE_SIZE;
        _prg_bank[2] = _prg->size() - PRG_PAGE_SIZE * 2;
        _prg_bank[3] = _prg->size() - PRG_PAGE_SIZE;
    }
    else
    {
        _prg_bank[0] = _prg->size() - PRG_PAGE_SIZE * 2;
        _prg_bank[1] = R[7] * PRG_PAGE_SIZE;
        _prg_bank[2] = R[6] * PRG_PAGE_SIZE;
        _prg_bank[3] = _prg->size() - PRG_PAGE_SIZE;
    }

    auto R0_a = R[0] & 0xfe;
    auto R0_b = R[0] | 1;
    auto R1_a = R[1] & 0xfe;
    auto R1_b = R[1] | 1;
    if (!_chr_mode_1)
    {
        _chr_bank[0] = R0_a * CHR_PAGE_SIZE;
        _chr_bank[1] = R0_b * CHR_PAGE_SIZE;
        _chr_bank[2] = R1_a * CHR_PAGE_SIZE;
        _chr_bank[3] = R1_b * CHR_PAGE_SIZE;
        _chr_bank[4] = R[2] * CHR_PAGE_SIZE;
        _chr_bank[5] = R[3] * CHR_PAGE_SIZE;
        _chr_bank[6] = R[4] * CHR_PAGE_SIZE;
        _chr_bank[7] = R[5] * CHR_PAGE_SIZE;
    }
    else
    {
        _chr_bank[0] = R[2] * CHR_PAGE_SIZE;
        _chr_bank[1] = R[3] * CHR_PAGE_SIZE;
        _chr_bank[2] = R[4] * CHR_PAGE_SIZE;
        _chr_bank[3] = R[5] * CHR_PAGE_SIZE;
        _chr_bank[4] = R0_a * CHR_PAGE_SIZE;
        _chr_bank[5] = R0_b * CHR_PAGE_SIZE;
        _chr_bank[6] = R1_a * CHR_PAGE_SIZE;
        _chr_bank[7] = R1_b * CHR_PAGE_SIZE;
    }
}
