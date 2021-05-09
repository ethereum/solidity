{
	function fun() -> a3, b3, c3, d3, e3, f3, g3, h3, i3, j3, k3, l3, m3, n3, o3, p3
	{
		let a := 1
		let b := 1
		let z3 := 1
		sstore(a, b)
		sstore(add(a, 1), b)
		sstore(add(a, 2), b)
		sstore(add(a, 3), b)
		sstore(add(a, 4), b)
		sstore(add(a, 5), b)
		sstore(add(a, 6), b)
		sstore(add(a, 7), b)
		sstore(add(a, 8), b)
		sstore(add(a, 9), b)
		sstore(add(a, 10), b)
		sstore(add(a, 11), b)
		sstore(add(a, 12), b)
		a3 := 1
		b3 := 1
		c3 := 1
		d3 := 1
		e3 := 1
		f3 := 1
		g3 := 1
		h3 := 1
		i3 := 1
		j3 := 1
		k3 := 1
		l3 := 1
		m3 := 1
		o3 := 1
		p3 := 1
	}
	let a1, b1, c1, d1, e1, f1, g1, h1, i1, j1, k1, l1, m1, n1, o1, p1 := fun()
	let a2, b2, c2, d2, e2, f2, g2, h2, i2, j2, k2, l2, m2, n2, o2, p2 := fun()
	sstore(a1, a2)
}
