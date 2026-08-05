// ``f`` and ``g`` are mutually recursive and spill nothing themselves, but ``f`` reaches ``h``,
// which spills. ``p`` spills too and calls into the cycle, so it must not be given the same memory
// slot as ``h``: ``p`` holds ``seed`` across the call to ``g`` and reads it back afterwards.
// The ``msg.data.length`` guards keep the inliner from collapsing the call graph; the calldata of
// ``test(uint256)`` is 36 bytes, so all of them fall through.
contract C {
    uint public x;

    function h(uint seed) internal view returns (uint out) {
        if ((msg.data.length & 8) != 0)
            return seed;
        unchecked {
            uint h0 = address(this).balance + seed;
            uint h1 = address(this).balance;
            uint h2 = address(this).balance;
            uint h3 = address(this).balance;
            uint h4 = address(this).balance;
            uint h5 = address(this).balance;
            uint h6 = address(this).balance;
            uint h7 = address(this).balance;
            uint h8 = address(this).balance;
            uint h9 = address(this).balance;
            uint h10 = address(this).balance;
            uint h11 = address(this).balance;
            uint h12 = address(this).balance;
            uint h13 = address(this).balance;
            uint h14 = address(this).balance;
            uint h15 = address(this).balance;
            uint h16 = address(this).balance;
            uint h17 = address(this).balance;
            out = h0 + h1 + h2 + h3 + h4 + h5 + h6 + h7 + h8 + h9 + h10 + h11 + h12 + h13 + h14 + h15 + h16 + h17;
        }
    }

    function f(uint n, uint seed) internal view returns (uint r) {
        unchecked {
            if (n == 0)
                return h(seed);
            return g(n - 1, seed);
        }
    }

    function g(uint n, uint seed) internal view returns (uint r) {
        if ((msg.data.length & 2) != 0)
            return seed;
        return f(n, seed);
    }

    function p(uint seed) internal returns (uint out) {
        if ((msg.data.length & 16) != 0)
            return seed;
        unchecked {
            uint p0 = address(this).balance + seed;
            uint p1 = address(this).balance;
            uint p2 = address(this).balance;
            uint p3 = address(this).balance;
            uint p4 = address(this).balance;
            uint p5 = address(this).balance;
            uint p6 = address(this).balance;
            uint p7 = address(this).balance;
            uint p8 = address(this).balance;
            uint p9 = address(this).balance;
            uint p10 = address(this).balance;
            uint p11 = address(this).balance;
            uint p12 = address(this).balance;
            uint p13 = address(this).balance;
            uint p14 = address(this).balance;
            uint p15 = address(this).balance;
            uint p16 = address(this).balance;
            uint nested = g(0, seed + 2000);
            x = seed;
            out = nested + p0 + p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10 + p11 + p12 + p13 + p14 + p15 + p16;
        }
    }

    // Calling ``f`` before ``p`` is what makes the requirements of the cycle observable to ``p``.
    function test(uint z) external returns (uint, uint, uint) {
        unchecked {
            uint warm = f(z & 1, z + 1000);
            uint result = p(z + 3);
            return (warm, result, x);
        }
    }
}
// ====
// compileViaYul: true
// compileViaSSACFG: false
// ----
// test(uint256): 0 -> 1000, 2006, 2003
