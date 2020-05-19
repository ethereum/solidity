contract Test {
    bytes6 name;

    constructor() public {
        function (bytes6 _name) internal setter = setName;
        setter("abcdef");

        applyShift(leftByteShift, 3);
    }

    function getName() public returns (bytes6 ret) {
        return name;
    }

    function setName(bytes6 _name) private {
        name = _name;
    }

    function leftByteShift(bytes6 _value, uint _shift) public returns (bytes6) {
        return _value << _shift * 8;
    }

    function applyShift(function (bytes6 _value, uint _shift) internal returns (bytes6) _shiftOperator, uint _bytes) internal {
        name = _shiftOperator(name, _bytes);
    }
}

// ====
// compileViaYul: also
// ----
// getName() -> "def\x00\x00\x00"
