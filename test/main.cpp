#include "Split.h"

// System Headers
#include <cassert>
#include <iostream>
#include <tuple>

int main()
{
    std::string str = "abc:123:456:7,89:xyz";

    auto split1 = tjh::split<3>(str.begin(), str.end(), ":");

    std::cout << std::tuple_size<decltype(split1)>::value << std::endl;
    static_assert(std::tuple_size<decltype(split1)>::value == 3, "split1.count != 3");

    std::cout << std::get<0>(split1) << std::endl;
    assert(std::get<0>(split1) == "abc");

    std::cout << std::get<1>(split1) << std::endl;
    assert(std::get<1>(split1) == "123");

    std::cout << std::get<2>(split1) << std::endl;
    assert(std::get<2>(split1) == "456:7,89:xyz");
    
    std::cout << "SUCCESS" << std::endl;
}
