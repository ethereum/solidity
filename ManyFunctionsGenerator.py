
n = 100

splitNumBegin = 128 - (n / 2)
i = 1

template = """
    function right{0}(uint seed) returns (uint) {{
        var r = nextRand(seed);
        if (r >= 2**{2})
            return right{1}(r);
        return left{1}(r);
    }}
    
    function left{0}(uint seed) returns (uint) {{
        var r = nextRand(nextRand(seed));
        if (r >= 2**{2})
            return left{1}(r);
        return right{1}(r);
    }}
"""

for i in range(1, n):
	print template.format(i, i + 1, i + splitNumBegin)