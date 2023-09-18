event E();

library L {
    event E();
}

contract C {
    function main() external pure returns (bytes32, bytes32) {
        assert(E.selector == L.E.selector);

        return (E.selector, L.E.selector);
    }
}
// ----
// main() -> 0x92bbf6e823a631f3c8e09b1c8df90f378fb56f7fbc9701827e1ff8aad7f6a028, 0x92bbf6e823a631f3c8e09b1c8df90f378fb56f7fbc9701827e1ff8aad7f6a028
