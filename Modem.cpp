#include "Modem.h"

const uint8_t Modem::MODEM_MESSAGE_START[4] = {0xEA, 0xEA, 0xEA, 0xEA};
const uint8_t Modem::MODEM_MESSAGE_END[4] = {0xEB, 0xEB, 0xEB, 0xEB};

Modem::Modem() : _pBuffer(nullptr), _numReceived(0), _size(0),_crc(0){}

void Modem::receive_byte(const uint8_t byte)
{
    do
    {
        /* control timeout, if yes, stop timer and counter = 0 */
        uint32_t timeMs = chrono::duration_cast<chrono::milliseconds>(_timer.elapsed_time()).count();
        if (timeMs > TIMEOUT_MS)
        {
            this->reset_receive();
        }
        else
        {
            _timer.reset();
        }

        /* wait for start byte */
        if (_numReceived == 0) 
        {
            if (byte == MODEM_MESSAGE_START[0])
            {
                ++_numReceived;
                _timer.start();
            }
            break;
        }

        ++_numReceived;
        /* control start message */
        if ( _numReceived <= sizeof(MODEM_MESSAGE_START))
        {
            if (byte != MODEM_MESSAGE_START[_numReceived-1])
            {
                this->reset_receive();
            }
            break;
        } 

        /* control if size received */
        if ((_numReceived > sizeof(MODEM_MESSAGE_START)) && (_numReceived <= (sizeof(MODEM_MESSAGE_START) + MODEM_LENGTH_HEADER_SIZE)))
        {
            uint16_t indexSize = _numReceived - (sizeof(MODEM_MESSAGE_START) + 1);
            _size |= static_cast<uint16_t>(byte) << 8 * indexSize;

            /* alloc buffer */
            if (indexSize == (MODEM_LENGTH_HEADER_SIZE -1))
            {
                /* control size of buffer payload. CRC is part of size (4). */
                if (_size > (MAX_SIZE_PAYLOAD_BYTES + 4) || _size <=  4)
                {
                    this->reset_receive();
                    break;
                }

                if (_pBuffer != nullptr) /* should not happend */
                {
                    free(_pBuffer); 
                }

                _pBuffer = static_cast<uint8_t*>(malloc(_size - 4));
                /* cannot allocate memory */
                if (_pBuffer == nullptr)
                {
                    this->reset_receive();
                }
            }
            break;
        }

        /* Get payload data */
        uint16_t indexPayload = _numReceived - sizeof(MODEM_MESSAGE_START) - MODEM_LENGTH_HEADER_SIZE - 1;
        uint16_t payloadSize = _size - 4;
        if (indexPayload < payloadSize)
        {
            _pBuffer[indexPayload] = byte;
            break;
        }

        /* Get CRC for check */
        if (indexPayload < _size)
        {
            _crc |= static_cast<uint16_t>(byte) << 8 * (indexPayload - payloadSize);
            /* control of crc */
            if (indexPayload == (_size - 1))
            {
                uint32_t crc;
                if (this->compute_crc32(crc) == false)
                {
                    this->reset_receive();
                    break;
                }
                if (crc != _crc)
                {
                    this->reset_receive();
                    break;
                }
            }
            break;
        }

        /* Control of end sequence */
        uint16_t indexEnd = _numReceived - sizeof(MODEM_MESSAGE_START) - MODEM_LENGTH_HEADER_SIZE - _size - 1;
        if (indexEnd < sizeof(MODEM_MESSAGE_END))
        {
            if (byte != MODEM_MESSAGE_END[indexEnd])
            {
                this->reset_receive();
                break;
            }
            if (indexEnd == (sizeof(MODEM_MESSAGE_END) - 1))
            {
                /* TODO here end of the whole expected buffer. Now reseting */
                this->reset_receive();
            }
            break;
        }
    } while (0);
}

void Modem::reset_receive(void)
{
    _timer.stop();
    _timer.reset();
    _numReceived = 0;
    _size = 0;
    if (_pBuffer != nullptr)
    {
        free(_pBuffer);
        _pBuffer = nullptr;
    }
}

bool Modem::compute_crc32(uint32_t& crc)
{
    uint16_t payloadSize = _size - 4;
    bool retVal = true;
    do
    {
        if (_crc32.compute_partial_start(&crc) != 0)
        {
            retVal = false;
            this->reset_receive();
            break;
        }
        if (_crc32.compute_partial(reinterpret_cast<const void *>(&_size), sizeof(_size), &crc) != 0)
        {
            retVal = false;

            this->reset_receive();
            break;
        }
        if (_crc32.compute_partial(reinterpret_cast<const void *>(_pBuffer), payloadSize, &crc) != 0)
        {
            retVal = false;

            this->reset_receive();
            break;
        }
        if (_crc32.compute_partial_stop(&crc) != 0)
        {
            retVal = false;

            this->reset_receive();
            break;
        }
    } while (0);
    return retVal;
}