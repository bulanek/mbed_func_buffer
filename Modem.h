#include "mbed.h"


class Modem
{
public:
    Modem();
    void receive_byte(const uint8_t byte);

private:
    static const uint8_t MODEM_MESSAGE_START[4];
    static const uint8_t MODEM_MESSAGE_END[4];
    static const uint8_t MODEM_LENGTH_HEADER_SIZE = 2;
    static const uint16_t TIMEOUT_MS = 100U;
    static const uint16_t MAX_SIZE_PAYLOAD_BYTES = 0xFF;


    void reset_receive(void);


    uint8_t* _pBuffer;
    uint16_t _numReceived;
    uint16_t _size;
    Timer _timer;
};