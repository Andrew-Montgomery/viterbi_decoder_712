// Copyright (c) 2020 Andrew Montgomery

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cstdint>
#include <vector>

#include "bit_vector.h"

// Commonly used puncture patterns and associated traceback lengths
const BitVector PuncturePattern712_12 = "11";
const BitVector PuncturePattern712_23 = "1110";
const BitVector PuncturePattern712_34 = "111001";
const BitVector PuncturePattern712_56 = "1110011001";

const uint32_t Traceback712_12 = 30;
const uint32_t Traceback712_23 = 45;
const uint32_t Traceback712_34 = 60;
const uint32_t Traceback712_56 = 90;

// Characteristics of this trellis/polynomial,
// If you are in an even numbered state [0,2,4,...] an input of zero caused you to
//   reach this state. If you are in an odd numbered state, an input of 1 caused you
//   to reach this state.
// The next states from your current state (curr) given an input bit are
// [curr] -> 0 -> [curr*2]
// [curr] -> 1 -> [curr*2+1]
// The only thing that doesn't have symmetry that can be leveraged is the output bits.
struct Trellis712 {
    Trellis712();

    // The outputs generated for each type of input and the resulting state
    uint8_t outputs[64][2][2];
    uint8_t nextState[64][2];
};

class ConvolutionalEncoder712 {
public:
    ConvolutionalEncoder712();

    // Updating the puncture pattern resets the encoder.
    void SetPuncturePattern(const BitVector &newPattern);

    // Main encode routine. Returns punctured bit vector.
    // Encoded length (inputSize*2) must be a multiple of the puncture pattern size.
    BitVector Encode(const BitVector &input);

    // Resets the internal running state of the encoder.
    void Reset();

private:
    Trellis712 trellis;
    BitVector puncturePattern;
    uint8_t currentState;
};
