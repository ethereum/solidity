---
layout: docs
title: Constants
permalink: /docs/constants/
---

State variables can be declared as constant (this is not yet implemented
for array and struct types and not possible for mapping types).
{% highlight javascript %}
contract C {
  uint constant x = 32**22 + 8;
  string constant text = "abc";
}
{% endhighlight %}

This has the effect that the compiler does not reserve a storage slot
for these variables and every occurrence is replaced by their constant value.

The value expression can only contain integer arithmetics.
