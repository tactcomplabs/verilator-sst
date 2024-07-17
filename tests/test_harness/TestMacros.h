#ifndef _TEST_MACROS_H_
#define _TEST_MACROS_H_

#define CHECK_EQ(got, exp) \
    if ((got) != (exp)) { \
        _Pragma ("GCC diagnostic push") \
        _Pragma ("GCC diagnostic ignored \"-Wformat\"") \
        output->fatal(CALL_INFO,1,"GOT = %lu EXP = %lu\n",got,exp); \
        _Pragma ("GCC diagnostic pop") \
    }

#define CHECK_TRUE(got) \
    if (got == false) { \
        output->fatal(CALL_INFO,1,"GOT = %u, expected truthy value\n",got); \
    }

#define UNIMPLEMENTED_TEST output->fatal(CALL_INFO,1,"UNIMPLEMENTED TEST\n");

#define SKIP_TEST \
    output->verbose(CALL_INFO,1,0,"SKIPPING TEST\n"); \
    return;

#endif
