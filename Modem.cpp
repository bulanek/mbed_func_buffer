#include "Modem.h"

const uint8_t Modem::MODEM_MESSAGE_START[4] = {0xEA, 0xEA, 0xEA, 0xEA};
const uint8_t Modem::MODEM_MESSAGE_END[4] = {0xEB, 0xEB, 0xEB, 0xEB};

Modem::Modem() : _pBuffer(nullptr), _numReceived(0), _size(0){}

void Modem::reset_receive(void)
{
    _timer.stop();
    _numReceived = 0;
    _size = 0;
    if (_pBuffer != nullptr)
    {
        free(_pBuffer);
        _pBuffer = nullptr;
    }
}

void Modem::receive_byte(const uint8_t byte)
{
    do
    {
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

        /* control timeout, if yes, stop timer and counter = 0 */
        uint32_t timeMs = chrono::duration_cast<chrono::milliseconds>(_timer.elapsed_time()).count();
        if (timeMs > TIMEOUT_MS)
        {
            this->reset_receive();
            break;
        }
        else
        {
            _timer.reset();
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
                /* control size of buffer payload */
                if (_size > MAX_SIZE_PAYLOAD_BYTES || _size == 0)
                {
                    this->reset_receive();
                    break;
                }

                if (_pBuffer != nullptr) /* should not happend */
                {
                    free(_pBuffer); 
                }

                _pBuffer = static_cast<uint8_t*>(malloc(_size));
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
        if (indexPayload < _size)
        {
            _pBuffer[indexPayload] = byte;
            break;
        }

        /* Here payload stored */
        if (indexPayload == (_size - 1))
        {

        }

    } while (0);
}
