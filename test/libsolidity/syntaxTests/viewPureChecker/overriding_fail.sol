contract D {
    uint x;
    function f() public view { x; }
    function g() public pure {}
}
contract C1 is D {
    function f() public override {}
    function g() public override view {}
}
contract C2 is D {
    function g() public override {}
}
// ----
// TypeError: (118-149): Overriding function changes state mutability from "view" to "nonpayable".
// TypeError: (154-190): Overriding function changes state mutability from "pure" to "view".
// TypeError: (216-247): Overriding function changes state mutability from "pure" to "nonpayable".
