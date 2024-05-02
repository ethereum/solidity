==== Source: A.sol ====
contract DummyContract1 {}
contract DummyContract2 {}
contract DummyContract3 {}

==== Source: B.sol ====
contract B {
    function f(uint8 a_0, uint8 a_1, uint8 a_2) public pure {
        a_1 = 1;
        a_2 = 2;
        a_0;
    }
}
