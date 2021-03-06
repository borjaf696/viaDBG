#include "kmer.h"

/*
 * Single_end kmer
 */
Kmer::Kmer(const DnaSequence &ds, size_t start, size_t length):exist(true) {
    _seq = *(ds.substr(start,length));
}

void Kmer::appendRight(DnaSequence::NuclType symbol) {
    //_seq.append_with_replace_right(symbol);
    _seq.append_nuc_right(symbol);
}

void Kmer::appendLeft(DnaSequence::NuclType symbol) {
    //_seq.append_with_replace_left(symbol);
    _seq.append_nuc_left(symbol);
}

void Kmer::appendRightReplace(DnaSequence::NuclType symbol) {
    _seq.append_with_replace_right(symbol);
}

void Kmer::appendLeftReplace(DnaSequence::NuclType symbol) {
    _seq.append_with_replace_left(symbol);
}

DnaSequence::NuclType Kmer::at(size_t index) const
{
    return _seq.atRaw(index);
}

Kmer& Kmer::operator=(const Kmer &other) {
    _seq = other._seq;
    _standard = other._standard;
    return *this;
}

bool Kmer::operator<(const Kmer &other) const {
    return (_seq < other._seq);
}

bool Kmer::operator>(const Kmer &other) const {
    return (_seq > other._seq);
}

bool Kmer::operator==(const Kmer& other) const {
    return (_seq == other._seq);
}

bool Kmer::operator!=(const Kmer& other) const{
    return (_seq != other._seq);
}

//Kmer iterator -> Single_end (false)
KmerIt<false>::KmerIt(const DnaSequence *seq, size_t pos)
        :_own_seq(seq),_pos(pos)
{
    if (pos != seq->length() - Parameters::get().kmerSize+1)
        _kmer = Kmer(*seq,0,Parameters::get().kmerSize);
}

bool KmerIt<false>::operator==(const KmerIt &kmerIt) const {
    return _own_seq == kmerIt._own_seq && _pos == kmerIt._pos;
}

bool KmerIt<false>::operator!=(const KmerIt &kmerIt) const {
    return !(*this == kmerIt);
}

KmerIt<false>& KmerIt<false>::operator++() {
    size_t newPos = _pos + Parameters::get().kmerSize;
    if (newPos < _own_seq->length())
        _kmer.appendRightReplace(_own_seq->atRaw(newPos));
    ++_pos;
    return *this;
}

KmerInfo<false> KmerIt<false>::operator*() const {
    return KmerInfo<false>(_kmer,_pos);
}

//IterKmers -> Single_end (false)
KmerIt<false> IterKmers<false>::begin() {
    if (_seq.length() < Parameters::get().kmerSize)
        return this->end();
    return KmerIt<false>(&_seq,0);
}

KmerIt<false> IterKmers<false>::end() {
    return KmerIt<false>(&_seq, _seq.length()-Parameters::get().kmerSize+1);
}

/*
 * Pair_end kmer
 */

Pair_Kmer::Pair_Kmer(const DnaSequence &ds_1, const DnaSequence &ds_2,size_t s1, size_t l1):
        exist(true) {
    _seq_left = *(ds_1.substr(s1,l1));
    _seq_right = *(ds_2.substr(s1,l1));
}

void Pair_Kmer::appendRight(DnaSequence::NuclType symbol_left, DnaSequence::NuclType symbol_right) {
    _seq_left.append_with_replace_right(symbol_left);
    _seq_right.append_with_replace_right(symbol_right);
}

void Pair_Kmer::appendLeft(DnaSequence::NuclType symbol_left, DnaSequence::NuclType symbol_right) {
    _seq_left.append_with_replace_left(symbol_left);
    _seq_right.append_with_replace_left(symbol_right);
}

pair<DnaSequence::NuclType,DnaSequence::NuclType> Pair_Kmer::at(size_t index) const
{
    return {_seq_left.atRaw(index),_seq_right.atRaw(index)};
}

Pair_Kmer& Pair_Kmer::operator=(const Pair_Kmer &other) {
    _seq_left = other._seq_left;
    _seq_right = other._seq_right;
    _standard = other._standard;
    return *this;
}

bool Pair_Kmer::operator<(const Pair_Kmer &other) const {
    return (_seq_left < other._seq_left) && (_seq_right < other._seq_right);
}

bool Pair_Kmer::operator>(const Pair_Kmer &other) const {
    return (_seq_left > other._seq_left)&& (_seq_right > other._seq_right);
}

bool Pair_Kmer::operator==(const Pair_Kmer& other) const {
    return (_seq_left == other._seq_left && _seq_right == other._seq_right);
}

bool Pair_Kmer::operator!=(const Pair_Kmer& other) const{
    return (_seq_left != other._seq_left || _seq_right != other._seq_right);
}

//KmerIt -> Pair_end (true)
KmerIt<true>::KmerIt(const DnaSequence *seq_left, const DnaSequence *seq_right,  size_t pos)
        :_seq_left(seq_left),_seq_right(seq_right),_pos(pos)
{
    if ((pos != (seq_left->length() - Parameters::get().kmerSize+1)) & (pos != (seq_right->length() - Parameters::get().kmerSize+1)))
        _pair_kmer = Pair_Kmer(*seq_left, *seq_right, pos, Parameters::get().kmerSize);
}

bool KmerIt<true>::operator==(const KmerIt &kmerIt) const {
    return _seq_left == kmerIt._seq_left && _seq_right == kmerIt._seq_right && _pos == kmerIt._pos;
}

bool KmerIt<true>::operator!=(const KmerIt &kmerIt) const {
    return !(*this == kmerIt);
}

KmerIt<true>& KmerIt<true>::operator++() {
    size_t newPos = _pos + Parameters::get().kmerSize;
    if ((newPos < _seq_left->length()) & (newPos < _seq_right->length()))
        _pair_kmer.appendRight(_seq_left->atRaw(newPos),_seq_right->atRaw(newPos));
    _pos++;
    return *this;
}

KmerInfo<true> KmerIt<true>::operator*() const {
    return KmerInfo<true>(_pair_kmer,_pos);
}

//IterKmers -> Pair_end (true)
KmerIt<true> IterKmers<true>::begin() {
    if ((_seq_left.length() < Parameters::get().kmerSize) || (_seq_right.length() < Parameters::get().kmerSize))
        return this->end();
    return KmerIt<true>(&_seq_left,&_seq_right, 0);
}

KmerIt<true> IterKmers<true>::end() {
    return KmerIt<true>(&_seq_left, &_seq_right, std::min(_seq_left.length(),_seq_right.length())-Parameters::get().kmerSize+1);
}

