library L {
    enum E { A, B }

    function equals(E a, E b) internal pure returns (bool) {
        return a == b;
    }
}

contract C {
    using L for L.E;

    function equalsA(uint choice) public returns (bool) {
        L.E x = L.E.A;
        return x.equals(L.E(choice));
    }
}
// ----
// equalsA(uint256): 0 -> true
// equalsA(uint256): 1 -> false
