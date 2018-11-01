#include <boost/functional/hash.hpp>
#include <cassert>
#include <string>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <algorithm>

class DnaSequence
{
public:
	typedef size_t NuclType;

private:
	static const int NUCL_BITS = 3;
    static const int operator_and = 3;
	static const int NUCL_IN_CHUNK = sizeof(NuclType) * 8 / NUCL_BITS;
    static const int offset = NUCL_BITS;

	struct SharedBuffer
	{
		SharedBuffer(): useCount(0), length(0) {}
		size_t useCount;
		size_t length;
		std::vector<size_t> chunks;
	};

public:
    static char nfi(size_t nuc)
    {
        if (!nuc)
            return 'A';
        if (nuc == 1)
            return 'C';
        if (nuc == 2)
            return 'G';
        if (nuc == 3)
            return 'T';
        return 'N';
    }

	DnaSequence():
		_complement(false)
	{
		_data = new SharedBuffer;
		++_data->useCount;
	}

	~DnaSequence()
	{
		if (_data != nullptr)
		{
			--_data->useCount;
			if (_data->useCount == 0)
				delete _data;
		}
	}

	explicit DnaSequence(const std::string& string):
		_complement(false)
	{
		_data = new SharedBuffer;
		++_data->useCount;

		if (string.empty()) return;

		_data->length = string.length();
		_data->chunks.assign((_data->length - 1) / NUCL_IN_CHUNK + 1, 0);
		for (size_t i = 0; i < string.length(); ++i) {
            size_t chunkId = i / NUCL_IN_CHUNK;
            _data->chunks[chunkId] |= dnaToId(string[i]) << (i % NUCL_IN_CHUNK) * offset;
        }
	}

	//Only accepts 1 nuc insertion -> Recordar que estan al reves en _data
	void append_nuc_right (DnaSequence::NuclType dnaSymbol) const{
		if (!_data->length && !_data->chunks.size())
		{
			_data->chunks.push_back(dnaSymbol);
			_data->length++;
			return;
		}
		if ((_data->length / NUCL_IN_CHUNK) == _data->chunks.size())
			_data->chunks.push_back(dnaSymbol);
		else
			_data->chunks[_data->chunks.size()-1] |= (dnaSymbol
				<< (_data->length % NUCL_IN_CHUNK) *offset);
		_data->length++;
	}

	void append_with_replace_right(DnaSequence::NuclType symbol) const {
		for (int i = 0; i < std::max(0,(int)_data->chunks.size()-1); ++i)
			_data->chunks[i] = (_data->chunks[i] >> offset) |
					(_data->chunks[i+1] << (NUCL_IN_CHUNK-1)*offset);
		_data->chunks[_data->chunks.size()-1] = (_data->chunks[_data->chunks.size()-1]>> offset) |
				(symbol << ((_data->length-1) % NUCL_IN_CHUNK)*offset);
	}

	//Only accepts 1 nuc insertion
	void append_nuc_left (DnaSequence::NuclType dnaSymbol) const{
		_data->length++;
		if ((_data->length/NUCL_IN_CHUNK) == _data->chunks.size())_data->chunks.push_back(0);
		for (uint i = _data->chunks.size()-1; i > 0; --i) {
			_data->chunks[i] = (_data->chunks[i] << offset) | (
					(_data->chunks[i-1] >> (NUCL_IN_CHUNK-1)*offset));
		}
		_data->chunks[0] = (_data->chunks[0] << offset) | dnaSymbol;
	}

	void append_with_replace_left(DnaSequence::NuclType symbol) const{
		for (int i = _data->chunks.size()-1; i > 0; --i){
			_data->chunks[i] = ((_data->chunks[i]<<offset) |
					(_data->chunks[i-1] >> (NUCL_IN_CHUNK-1)*offset));
        }
		_data->chunks[0] = (_data->chunks[0] << offset) | symbol ;
	}

	void set(DnaSequence::NuclType dnaSymbol, size_t index) const
	{
		size_t aux_v = ~(operator_and << (index % NUCL_IN_CHUNK)*offset);
		_data->chunks[index / NUCL_IN_CHUNK] &= aux_v;
		_data->chunks[index / NUCL_IN_CHUNK] |=dnaSymbol << ((index % NUCL_IN_CHUNK)*offset);
	}

	DnaSequence(const DnaSequence& other):
		_complement(other._complement)
	{
		_data = new SharedBuffer();
		_data->length = other.length();
		_data->chunks = other.getChunk();
		++_data->useCount;
		/*Original*/
		//		++_data->useCount;
	}

	DnaSequence(DnaSequence&& other):
		_data(other._data),
		_complement(other._complement)
	{
		other._data = nullptr;
	}

	DnaSequence& operator=(const DnaSequence& other)
	{
		--_data->useCount;
		if (_data->useCount == 0) delete _data;

		_complement = other._complement;
		_data = other._data;
		++_data->useCount;
		return *this;
	}

	DnaSequence& operator=(DnaSequence&& other)
	{
		_data = other._data;
		_complement = other._complement;
		other._data = nullptr;
		return *this;
	}

	void copy(const DnaSequence* other){
		_data = other->_data;
		_complement = other->_complement;
		++_data->useCount;
	}

	std::vector<size_t> getChunk()const {
		return _data->chunks;
	}

	NuclType operator[](int index){
		if (_complement)
			index = _data->length-index+1;
		size_t id = (_data->chunks[index / NUCL_IN_CHUNK] >> (index % NUCL_IN_CHUNK)*offset)&operator_and;
        if (id > 3)
            return 4;
		return !_complement?id : ~id & 3;
	}

	//TODO: Revisar las constantes esas
	const DnaSequence& operator*() const{
		return *this;
	}

    /*
     * Seria necesario extenderlo para tratar la orientacion de las lecturas
     */
	bool operator==(const DnaSequence& other) const {
		if (other.length() != this->length())
			return false;
        return (this->_data->chunks == other._data->chunks);
	}

	bool operator!=(const DnaSequence& other) const {
		if (other.length() != this->length())
			return true;
		return !(this->_data->chunks == other._data->chunks);
	}

    bool higher(const DnaSequence & other) const
    {
        if (other.length() > length())
            return false;
        if (other.length() < length())
            return true;
        for (uint i = 0; i < _data->length;++i)
        {
            if (atRaw(i) > other.atRaw(i))
                return true;
            if (atRaw(i) < other.atRaw(i))
                return false;
        }
        return false;
    }

    bool lower(const DnaSequence & other) const
    {
        if (other.length() > length())
            return true;
        if (other.length() < length())
            return false;
        for (uint i = 0; i < _data->length;++i)
        {
            if (atRaw(i) > other.atRaw(i))
                return false;
            if (atRaw(i) < other.atRaw(i))
                return true;
        }
        return false;
    }

    bool operator<(const DnaSequence & other) const
    {
        return (lower(other));
    }

    bool operator>(const DnaSequence & other) const
    {
        return (higher(other));
    }

	size_t length() const {return _data->length;}

	size_t hash() const
	{
		size_t seed = 0;
		for (uint i = 0; i < _data->chunks.size();++i)
		    boost::hash_combine(seed, _data->chunks[i]);
		return seed;
	}

	char at(size_t index) const
	{
		if (_complement)
		{
			index = _data->length - index - 1;
		}
		if (index >= length())
			index = length()-1;
		size_t id = (_data->chunks[index / NUCL_IN_CHUNK] >>
					 (index % NUCL_IN_CHUNK) * offset ) & operator_and;
        if (id > 3)
            return 'N';
		return idToDna(!_complement ? id : ~id & 3);
	}

	NuclType atRaw(size_t index) const
	{
		if (_complement)
		{
			index = _data->length - index - 1;
		}
		size_t id = (_data->chunks[index / NUCL_IN_CHUNK] >>
					 (index % NUCL_IN_CHUNK) * offset ) & operator_and;
        if (id > 3)
            return 4;
		return !_complement ? id : ~id & 3;
	}

	DnaSequence complement() const
	{
		DnaSequence complSequence(*this);
		complSequence._complement = !(this->_complement);
        complSequence = complSequence.substr(0, complSequence._data->length);
        complSequence._complement = !(complSequence._complement);
		return complSequence;
	}

	DnaSequence substr(size_t start, size_t length) const;
	std::string str() const;

private:
	static std::vector<size_t> _dnaTable;

	static size_t dnaToId(char c)
	{
		return _dnaTable[(size_t)c];
	}

	static char idToDna(size_t id)
	{
		static char table[] = {'A', 'C', 'G', 'T'};
		return table[id];
	}

	struct TableFiller
	{
		TableFiller()
		{
			static bool tableFilled = false;
			if (!tableFilled)
			{
				tableFilled = true;
				_dnaTable.assign(256, -1);	//256 chars
				_dnaTable[(size_t)'A'] = 0;
				_dnaTable[(size_t)'a'] = 0;
				_dnaTable[(size_t)'C'] = 1;
				_dnaTable[(size_t)'c'] = 1;
				_dnaTable[(size_t)'G'] = 2;
				_dnaTable[(size_t)'g'] = 2;
				_dnaTable[(size_t)'T'] = 3;
				_dnaTable[(size_t)'t'] = 3;
                _dnaTable[(size_t)'N'] = 4;
                _dnaTable[(size_t)'n'] = 4;
			}
		}
	};
	static TableFiller _filler;

	SharedBuffer* _data;
	bool _complement;
};

inline std::string DnaSequence::str() const
{
	std::string result;
	result.reserve(this->length());
	for (size_t i = 0; i < this->length(); ++i)
	{
		result.push_back(this->at(i));
	}
	return result;
}

inline DnaSequence DnaSequence::substr(size_t start, size_t length) const
{
	DnaSequence newSequence;
	if (start >= _data->length)
		return newSequence;
	if (start + length > _data->length)
	{
		length = _data->length - start;
	}

	newSequence._data->length = length;
	newSequence._data->chunks.assign((length - 1) / NUCL_IN_CHUNK + 1, 0);

	for (size_t i = 0; i < length; ++i)
	{
		size_t nucId = this->atRaw(start + i);
		size_t newChunkId = i / NUCL_IN_CHUNK;
		newSequence._data->chunks[newChunkId] |= nucId << (i % NUCL_IN_CHUNK) * offset;
	}
    newSequence._complement = _complement;

	return newSequence;
}
