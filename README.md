# viterbi_decoder_712
Reference implementation of a Viterbi decoder for the convolutional code polynomial 7,1,2 [171 133] commonly used in communications and satellite communications.

* Written in C++ with example usage and QtCreator project file.
* Uses BitVector class for inputs/outputs. (Thin wrapper over std::vector<uint8_t>)
* Hard decisions using Hamming Distance as path metric.
* Supports arbitrary puncture patterns and traceback depth lengths.
  * Provides several commonly used patterns and traceback depths.
* Supports continuous and terminated input modes.
  * Both modes start from the zero state.
  * Terminated inputs zero pad to force ending on the zero state.
  * Continuous inputs have a delay of TracebackLength with leading zeros.
* Includes encoder.