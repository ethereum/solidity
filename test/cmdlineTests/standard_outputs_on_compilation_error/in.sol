// SPDX-License-Identifier: GPL-3.0
pragma solidity *;

contract C {
    // This will trigger an error at the compilation stage.
    // CodeGenerationError due to immutable initialization in constructor being optimized out.
    uint immutable public x;

    constructor() {
        x = 0;
        while (true) {}
    }
}
