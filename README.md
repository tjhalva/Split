# Split: A Variadic C++ String to std::tuple Splitter

The standalone split<N> method takes two std::string iterators as input along with a std::string delimiter and returns a std::tuple containing the sub-divided string(s) which are separated by the provided delimiter. The number of delimiters, N, is provided statically at runtime via an integral template argument to the split method.

## Behavior

If the delimiter occurs less than N times, empty values are returned. If the delimiter occurs more than N times, the last entry will contain the remainder of the string after the (N-1)th delimiter. If the delimiter occurs in the provided string <=N times, the delimiter will not be present within the output values.

## Language Support & Compilation

The method requires features present within the C++14 language specification; the required C++14 features are however re-implemented to support a C++11 compiler. If a C++14 compiler is used, the latest language specification is used and the supporting code for C++11 is excluded. The method has been tested using gcc and clang; there is however a major difference between the compilers in that the order in which tuple indices are iterated is opposite between these two compilers. Appropriate compiler specific behavior has been implemented to account for this discrepancy.
