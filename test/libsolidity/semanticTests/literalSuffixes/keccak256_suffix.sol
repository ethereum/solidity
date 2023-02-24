==== Source: builtin.sol ====
function builtinKeccak(bytes memory input) pure returns (bytes32) {
    return keccak256(input);
}
==== Source: suffix.sol ====
import {builtinKeccak} from "builtin.sol";

function keccak256(bytes memory input) pure suffix returns (bytes32) {
    // NOTE: A call to keccak256() here would not call the built-in
    // and would instead result in infinite recursion.
    return builtinKeccak(input);
}

contract C {
    function testBuiltin() public pure returns (bytes32) {
        return builtinKeccak("solidity");
    }

    function testSuffix() public pure returns (bytes32) {
        return "solidity" keccak256;
    }

    function testSuffixFunctionCall() public pure returns (bytes32) {
        return keccak256("solidity");
    }
}
// ----
// testBuiltin() -> 0xa477d97b122e6356d32a064f9ee824230d42d04c7d66d8e7d125a091a42b0b25
// testSuffix() -> 0xa477d97b122e6356d32a064f9ee824230d42d04c7d66d8e7d125a091a42b0b25
// testSuffixFunctionCall() -> 0xa477d97b122e6356d32a064f9ee824230d42d04c7d66d8e7d125a091a42b0b25
