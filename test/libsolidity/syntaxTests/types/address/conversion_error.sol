contract C {
    function f() public pure returns (address) {
        return address(-1);
    }
    function g() public pure returns (address) {
        return -1;
    }
	function h() public pure returns (address) {
		return address(2**160);
	}
}
// ----
// TypeError 9640: (77-88): Explicit type conversion not allowed from "int_const -1" to "address".
// TypeError 6359: (160-162): Return argument type int_const -1 is not implicitly convertible to expected type (type of first return variable) address.
// TypeError 9640: (225-240): Explicit type conversion not allowed from "int_const 1461...(41 digits omitted)...2976" to "address".
