contract C {
    uint[] data;
    function f(uint x, uint[] calldata input) public returns (uint, uint) {
        data.push(x);
        (uint a, uint[] calldata b) = fun(input, data);
        return (a, b[1]);

    }
}

function fun(uint[] calldata _x, uint[] storage _y) view  returns (uint, uint[] calldata) {
	return (_y[0], _x);
}
// ----
// f(uint256,uint256[]): 7, 0x40, 3, 8, 9, 10 -> 7, 9
