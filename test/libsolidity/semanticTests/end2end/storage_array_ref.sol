
		contract BinarySearch {
		  /// Finds the position of _value in the sorted list _data.
		  /// Note that "internal" is important here, because storage references only work for internal or private functions
		  function find(uint[] storage _data, uint _value) internal returns (uint o_position) {
			return find(_data, 0, _data.length, _value);
		  }
		  function find(uint[] storage _data, uint _begin, uint _len, uint _value) private returns (uint o_position) {
			if (_len == 0 || (_len == 1 && _data[_begin] != _value))
			  return uint(-1); // failure
			uint halfLen = _len / 2;
			uint v = _data[_begin + halfLen];
			if (_value < v)
			  return find(_data, _begin, halfLen, _value);
			else if (_value > v)
			  return find(_data, _begin + halfLen + 1, halfLen - 1, _value);
			else
			  return _begin + halfLen;
		  }
		}

		contract Store is BinarySearch {
			uint[] data;
			function add(uint v) public {
				data.push(0);
				data[data.length - 1] = v;
			}
			function find(uint v) public returns (uint) {
				return find(data, v);
			}
		}
	
// ====
// optimize-yul: false
// ----
// find(uint256): 0x07 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// add(uint256): 0x07 -> 
// find(uint256): 0x07 -> 0x00
// add(uint256): 0x0b -> 
// add(uint256): 0x11 -> 
// add(uint256): 0x1b -> 
// add(uint256): 0x1f -> 
// add(uint256): 0x20 -> 
// add(uint256): 0x42 -> 
// add(uint256): 0xb1 -> 
// find(uint256): 0x07 -> 0x00
// find(uint256): 0x1b -> 0x03
// find(uint256): 0x20 -> 0x05
// find(uint256): 0xb0 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// find(uint256): 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// find(uint256): 0x190 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff

