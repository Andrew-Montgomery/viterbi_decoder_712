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

#include "viterbi_decoder_712.h"

#include <iostream>
#include <limits>

ViterbiDecoder712H::ViterbiDecoder712H()
{
    puncturePattern = PuncturePattern712_12;
    tracebackDepth = Traceback712_12;
    decisionPos = 0;
    prevMetric = nullptr;
    currMetric = nullptr;
}

void ViterbiDecoder712H::SetTracebackDepth(uint32_t depth)
{
    assert(depth > 0);
    tracebackDepth = depth;
    Reset();
}

uint32_t ViterbiDecoder712H::GetTracebackDepth() const
{
    return tracebackDepth;
}

void ViterbiDecoder712H::SetPuncturePattern(const BitVector &pattern)
{
    puncturePattern = pattern;

    if(puncturePattern.Size() == 0) {
        puncturePattern = PuncturePattern712_12;
    }

    Reset();
}

BitVector ViterbiDecoder712H::GetPuncturePattern() const
{
    return puncturePattern;
}

BitVector ViterbiDecoder712H::Decode(const BitVector &input)
{
    assert(input.Length() % puncturePattern.Ones() == 0);

    // Perform depuncturing up front
    // Create new array where we insert the punctured data with a zero
    // Later on, the HammingDistanceViterbi function will ignore this bit.
    BitVector depunctured;

    // Number of times we will iterate through the puncture pattern
    int iter = input.Length() / puncturePattern.Ones();
    depunctured.Resize(iter * puncturePattern.Length());

    int srcIndex = 0;
    for(int i = 0; i < iter; i++) {
        for(int j = 0; j < puncturePattern.Length(); j++) {
            if(puncturePattern[j] == 1) {
                depunctured[i*puncturePattern.Length() + j] = input[srcIndex++];
            } else {
                // Set any punctured values to zero, doesn't matter value is used
                depunctured[i*puncturePattern.Length() + j] = 0;
            }
        }
    }

    assert((depunctured.Length() % N) == 0);

    // How many input decoded bits are in the message
    int iters = depunctured.Size() / N;

    BitVector decoded;
    decoded.Resize(iters);

    // Pointer to depunctured bits
    const uint8_t *s = &depunctured[0];
    int punctureIndex = 0;

    // Go through each trellis column
    for(int i = 0; i < iters; i++) {
        // Pointer to decision column
        uint8_t *d = &decisions[decisionPos][0];

        // For each state transition find the potential previous states
        // Calculate the min hamming distance for all transitions into a state
        // Store the smallest hamming distance transition
        // Accumulate the hamming distance as we move through the trellis
        // This loop is taking advantage of symmetries in the 7,1,2 encoder state diagram.
        // Since every state goes to two other states we only have to loop through STATES/2 times.
        for(int state = 0; state < 32; state++) {
            // Calculate metrics of new coded bits without and with current full path metrics.
            uint32_t m[2], fm[2];

            m[0] = HD(s, trellis.outputs[state][0], &puncturePattern[punctureIndex]);
            m[1] = HD(s, trellis.outputs[state][1], &puncturePattern[punctureIndex]);

            fm[0] = prevMetric[state] + m[0];
            fm[1] = prevMetric[state+32] + m[1];
            uint32_t p1 = (fm[0] <= fm[1]) ? 0 : 1;
            currMetric[state*2] = fm[p1];
            *d++ = state + 32 * p1;

            fm[0] = prevMetric[state] + m[1];
            fm[1] = prevMetric[state+32] + m[0];
            uint32_t p2 = (fm[0] <= fm[1]) ? 0 : 1;
            currMetric[state*2+1] = fm[p2];
            *d++ = state + 32 * p2;
        }

        // Traceback, find the state with the current best path metric.
        uint32_t bestFinalState = 0;
        uint32_t bestHamming = currMetric[0];

        for(int i = 1; i < STATES; i++) {
            if(currMetric[i] < bestHamming) {
                bestHamming = currMetric[i];
                bestFinalState = i;
            }
        }

        // Trace back the best path, only want the input from the first
        // step in the traceback.
        int pathPos = decisionPos;
        int cState = bestFinalState;
        for(int i = 0; i < decisions.size() - 1; i++) {
            cState = decisions[pathPos][cState];

            pathPos--;
            if(pathPos < 0) {
                pathPos = decisions.size() - 1;
            }
        }

        // Append the first input to our traceback
        decoded[i] = cState & 0x1;

        // Advance and wrap trellis position.
        decisionPos++;
        if(decisionPos >= decisions.size()) {
            decisionPos = 0;
        }

        // Swap path metric pointers
        uint32_t *tmp = prevMetric;
        prevMetric = currMetric;
        currMetric = tmp;

        // Advance input
        s += N;
        // Advance puncture pattern index
        punctureIndex += 2;
        if(punctureIndex >= puncturePattern.Size()) {
            punctureIndex = 0;
        }
    }

    assert(punctureIndex == 0);

    return decoded;
}

BitVector ViterbiDecoder712H::DecodeTerminated(const BitVector &input)
{
    assert(input.Size() % puncturePattern.Ones() == 0);
    uint32_t returnSize = ((input.Size() * puncturePattern.Size()) / puncturePattern.Ones()) / 2;

    // Reset trellis before and after a terminated decode
    Reset();

    BitVector decoded = Decode(input);

    // Append enough zeros to satisfy the puncture pattern ratio and flush the full traceback.
    int zerosToPad = ceil((double)(tracebackDepth * N) / puncturePattern.Ones())
            * puncturePattern.Ones();

    BitVector tail(zerosToPad);
    tail.SetAll(0);

    decoded += Decode(tail);

    // Reset trellis before and after a terminated decode
    Reset();

    return decoded.Extract(tracebackDepth, returnSize);
}

uint8_t ViterbiDecoder712H::HD(const uint8_t a[2], const uint8_t b[2], const uint8_t p[2])
{
    return ((a[0] ^ b[0]) & p[0]) + ((a[1] ^ b[1]) & p[1]);
}

void ViterbiDecoder712H::Reset()
{
    // Need tracebackDepth+1 to ensure we have tracebackDepth previous states
    int trellisDepth = tracebackDepth + 1;
    decisions.resize(trellisDepth);

    // Create each trellis column (states)
    for(int i = 0; i < decisions.size(); i++) {
        decisions[i].resize(STATES);
        for(int j = 0; j < STATES; j++) {
            decisions[i][j] = 0;
        }
    }

    decisionPos = 1;

    // Reset path metric pointers
    prevMetric = hammingDistance[0];
    currMetric = hammingDistance[1];

    // Initialize metrics to force zero starting state.
    for(int i = 0; i < STATES; i++) {
        prevMetric[i] = std::numeric_limits<uint32_t>::max() / 2;
        currMetric[i] = 0;
    }
    // This forces the zero starting state.
    prevMetric[0] = 0;
}

