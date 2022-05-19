//
// Created by blackox626 on 2022/5/19.
//

#ifndef RGBANDYUV_HASHCODE_H
#define RGBANDYUV_HASHCODE_H


#include <string.h>

#define HASHCODE_MULT 31

//默认的hashCode
int myHashCodeDefault(void * a);

//int类型hashCode
int myHashCodeInt(void * a);

//char类型的hashCode
int myHashCodeChar(void * a);

//string类型的hashCode
int myHashCodeString(void * a);


#endif //RGBANDYUV_HASHCODE_H
