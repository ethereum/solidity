contract D {
    uint x;
    function f() public virtual view { x; }
    function g() public virtual pure {}
}
contract C1 is D {
    function f() public override {}
    function g() public virtual override view {}
}
contract C2 is D {
    function g() public override {}
}
// ----
// TypeError: (134-165): Overriding function changes state mutability from "view" to "nonpayable".
// TypeError: (170-214): Overriding function changes state mutability from "pure" to "view".
// TypeError: (240-271): Overriding function changes state mutability from "pure" to "nonpayable".
