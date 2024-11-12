#pragma once
#include <iostream>
#include <cstdlib>
#define Assert(argc,argc1) if(argc){std::cerr<< #argc<<" not flase, line:"<<__LINE__<<" file:"<<__FILE__<<" "<<argc1<<std::endl;std::abort();}
#define TimeOut(argc1,argc2) {std::cerr<< #argc1 <<":"<< argc2<<std::endl;std::abort();}