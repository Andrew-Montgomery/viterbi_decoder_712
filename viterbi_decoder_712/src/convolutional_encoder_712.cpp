#include "convolutional_encoder_712.h"

static uint64_t Popcount(uint64_t n)
{
    int i = 0;
    for(; n != 0; n &= n - 1) {
        i++;
    }
    return i;
}

Trellis712::Trellis712()
{
    const uint32_t g[2] = { 0x6D, 0x4F };

    uint32_t stateMask = (64 - 1);

    // Build state diagram
    for(int i = 0; i < 64; i++) {
        for(int j = 0; j < 2; j++) {
            // Calculate output
            uint32_t reg = (i*2) | j;

            for(int k = 0; k < 2; k++) {
                uint32_t output = reg & g[k];
                outputs[i][j][k] = Popcount(output) & 1;
            }

            // Calculate next state
            reg = (reg & stateMask);

            nextState[i][j] = reg;
        }
    }
}

ConvolutionalEncoder712::ConvolutionalEncoder712()
{
    puncturePattern = PuncturePattern712_12;
    currentState = 0;
}

void ConvolutionalEncoder712::SetPuncturePattern(const BitVector &newPattern)
{
    if(newPattern.Size() == 0) {
        puncturePattern = PuncturePattern712_12;
    } else {
        assert((newPattern.Size() % 2) == 0);
        puncturePattern = newPattern;
    }

    Reset();
}

BitVector ConvolutionalEncoder712::Encode(const BitVector &input)
{
    assert(input.Size() > 0);
    assert(puncturePattern.Size() > 0);
    assert(((2 * input.Size()) % puncturePattern.Size()) == 0);

    const int encodedLength = 2 * (puncturePattern.Ones() * input.Size()) / puncturePattern.Size();
    BitVector encoded(encodedLength);

    // Index into encoded buffer
    int encodedIx = 0;
    // Index into puncture pattern
    int punctureIx = 0;

    for(int i = 0; i < input.Size(); i++) {
        // Insert two output bits into encoded array only if puncture bit is set to 1
        for(int bit = 0; bit < 2; bit++) {
            if(puncturePattern[punctureIx++]) {
                encoded[encodedIx++] = trellis.outputs[currentState][input[i]][bit];
            }
        }

        currentState = trellis.nextState[currentState][input[i]];
        if(punctureIx >= puncturePattern.Size()) {
            punctureIx = 0;
        }
    }

    return encoded;
}

void ConvolutionalEncoder712::Reset()
{
    currentState = 0;
}
