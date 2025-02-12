# ILIS: an Interval- and Lyapunov-based library for the computation of Invariant Sets 

ILIS is a C++ library to semi-automatically compute invariant sets' approximations of a given dynamical system with an associated candidate Lyapunov function.

ILIS allows for the computation of a **certified covering** of the so-called *value function* (i.e., the function that encodes the maximum of the energy’s Lie derivative at each energy’s level, see [this paper](https://hal.science/hal-03957165) for a formal definition), which gives us fundamental information on the system’s convergence properties. It additionally allows for a faster, certified computation of the value function’s zeros alone, which encode estimates of the system's equilibria and/or attractors and their regions of attraction. Under the hood, ILIS implements a branch-and-prune algorithm based on interval analysis methods, effectively implementing a specialized and certified global parametric optimization solver.

ILIS has been developed mainly for internal use and it will most likely remain in permanent beta. It is released under the [CRAPL](https://matt.might.net/articles/crapl/).
