# viterbi_decoder
Reference implementation of a Viterbi decoder

- C++
- Hard decisions using Hamming Distance as path metric.
- Supports puncturing.
- Supports arbitrary traceback lengths.
- Supports polynomial constraint lengths up to 16.
- Supports continuous and terminated inputs.
-- Both will start from the zero state.
-- Terminated inputs zero pad to force ending on the zero state.
-- Continuous inputs have a delay of TracebackLength with leading zeros.

- Includes encoder.