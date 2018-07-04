contract D {
    uint x;
    function f() public view { x; }
    function g() public pure {}
}
contract C1 is D {
    function f() public {}
    function g() public view {}
}
contract C2 is D {
    function g() public {}
}
// ----
// TypeError: (118-140): Overriding function changes state mutability from "view" to "nonpayable".
// TypeError: (145-172): Overriding function changes state mutability from "pure" to "view".
// TypeError: (198-220): Overriding function changes state mutability from "pure" to "nonpayable".
