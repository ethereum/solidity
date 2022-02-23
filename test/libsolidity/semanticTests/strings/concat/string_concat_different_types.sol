contract C{
    string s = "bcdef";

    function f(string memory a) public returns (string memory) {
        return string.concat(a, "bcdef");
    }
    function g(string calldata a) public returns (string memory) {
        return string.concat(a, "abcdefghabcdefghabcdefghabcdefghab");
    }
    function h(string calldata a) public returns (string memory) {
        return string.concat(a, s);
    }
    function j(string calldata a) public returns (string memory) {
        string storage ref = s;
        return string.concat(a, ref, s);
    }
    function k(string calldata a, bytes memory b) public returns (string memory) {
        return string.concat(a, string(b));
    }
    function slice(string calldata a) public returns (string memory) {
        require(bytes(a).length > 2, "");
        return string.concat(a[:2], a[2:]);
    }
    function strParam(bytes calldata a) public returns (string memory) {
        return string.concat(string(a), "bcdef");
    }
}
// ====
// compileViaYul: also
// ----
// f(string): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 0x25, 0x6162636461626364616263646162636461626364616263646162636461626364, 44502269928904312298000709931354278973409164155382318144318241583783949107200
// g(string): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 0x42, 0x6162636461626364616263646162636461626364616263646162636461626364, 0x6162636465666768616263646566676861626364656667686162636465666768, 44047497324925121336511606693520958599579173549109180625971642598225011015680
// h(string): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 0x25, 0x6162636461626364616263646162636461626364616263646162636461626364, 44502269928904312298000709931354278973409164155382318144318241583783949107200
// j(string): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 0x2a, 0x6162636461626364616263646162636461626364616263646162636461626364, 44502269928944786876717917111204727192787026596791669343131645116682757734400
// k(string,bytes): 0x40, 0x80, 32, "abcdabcdabcdabcdabcdabcdabcdabcd", 5, "bcdef" -> 0x20, 0x25, 0x6162636461626364616263646162636461626364616263646162636461626364, 44502269928904312298000709931354278973409164155382318144318241583783949107200
// slice(string): 0x20, 4, "abcd" -> 0x20, 4, "abcd"
// strParam(bytes): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 0x25, 0x6162636461626364616263646162636461626364616263646162636461626364, 44502269928904312298000709931354278973409164155382318144318241583783949107200
