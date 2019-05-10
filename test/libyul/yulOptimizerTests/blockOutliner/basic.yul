{
	{
		let a let b let c let d
		{
			a := mul(c,b)
			if lt(a,c) {
				a := add(a,c)
			}
		}
		{
			if eq(1,2) {
				d := mul(a,b)
				if lt(d,a) {
					d := add(d,a)
				}
			}
			{
				d := add(d,a)
			}
		}
		{
			c := mul(a,b)
			if lt(c,a) {
				c := add(c,a)
			}
		}
	}
	function f(a, b) -> r {
	    {
			r := mul(a,b)
			if lt(r, a) {
			    r := add(r,a)
			}
		}
		function g(x, y) -> z {
		    z := mul(x,y)
			if lt(z,x) {
			    z := add(z,x)
			}
		}
		r := g(b,a)
	}
	function h(a) -> r {
		{
		    r := add(r,a)
		}
		{
		    r := add(r,a)
		}
	}
}
// ====
// step: blockOutliner
// ----
// {
//     {
//         let a
//         let b
//         let c
//         let d
//         { a := g_1(a, c, b) }
//         {
//             if eq(1, 2) { d := g_1(d, a, b) }
//             { d := outlined$66$(d, a) }
//         }
//         { c := g_1(c, a, b) }
//     }
//     function g(x, y) -> z
//     { z := g_1(z, x, y) }
//     function f(a_1, b_2) -> r
//     {
//         { r := g_1(r, a_1, b_2) }
//         r := g(b_2, a_1)
//     }
//     function h(a_3) -> r_4
//     {
//         { r_4 := outlined$66$(r_4, a_3) }
//         { r_4 := outlined$66$(r_4, a_3) }
//     }
//     function outlined$66$(a, c) -> a_2
//     {
//         a := add(a, c)
//         a_2 := a
//     }
//     function g_1(a, c, b) -> a_4
//     {
//         a := mul(c, b)
//         if lt(a, c) { a := outlined$66$(a, c) }
//         a_4 := a
//     }
// }
