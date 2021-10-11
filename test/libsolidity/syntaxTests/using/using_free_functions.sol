function id(uint x) pure returns (uint) {
    return x;
}

function zero(uint) pure returns (uint) {
    return 0;
}
using {id} for uint;
contract C {
    using {zero} for uint;

    function g(uint z) pure external {
        z.zero();
        z.id();
    }
}
