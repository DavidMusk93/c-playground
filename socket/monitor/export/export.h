#ifndef MONITOR_EXPORT_H
#define MONITOR_EXPORT_H

#define JSIG_PARAMLEFT "("
#define JSIG_PARAMRIGHT ")"
#define JSIG_VOID "V"
#define JSIG_INT "I"
#define JSIG_STRING "Ljava/lang/String;"
#define JSIG_OBJECT "Ljava/lang/Object;"

#define WHILEFALSE while(0)
#define DIMENSIONOF(x) sizeof(x)/sizeof(x[0])

#define _ARGS_17(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, ...) _16
#define ARGS_LEN(...) _ARGS_17(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#define _JSIG(x) JSIG_##x
#define _JSIGLIST_0(x)
#define _JSIGLIST_1(x) _JSIG(x)
#define _JSIGLIST_2(x, ...) _JSIG(x) _JSIGLIST_1(__VA_ARGS__)
#define _JSIGLIST_3(x, ...) _JSIG(x) _JSIGLIST_2(__VA_ARGS__)
#define _JSIGLIST_4(x, ...) _JSIG(x) _JSIGLIST_3(__VA_ARGS__)
#define _CONCAT(x, y) x##y
#define CONCAT(x, y) _CONCAT(x,y)
#define JSIGLIST(...) CONCAT(_JSIGLIST_,ARGS_LEN(__VA_ARGS__))(__VA_ARGS__)

#define FORWARDER_DEFAULTPORT 23402

#define JNICLASS "x/y/z"
#define JNIVRESION JNI_VERSION_1_6

#define JNIFUNCTIONARGS(env, obj) JNIEnv*env,jobject obj
#define JNIHOOKPOINTARGS(vm, reserved) JavaVM*vm,void*reserved

enum EXPORT_STATUS {
    EXPORT_SUCCESS = 0,
    EXPORT_FAILURE = 1,
};

#endif //MONITOR_EXPORT_H
