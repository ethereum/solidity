event E();

library L1 {
    event E(string);
}

library L2 {
    event E();
}

library K {
    function main() internal pure returns (bytes32, bytes32, bytes32) {
        // Here E is the global event.
        assert(E.selector != L1.E.selector);
        assert(E.selector == L2.E.selector);

        return (E.selector, L1.E.selector, L2.E.selector);
    }
}

contract C {
    event E(string);

    function main() external pure returns (bytes32, bytes32, bytes32) {
        // Here E is the local event.
        assert(E.selector == L1.E.selector);
        assert(E.selector != L2.E.selector);

        return (E.selector, L1.E.selector, L2.E.selector);
    }

    function k_main() external pure returns (bytes32, bytes32, bytes32) {
        return K.main();
    }
}
// ----
// main() -> 0x3e9992c940c54ea252d3a34557cc3d3014281525c43d694f89d5f3dfd820b07d, 0x3e9992c940c54ea252d3a34557cc3d3014281525c43d694f89d5f3dfd820b07d, 0x92bbf6e823a631f3c8e09b1c8df90f378fb56f7fbc9701827e1ff8aad7f6a028
// k_main() -> 0x92bbf6e823a631f3c8e09b1c8df90f378fb56f7fbc9701827e1ff8aad7f6a028, 0x3e9992c940c54ea252d3a34557cc3d3014281525c43d694f89d5f3dfd820b07d, 0x92bbf6e823a631f3c8e09b1c8df90f378fb56f7fbc9701827e1ff8aad7f6a028
