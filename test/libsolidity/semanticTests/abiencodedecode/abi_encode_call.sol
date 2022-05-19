pragma abicoder v2;
contract C {
	type UnsignedNumber is uint256;
	enum Enum { First, Second, Third }

	struct Struct {
		UnsignedNumber[] dynamicArray;
		uint256 justAnInt;
		string name;
		bytes someBytes;
		Enum theEnum;
	}

	function callMeMaybe(Struct calldata _data, int256 _intVal, string memory _nameVal) external pure {
		assert(_data.dynamicArray.length == 3);
		assert(UnsignedNumber.unwrap(_data.dynamicArray[0]) == 0);
		assert(UnsignedNumber.unwrap(_data.dynamicArray[1]) == 1);
		assert(UnsignedNumber.unwrap(_data.dynamicArray[2]) == 2);
		assert(_data.justAnInt == 6);
		assert(keccak256(bytes(_data.name)) == keccak256("StructName"));
		assert(keccak256(_data.someBytes) == keccak256(bytes("1234")));
		assert(_data.theEnum == Enum.Second);
		assert(_intVal == 5);
		assert(keccak256(bytes(_nameVal)) == keccak256("TestName"));
	}

	function callExternal() public returns (bool) {
		Struct memory structToSend;
		structToSend.dynamicArray = new UnsignedNumber[](3);
		structToSend.dynamicArray[0] = UnsignedNumber.wrap(0);
		structToSend.dynamicArray[1] = UnsignedNumber.wrap(1);
		structToSend.dynamicArray[2] = UnsignedNumber.wrap(2);
		structToSend.justAnInt = 6;
		structToSend.name = "StructName";
		structToSend.someBytes = bytes("1234");
		structToSend.theEnum = Enum.Second;

		(bool success,) = address(this).call(abi.encodeCall(this.callMeMaybe, (
			structToSend,
			5,
			"TestName"
		)));

		return success;
	}
}

// ----
// callExternal() -> true
