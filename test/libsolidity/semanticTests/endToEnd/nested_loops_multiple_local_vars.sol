contract test {
    function f(uint x) public returns(uint y) {
        while (x > 0) {
            uint z = x + 10;
            uint k = z + 1;
            if (k > 20) {
                break;
                uint p = 100;
                k += p;
            }
            if (k > 15) {
                x--;
                continue;
                uint t = 1000;
                x += t;
            }
            while (k > 10) {
                uint m = k - 1;
                if (m == 10) return x;
                return k;
                uint h = 10000;
                z += h;
            }
            x--;
            break;
        }
        return x;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256): 0 -> 0
// f(uint256): 1 -> 12
// f(uint256): 2 -> 13
// f(uint256): 3 -> 14
// f(uint256): 4 -> 15
// f(uint256): 5 -> 15
// f(uint256): 6 -> 15
// f(uint256): 7 -> 15
// f(uint256): 8 -> 15
// f(uint256): 9 -> 15
// f(uint256): 10 -> 10
// f(uint256): 11 -> 11
