event E();

contract C {
    event E(uint);

    function f() public {
        emit E();
        emit E(1);
    }
}
// ----
// Warning 2519: (29-43): This declaration shadows an existing declaration.
// TypeError 6160: (84-87): Wrong argument count for function call: 0 arguments given but expected 1.
