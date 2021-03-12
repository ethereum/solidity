library L {
    function equals(address a, address b) internal pure returns (bool) {
        return a == b;
    }
}

contract C {
    using L for address;

    function foo(address a, address b) public returns (bool) {
        return a.equals(b);
    }
}
// ====
// compileViaYul: also
// ----
// foo(address,address): 0x111122223333444455556666777788889999aAaa, 0x111122223333444455556666777788889999aAaa -> true
// foo(address,address): 0x111122223333444455556666777788889999aAaa, 0x0000000000000000000000000000000000000000 -> false
