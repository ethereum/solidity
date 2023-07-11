abstract contract C {
    constructor(uint, bool) {}
}

abstract contract D is C {}
abstract contract E is C { constructor() {} }
