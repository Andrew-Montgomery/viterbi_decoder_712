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

#include "bit_vector.h"
#include "convolutional_encoder_712.h".h"

// Hard decision Viterbi Decoder for the 7,1,2 [171, 133] polynomial.
// Uses BitVector class for input/outputs.
// Puncture pattern and traceback depth can be set by user.
// Can operate as a continuous or terminated decoder.
// Continuous decoding will return 'TracebackDepth' 0 bits after a reset.
// Continuous decoding assumes start state is zero.
// Terminated decoding assuming first and last state is zero.
class ViterbiDecoder712H {
    // Input rate
    static const uint32_t k = 1;
    // Output rate
    static const uint32_t n = 2;
    // Constraint size
    static const uint32_t K = 7;
    // Number of unique states in 7,1,2 encoder
    static const uint32_t STATES = 64;

public:
    ViterbiDecoder712H();

    // Setting a new traceback depth resets the decode state.
    void SetTracebackDepth(uint32_t depth);
    uint32_t GetTracebackDepth() const;
    // Setting a new puncture pattern resets the decode state.
    void SetPuncturePattern(const BitVector &pattern);
    bool HasPuncturePattern() const;

    // Treat input as continous stream using previous state.
    // Uses last state as start of decode unless Reset is called.
    // Same functionality as 'Continuous' TerminationMethod in MATLAB.
    BitVector Decode(const BitVector &input);
    // Treat input independently.
    // Same functionality as 'Terminated' mode in MATLAB.
    // Starts with reset, assumes first and last state is zero.
    // Zero-pads to flush
    // Ends with reset
    BitVector DecodeTerminated(const BitVector &input);

    // Resets/Flushes all variables
    void Reset();

private:
    // Hamming distance between a and b (2 bits) ignoring the bits
    // if the associated bit in p is set to zero.
    uint8_t HD(const uint8_t a[2], const uint8_t b[2], const uint8_t p[2]);
    BitVector Depuncture(const BitVector &input);

    Trellis712 trellis;
    // User supplied puncture pattern
    BitVector puncturePattern;
    // Rolling index through puncture pattern used for marking which bits
    //   to ignore when calculating Hamming Distance
    uint32_t punctureIndex;
    // Rolling index through puncture pattern used for depuncturing
    uint32_t depunctureIndex;
    // User specified
    int tracebackDepth;
    // The decision matric.
    // Allocated up front to match the traceback depth.
    // TracebackDepth * 64 values. Each value is the previous state into that state.
    std::vector<std::vector<uint8_t>> decisions;
    // Tracks our current position in the decision table
    int decisionPos;

    // Stored Hamming distance and previous state hamming distance
    uint32_t hammingDistance[2][64];
    // Pointers to the two hamming distance arrays. Swapped on each bit decision.
    uint32_t *prevMetric, *currMetric;
};
