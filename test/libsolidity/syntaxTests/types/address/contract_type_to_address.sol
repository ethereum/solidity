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
// TypeError 9640: (82-92): Explicit type conversion not allowed from "type(contract C)" to "address".
// TypeError 9640: (102-112): Explicit type conversion not allowed from "type(contract I)" to "address".
// TypeError 9640: (166-182): Explicit type conversion not allowed from "type(contract C)" to "address".
// TypeError 9640: (192-208): Explicit type conversion not allowed from "type(contract I)" to "address".
// TypeError 9640: (218-234): Explicit type conversion not allowed from "type(library L)" to "address".
