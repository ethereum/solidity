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
// TypeError: (118-149): Overriding function mutability differs: "view" vs "nonpayable".
// TypeError: (154-190): Overriding function mutability differs: "pure" vs "view".
// TypeError: (216-247): Overriding function mutability differs: "pure" vs "nonpayable".
