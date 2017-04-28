# fixed_eytzinger_map [![Build Status](https://travis-ci.org/mikekazakov/eytzinger.svg?branch=master)](https://travis-ci.org/mikekazakov/eytzinger)
Cache-friendly associative container with an Eytzinger layout for C++11/14/17


## About
`fixed_eytzinger_map` is a free implementation of Eytzinger’s layout, in a form of an STL-like generic associative container, broadly compatible with a well-established access patterns.

An Eytzinger map, or BFS(breadth-first search) map, places elements in a lookup order, which leads to a better memory locality. In practice, such container can outperform searching in sorted arrays, like `boost::flat_map`, due to less cache misses made in a lookup process. In comparison with RB-based trees, like `std::map`, lookup in Eytzinger map can be multiple times faster. Some comparison graphs are [given here](https://kazakov.life/2017/03/06/cache-friendly-associative-container/).

Unlike std- or boost- associative containers, `fixed_eytzinger_map` can't be modified after initial construction. This limitation comes from it's layout principle and an inability to alter it with any bearable complexity. It's content can be assigned or swapped in a transaction-like manner, but no per-key alterations are allowed.

`fixed_eytzinger_map` supports heterogeneous lookup, either with `std::less<>` or using a custom comparator with an `is_transparent` tag.
