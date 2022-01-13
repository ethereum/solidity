contract Parent {
    constructor() {
        return;
    }
}

contract Child is Parent {
    uint public immutable baked = 123;
}

