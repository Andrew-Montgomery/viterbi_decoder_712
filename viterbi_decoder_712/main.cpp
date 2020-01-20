#include <iostream>
#include <random>

#include "src/viterbi_decoder_712.h"

int main()
{
    BitVector input, encoded, decoded;

    // Create encoder and decoder. Match puncture patterns.
    ConvolutionalEncoder712 encoder;
    encoder.SetPuncturePattern(PuncturePattern712_56);

    ViterbiDecoder712H decoder;
    decoder.SetPuncturePattern(PuncturePattern712_56);
    decoder.SetTracebackDepth(Traceback712_56);

    // Create input bit vector
    int inputBits = 260 - 8;
    for(int i = 0; i < inputBits; i++) {
        input.PushBack(rand() & 0x1);
    }
    // Force final state to zero
    for(int i = 0; i < 8; i++) {
        input.PushBack(0);
    }

    // Encode and decode data
    encoded = encoder.Encode(input);
    decoded = decoder.DecodeTerminated(encoded);

    assert(input == decoded);

    return 0;
}
