//
//  potato.hpp
//  project3_650
//
//  Created by Elaine on 2022/2/19.
//

#ifndef potato_hpp
#define potato_hpp

#include <stdio.h>

class Potato{
public:
    size_t hops;
    int path[512];
    int cnt;
public:
    Potato():cnt(0), hops(0){}

};

#endif /* potato_hpp */

