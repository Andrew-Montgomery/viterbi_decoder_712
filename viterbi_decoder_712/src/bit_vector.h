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

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

class BitVector;

int HammingDistance(const BitVector &v1, const BitVector &v2);

class BitVector {
public:
    BitVector() {}
    // Initial size of zeros
    explicit BitVector(int32_t initialSize);
    BitVector(const BitVector &other);
    BitVector(BitVector &&other);
    BitVector(const char *bitString); // Null terminated string
    BitVector(const uint8_t *bits, int count); // Array of 1 or 0's

    bool operator==(const BitVector &other) const;
    bool operator!=(const BitVector &other) const;

    BitVector& operator=(const char *bitString);
    BitVector& operator=(const BitVector &other);
    BitVector& operator+=(const BitVector &other);

    uint8_t& operator[](int pos);
    const uint8_t& operator[](int pos) const;

    uint8_t At(int pos) const;

    // Extract N bits starting at pos
    BitVector Extract(int pos, int bits) const;
    BitVector ExtractAndRemove(int pos, int bits);

    // Replaces the bits in the vector with 'bits' number of bits from src
    void FromInt(uint32_t src, int bits);

    std::string ToString() const;

    // Create an int of type T, using a specific number of bits
    // Choose to remove the bits from the vector
    template<class T>
    T ToInt(int bits, bool reverse, bool removeFromVector);
    template<class T>
    T ToIntFromPos(int pos, int bits, bool reverse) const;
    template<class T>
    T ToIntFromPos(int pos, int bits, bool reverse, bool removeFromVector);

    // Append a series of bits from an integer
    // When lsbFirst is true, the least significant bits are added to the vector first
    void Append(uint32_t src, int bits, bool lsbFirst);
    void Append(const BitVector &other) { *this += other; }
    void PushBack(uint8_t bit);

    void Resize(int newSize);
    void Clear();

    // Return the number of set bits
    int Ones() const;
    int Zeros() const;

    // Number of bits
    int Length() const;
    int Size() const;

    // Set all bits to b
    void SetAll(uint8_t b);

    void FlipBit(int pos);
    BitVector& Reverse();

    std::vector<uint8_t>::iterator begin() { return v.begin(); }
    std::vector<uint8_t>::const_iterator begin() const { return v.begin(); }
    std::vector<uint8_t>::iterator end() { return v.end(); }
    std::vector<uint8_t>::const_iterator end() const { return v.end(); }

private:
    std::vector<uint8_t> v;
};

inline int HammingDistance(const BitVector &v1, const BitVector &v2)
{
    assert(v1.Length() == v2.Length());

    int distance = 0;
    for(int i = 0; i < v1.Length(); i++) {
        if(v1[i] != v2[i]) distance++;
    }

    return distance;
}

inline BitVector operator+(const BitVector &left, const BitVector &right)
{
    BitVector r = left;
    r += right;
    return r;
}

inline BitVector::BitVector(int32_t initialSize)
{
    Resize(initialSize);
    for(int i = 0; i < initialSize; i++) {
        v[i] = 0;
    }
}

inline BitVector::BitVector(const BitVector &other)
{
    v = other.v;
}

inline BitVector::BitVector(BitVector &&other)
{
    v = std::move(other.v);
}

inline BitVector::BitVector(const char *bitString)
{
    assert(bitString);
    while(*bitString) {
        if(*bitString == '0') {
            v.push_back(0);
        } else if(*bitString == '1') {
            v.push_back(1);
        } else {
            // Ignore
            assert(false);
        }
        bitString++;
    }
}

inline BitVector::BitVector(const uint8_t *bits, int count)
{
    assert(bits);
    v.resize(count);
    for(int i = 0; i < count; i++) {
        v[i] = bits[i];
    }
}

inline bool BitVector::operator==(const BitVector &other) const
{
    assert(Length() == other.Length());
    for(int i = 0; i < Length(); i++) {
        if(v[i] != other[i]) {
            return false;
        }
    }

    return true;
}

inline bool BitVector::operator!=(const BitVector &other) const
{
    return !(*this == other);
}

inline BitVector& BitVector::operator=(const char *bitString)
{
    v.clear();
    assert(bitString);
    while(*bitString) {
        if(*bitString == '0') {
            v.push_back(0);
        } else if(*bitString == '1') {
            v.push_back(1);
        } else {
            // Ignore
            assert(false);
        }
        bitString++;
    }
    return *this;
}

inline BitVector& BitVector::operator=(const BitVector &other)
{
    v = other.v;
    return *this;
}

inline BitVector& BitVector::operator+=(const BitVector &other)
{
    v.insert(end(), other.begin(), other.end());
    return *this;
}

inline uint8_t& BitVector::operator[](int pos)
{
    return v[pos];
}

inline const uint8_t& BitVector::operator[](int pos) const
{
    return v[pos];
}

inline uint8_t BitVector::At(int pos) const
{
    return v[pos];
}

// Extract N bits starting at pos
// Leave the original vector
inline BitVector BitVector::Extract(int pos, int bits) const
{
    BitVector ret;
    for(int i = 0; i < bits; i++) {
        ret.PushBack(v[pos + i]);
    }

    return ret;
}

inline BitVector BitVector::ExtractAndRemove(int pos, int bits)
{
    BitVector r = Extract(pos, bits);
    v.erase(v.begin()+pos, v.begin()+pos+bits);
    return r;
}

inline void BitVector::FromInt(uint32_t src, int bits)
{
    v.clear();
    for(int i = 0; i < bits; i++) {
        v.push_back(src & 1);
        src >>= 1;
    }
}

inline std::string BitVector::ToString() const
{
    std::string s;
    for(int i = 0; i < Length(); i++) {
        s.push_back((v[i] == 0) ? '0' : '1');
    }
    return s;
}

// Create an int of type T, using a specific number of bits
// Choose to remove the bits from the vector
template<class T>
inline T BitVector::ToInt(int bits, bool reverse, bool removeFromVector)
{
    return ToIntFromPos<T>(0, bits, reverse, removeFromVector);
}

//template<class T>
//inline T BitVector::ToIntFromPos(int pos, int bits) const
//{
//    T intType = 0;
//    for(int i = 0; i < bits; i++) {
//        intType <<= 1;
//        intType |= v[i + pos];
//    }
//    return intType;
//}

template<class T>
inline T BitVector::ToIntFromPos(int pos, int bits, bool reverse) const
{
    T intType = 0;

    if(reverse) {
        for(int i = 0; i < bits; i++) {
            intType <<= 1;
            intType |= v[i + pos];
        }
    } else {
        for(int i = 0; i < bits; i++) {
//            intType |= ((v[i+pos]) << (bits-i-1));
            intType |= ((v[i+pos]) << i);
        }
    }

    return intType;
}

// If reverse is true, MSB first
template<class T>
inline T BitVector::ToIntFromPos(int pos, int bits, bool reverse, bool removeFromVector)
{
    T intType = 0;

    if(reverse) {
        for(int i = 0; i < bits; i++) {
            intType <<= 1;
            intType |= v[i + pos];
        }
    } else {
        for(int i = 0; i < bits; i++) {
            intType |= ((v[i+pos]) << i);
        }
    }

    if(removeFromVector) {
        v.erase(v.begin()+pos, v.begin()+pos+bits);
    }
    return intType;
}

// Append a series of bits from an integer
inline void BitVector::Append(uint32_t src, int bits, bool lsbFirst)
{
    if(lsbFirst) {
        for(int i = 0; i < bits; i++) {
            v.push_back(src & 1);
            src >>= 1;
        }
    } else {
        for(int i = 0; i < bits; i++) {
            v.push_back((src >> (bits-i-1)) & 1);
        }
    }
}

inline void BitVector::PushBack(uint8_t bit)
{
    assert(bit <= 1);
    v.push_back(bit & 1);
}

inline void BitVector::Resize(int newSize)
{
    v.resize(newSize);
}

inline void BitVector::Clear()
{
    v.clear();
}

inline int BitVector::Ones() const
{
    int ones = 0;
    for(int i = 0; i < Length(); i++) {
        if(At(i) == 1) {
            ones++;
        }
    }
    return ones;
}

inline int BitVector::Zeros() const
{
    return Length() - Ones();
}

inline int BitVector::Length() const
{
    return (int)v.size();
}

inline int BitVector::Size() const
{
    return (int)v.size();
}

inline void BitVector::SetAll(uint8_t b)
{
    for(int i = 0; i < Length(); i++) {
        v[i] = b;
    }
}

inline void BitVector::FlipBit(int pos)
{
    // Modulo 2 adder
    v[pos] = (v[pos] + 1) & 1;
}

inline BitVector& BitVector::Reverse()
{
    std::reverse(v.begin(), v.end());
    return *this;
}
