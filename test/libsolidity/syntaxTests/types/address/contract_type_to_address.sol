interface I {}

library L {}

contract C {
    function f() public pure {
        address(C);
        address(I);
        address(L); // This one is allowed

        address(type(C));
        address(type(I));
        address(type(L));
    }
}
// ----
// TypeError 9640: (82-92='address(C)'): Explicit type conversion not allowed from "type(contract C)" to "address".
// TypeError 9640: (102-112='address(I)'): Explicit type conversion not allowed from "type(contract I)" to "address".
// TypeError 9640: (166-182='address(type(C))'): Explicit type conversion not allowed from "type(contract C)" to "address".
// TypeError 9640: (192-208='address(type(I))'): Explicit type conversion not allowed from "type(contract I)" to "address".
// TypeError 9640: (218-234='address(type(L))'): Explicit type conversion not allowed from "type(library L)" to "address".
